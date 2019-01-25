#ifndef STINGRAYKIT_STRING_REGEX_H
#define STINGRAYKIT_STRING_REGEX_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/shared_ptr.h>

#include <numeric>
#include <vector>

/*
 * 	.			Match any character
 * 	^			Match beginning of input
 * 	$			Match end of input
 * 	\b			Match word boundary
 * 	\B			Match anything other than a word boundary
 * 	|			Or operator
 *
 * 	()			Capture group
 *
 * 	Repetition
 * 	?			<= 1
 * 	*			>= 0
 * 	+			>= 1
 * 	{n}			n
 * 	{n,}		>= n
 * 	{n,m}		>= n && <= m
 *
 * 	Sets
 * 	[abc]		Any of characters included
 * 	[^abc]		Any of characters NOT included
 * 	[a-z]		Any of characters in the range
 * 	[a-zA-Z]	Any of characters in the ranges
 */

namespace stingray
{

	namespace regex_constants
	{
		enum match_flag_type
		{
			match_default		= 0x0,
			format_default		= 0x0,

			match_not_bol		= 0x1,
			match_not_eol		= 0x2,
			match_not_bow		= 0x4,
			match_not_eow		= 0x8,
			match_any			= 0x10,
			match_not_null		= 0x20,
			match_continuous	= 0x40,
			match_prev_avail	= 0x80,

			format_sed			= 0x100,
			format_no_copy		= 0x200,
			format_first_only	= 0x400
		};
	}

	class regex;
	class smatch;


	struct regex_error : public Exception
	{
		regex_error() : Exception("regex error!") { }
		regex_error(const std::string& msg) : Exception("regex error: " + msg) { }
	};


	bool regex_search(const std::string& str, smatch& m, const regex& re, regex_constants::match_flag_type flags = regex_constants::match_default);
	std::string regex_replace(const std::string& str, const regex& re, const std::string& replacement, regex_constants::match_flag_type flags = regex_constants::format_default);


	class regex
	{
		friend class smatch;
		friend bool regex_search(const std::string& str, smatch& m, const regex& re, regex_constants::match_flag_type flags);
		friend std::string regex_replace(const std::string& str, const regex& re, const std::string& replacement, regex_constants::match_flag_type flags);

		class Impl;
		STINGRAYKIT_DECLARE_PTR(Impl);

	private:
		ImplPtr		_impl;

	public:
		explicit regex(const std::string& str);

		std::string ToString() const;
	};


	class smatch
	{
		typedef std::vector<std::string>		ResultsVec;
		typedef std::vector<off_t>				PositionsVec;


		template < typename PlatformRegex >
		friend class BasicRegexImpl;

	public:
		typedef ResultsVec::value_type			value_type;
		typedef char							char_type;
		typedef value_type						string_type;
		typedef ResultsVec::const_reference		reference;
		typedef ResultsVec::const_reference		const_reference;
		typedef ResultsVec::const_iterator		iterator;
		typedef ResultsVec::const_iterator		const_iterator;
		typedef ResultsVec::size_type			size_type;
		typedef ResultsVec::difference_type		difference_type;

	private:
		PositionsVec	_positions;
		ResultsVec		_results;
		value_type		_prefix;
		value_type		_suffix;

	public:
		const_reference operator[] (size_type n) const			{ return _results.at(n); }

		size_type size() const									{ return _results.size(); }
		size_type length(size_type n = 0) const					{ return _results.at(n).size(); }
		difference_type position(size_type n = 0) const			{ return _positions.at(n); }

		iterator begin()										{ return _results.begin(); }
		iterator end()											{ return _results.end(); }
		const_iterator begin() const							{ return _results.begin(); }
		const_iterator end() const								{ return _results.end(); }

		const_reference prefix() const							{ return _prefix; }
		const_reference suffix() const							{ return _suffix; }
		string_type str(size_type n = 0) const					{ return _results.at(n); }
	};

}

#endif
