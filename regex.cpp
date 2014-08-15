#include <stingray/toolkit/regex.h>

#ifdef PLATFORM_POSIX
#	include <regex.h>
#else
#	warning No regex support on nonposix systems!
#endif

#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/array.h>
#include <stingray/toolkit/exception.h>


namespace stingray
{

#ifdef PLATFORM_POSIX
	class regex::Impl
	{
	private:
		std::string	_str;
		regex_t		_regex;

	public:
		Impl(const std::string& str)
			: _str(str)
		{
			ReplaceAll(_str, "\\d", "[0-9]");

			int ret = regcomp(&_regex, _str.c_str(), REG_EXTENDED);
			TOOLKIT_CHECK(ret == 0, StringBuilder() % "Could not compile regex '" % _str % "', ret = " % ret % "\n" % GetRegexError(ret));
		}

		~Impl()
		{
			regfree(&_regex);
		}

		bool Match(const std::string& str, smatch& m)
		{
			array<regmatch_t, 32>	matches;
			int ret = regexec(&_regex, str.c_str(), matches.size(), matches.data(), 0);

			if (ret == REG_NOMATCH)
				return false;
			TOOLKIT_CHECK(ret == 0, StringBuilder() % "Could not execute regex '" % _str % "' for string '" % str % "', ret = " % ret % "\n" % GetRegexError(ret));

			for (regmatch_t* submatch = matches.data(); submatch->rm_so >= 0; ++submatch)
			{
				int len = submatch->rm_eo - submatch->rm_so;
				TOOLKIT_CHECK(len > 0, "Submatch length is negative!");
				std::string match_str(str.substr(submatch->rm_so, len));
				m._results.push_back(match_str);
				m._positions.push_back(submatch->rm_so);
			}
			return true;
		}

	private:
		std::string GetRegexError(int errCode) const
		{
			std::string result(256, '~');
			result.resize(regerror(errCode, &_regex, &result[0], result.size()));
			return result;
		}
	};
#endif

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


	bool regex_search(const std::string& str, smatch& m, const regex& re)
	{
#ifdef PLATFORM_POSIX
		return re._impl->Match(str, m);
#else
		TOOLKIT_THROW(NotImplementedException());
#endif
	}

}
