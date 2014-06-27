#ifndef STINGRAY_TOOLKIT_REGEX_H
#define STINGRAY_TOOLKIT_REGEX_H


#include <numeric>
#include <string>
#include <vector>

#include <stingray/toolkit/bind.h>
#include <stingray/toolkit/iterators.h>


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

	class regex;
	class smatch;


	struct regex_error : public Exception
	{
		regex_error() : Exception("regex error!") { }
		regex_error(const std::string& msg) : Exception("regex error: " + msg) { }
	};


	void regex_search(const std::string& str, smatch& m, const regex& re);


	class regex
	{
		friend class smatch;
		friend void regex_search(const std::string& str, smatch& m, const regex& re);

		class Impl;
		TOOLKIT_DECLARE_PTR(Impl);

	private:
		ImplPtr		_impl;

	public:
		regex(const std::string& str);
		regex(const regex& other);
		~regex();

		regex& operator = (const regex& other);
	};


	class smatch
	{
		typedef std::vector<std::string>		ResultsVec;
		typedef std::vector<off_t>				PositionsVec;

		friend class regex::Impl;

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
		const_reference operator[] (size_type n) const			{ return _results[n]; }

		size_type size() const									{ return _results.size(); }
		size_type length(size_type n = 0) const					{ return _results[n].size(); }
		difference_type position(size_type n = 0) const			{ return _positions[n]; }

		iterator begin()										{ return _results.begin(); }
		iterator end()											{ return _results.end(); }
		const_iterator begin() const							{ return _results.begin(); }
		const_iterator end() const								{ return _results.end(); }

		const_reference prefix() const							{ return _prefix; }
		const_reference suffix() const							{ return _suffix; }
		string_type str(size_type n = 0) const					{ return _results[n]; }
	};

}

#endif
