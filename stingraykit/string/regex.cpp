// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/regex.h>

#include <stingraykit/string/StringUtils.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/variant.h>

#ifdef PLATFORM_POSIX
#	include <regex.h>
#else
#	warning No regex support on nonposix systems!
#endif

namespace stingray
{

	struct RegexMatch
	{
		int	Begin;
		int	End;

		RegexMatch() : Begin(0), End(0) { }

		int GetLength() const
		{
			int len = End - Begin;
			STINGRAYKIT_CHECK(len >= 0, "Submatch length is negative!");
			return len;
		}

		std::string ToString() const { return StringBuilder() % "(" % Begin % ", " % End % ")"; }
	};

	typedef std::vector<RegexMatch>						RegexMatchVec;

	typedef variant<TypeList<std::string, int>::type>	ReplacementEntry;
	typedef std::vector<ReplacementEntry>				ReplacementEntryVec;

	class ReplacementEntryVisitor : public static_visitor<std::string>
	{
	private:
		const char*				_srcString;
		const RegexMatchVec&	_matches;

	public:
		ReplacementEntryVisitor(const char* srcString, const RegexMatchVec& matches)
			: _srcString(srcString), _matches(matches)
		{ }

		std::string operator () (int matchNumber) const
		{
			int i = matchNumber;
			STINGRAYKIT_CHECK(i >= 0 && i < (int)_matches.size(), IndexOutOfRangeException(i, _matches.size()));
			std::string res = std::string(_srcString + _matches[i].Begin, _matches[i].GetLength());
			return res;
		}

		std::string operator () (const std::string& str) const
		{ return str; }
	};

	template < typename PlatformRegex >
	class BasicRegexImpl : public PlatformRegex
	{
	public:
		BasicRegexImpl(const std::string& str)
			: PlatformRegex(str)
		{ }

		bool Search(const std::string& str, smatch& m, regex_constants::match_flag_type flags)
		{
			m._results.clear();
			m._positions.clear();

			STINGRAYKIT_CHECK(flags == regex_constants::match_default, NotImplementedException());

			RegexMatchVec matches;
			if (!this->DoMatch(str.c_str(), matches))
				return false;

			for (size_t i = 0; i < matches.size(); ++i)
			{
				RegexMatch& submatch = matches[i];
				m._results.push_back(str.substr(submatch.Begin, submatch.GetLength()));
				m._positions.push_back(submatch.Begin);
			}
			return true;
		}

		std::string Replace(const std::string& str, const regex& re, const std::string& replacement, regex_constants::match_flag_type flags)
		{
			STINGRAYKIT_CHECK((flags & ~regex_constants::format_first_only) == 0, NotImplementedException());

			ReplacementEntryVec replacement_entries;
			PlatformRegex group_regex("\\\\(\\d+)"); // TODO: cache this somewhere
			RegexMatchVec group_matches;

			size_t replacement_begin = 0;
			do
			{
				if (!group_regex.DoMatch(replacement.c_str() + replacement_begin, group_matches))
				{
					if (replacement_begin < replacement.size())
						replacement_entries.push_back(replacement.substr(replacement_begin));
					break;
				}

				if (group_matches.at(0).Begin > 0)
					replacement_entries.push_back(replacement.substr(replacement_begin, group_matches.at(0).Begin));
				replacement_entries.push_back(FromString<int>(replacement.substr(group_matches.at(1).Begin, group_matches.at(1).GetLength())));
				replacement_begin += group_matches.at(0).End;
			} while (true);

			int begin = 0;
			std::string result;

			RegexMatchVec matches;
			do
			{
				if (!this->DoMatch(str.c_str() + begin, matches))
				{
					result += str.substr(begin);
					break;
				}

				ReplacementEntryVisitor visitor(str.c_str() + begin, matches);

				result += str.substr(begin, matches.at(0).Begin);
				for (ReplacementEntryVec::const_iterator it = replacement_entries.begin(); it != replacement_entries.end(); ++it)
					result += apply_visitor(visitor, *it);
				begin += matches.at(0).End;
			} while (true);

			return result;
		}
	};

#ifdef PLATFORM_POSIX
	class PosixRegexImpl
	{
	private:
		std::string	_str;
		regex_t		_regex;

	public:
		PosixRegexImpl(const std::string& str)
			: _str(str)
		{
			ReplaceAll(_str, "\\d", "[0-9]");
			ReplaceAll(_str, "\\w", "[a-zA-Z0-9_]");

			int ret = regcomp(&_regex, _str.c_str(), REG_EXTENDED);
			STINGRAYKIT_CHECK(ret == 0, StringBuilder() % "Could not compile regex '" % _str % "', ret = " % ret % "\n" % GetRegexError(_regex, ret));
		}

		~PosixRegexImpl()
		{ regfree(&_regex); }

		bool DoMatch(const char* str, RegexMatchVec& matches) const
		{
			std::vector<regmatch_t> posix_matches(32);
			int ret = regexec(&_regex, str, posix_matches.size(), posix_matches.data(), 0);

			if (ret == REG_NOMATCH)
				return false;

			if (ret != 0)
				STINGRAYKIT_THROW(StringBuilder() % "Could not execute regex '" % _str % "' for string '" % str % "', ret = " % ret % "\n" % GetRegexError(_regex, ret));

			int count = 0;
			while (posix_matches[count].rm_so >= 0)
				++count;


			matches.resize(count);
			for (int i = 0; i < count; ++i)
			{
				matches[i].Begin = posix_matches[i].rm_so;
				matches[i].End = posix_matches[i].rm_eo;
			}
			return true;
		}

	private:
		static std::string GetRegexError(const regex_t& regex, int errCode)
		{
			std::string result(256, '~');
			result.resize(regerror(errCode, &regex, &result[0], result.size()));
			return result;
		}
	};

	typedef PosixRegexImpl	PlatformRegexImpl;
#endif

	class regex::Impl : public BasicRegexImpl<PlatformRegexImpl>
	{
	public:
		Impl(const std::string& str) : BasicRegexImpl<PlatformRegexImpl>(str) { }
	};


	regex::regex(const std::string& str)
		: _impl(new Impl(str))
	{ }

	regex::regex(const regex& other)
		: _impl(other._impl)
	{ }

	regex::~regex()
	{ }

	regex& regex::operator = (const regex& other)
	{
		_impl = other._impl;
		return *this;
	}


	bool regex_search(const std::string& str, smatch& m, const regex& re, regex_constants::match_flag_type flags)
	{
		return re._impl->Search(str, m, flags);
	}


	std::string regex_replace(const std::string& str, const regex& re, const std::string& replacement, regex_constants::match_flag_type flags)
	{
		return re._impl->Replace(str, re, replacement, flags);
	}

}
