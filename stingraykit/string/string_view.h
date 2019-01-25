#ifndef STINGRAYKIT_STRING_STRING_VIEW_H
#define STINGRAYKIT_STRING_STRING_VIEW_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

#include <limits>
#include <string>
#include <string.h>

namespace stingray
{

	template < typename CharT, typename Traits = std::char_traits<CharT> >
	class basic_string_view
	{
	public:
		typedef Traits												traits_type;
		typedef CharT												value_type;
		typedef CharT*												pointer;
		typedef const CharT*										const_pointer;
		typedef CharT&												reference;
		typedef const CharT&										const_reference;
		typedef const CharT*										const_iterator;
		typedef const_iterator										iterator;
		typedef std::reverse_iterator<const_iterator>				const_reverse_iterator;
		typedef const_reverse_iterator								reverse_iterator;
		typedef size_t												size_type;
		typedef ptrdiff_t											difference_type;

		static const size_type npos = -1;

	private:
		const CharT*			_data;
		size_type				_size;

	public:
		basic_string_view() : _data(null), _size(0) { }
		basic_string_view(const CharT* s, size_type count) : _data(s), _size(count) { }
		basic_string_view(const CharT* s) : _data(s), _size(Traits::length(s)) { }
		basic_string_view(const std::string& str) : _data(str.data()), _size(str.size()) { }

		const_iterator begin() const					{ return _data; }
		const_iterator end() const						{ return _data + size(); }
		const_reverse_iterator rbegin() const			{ return reverse_iterator(end()); }
		const_reverse_iterator rend() const				{ return reverse_iterator(begin()); }

		const_reference at(size_type pos) const			{ STINGRAYKIT_CHECK_RANGE(pos, size()); return _data[pos]; }
		const_reference operator[](size_type pos) const { return _data[pos]; }
		const_reference front() const					{ return _data[0]; }
		const_reference back() const					{ return _data[size() - 1]; }
		const_pointer data() const						{ return _data; }

		size_type size() const							{ return _size; }
		size_type length() const						{ return size(); }
		size_type max_size() const						{ return std::numeric_limits<size_type>::max(); }
		bool empty() const								{ return !size(); }

		void remove_prefix(size_type n)
		{
			STINGRAYKIT_CHECK_RANGE(n, size());
			_data += n;
			_size -= n;
		}

		void remove_suffix(size_type n)
		{
			STINGRAYKIT_CHECK_RANGE(n, size());
			_size -= n;
		}

		void swap(basic_string_view& other)
		{
			std::swap(_data, other._data);
			std::swap(_size, other._size);
		}

		std::basic_string<CharT, Traits> copy() const
		{ return std::basic_string<CharT, Traits>(_data, size()); }

		basic_string_view substr(size_type pos = 0, size_type count = npos) const
		{
			STINGRAYKIT_CHECK_RANGE(pos, size());
			return basic_string_view(_data + pos, std::min(count, size() - pos));
		}

		int compare(basic_string_view v) const
		{
			const int result = Traits::compare(data(), v.data(), std::min(size(), v.size()));
			return result == 0 ? size() - v.size() : result;
		}

		int compare(size_type pos1, size_type count1, basic_string_view v) const									{ return substr(pos1, count1).compare(v); }
		int compare(size_type pos1, size_type count1, basic_string_view v, size_type pos2, size_type count2) const	{ return substr(pos1, count1).compare(v.substr(pos2, count2)); }
		int compare(const CharT* s) const																			{ return compare(basic_string_view(s)); }
		int compare(size_type pos1, size_type count1, const CharT* s) const											{ return substr(pos1, count1).compare(basic_string_view(s)); }
		int compare(size_type pos1, size_type count1, const CharT* s, size_type count2) const						{ return substr(pos1, count1).compare(basic_string_view(s, count2)); }

		bool starts_with(basic_string_view x) const	{ return size() >= x.size() && compare(0, x.size(), x) == 0; }
		bool starts_with(CharT x) const				{ return starts_with(basic_string_view(&x, 1)); }
		bool starts_with(const CharT* x) const		{ return starts_with(basic_string_view(x)); }

		bool ends_with(basic_string_view x) const	{ return size() >= x.size() && compare(size() - x.size(), npos, x) == 0; }
		bool ends_with(CharT x) const				{ return ends_with(basic_string_view(&x, 1)); }
		bool ends_with(const CharT* x) const		{ return ends_with(basic_string_view(x)); }

		size_type find(basic_string_view v, size_type pos = 0) const
		{
			if (pos > size())
				return npos;

			if (v.empty())
				return pos;

			if (v.size() > size() - pos)
				return npos;

			for (; pos < size() - v.size() + 1; ++pos)
				if (basic_string_view(_data + pos, v.size()).compare(v) == 0)
					return pos;

			return npos;
		}

		size_type find(CharT ch, size_type pos = 0) const						{ return find(basic_string_view(&ch, 1), pos); }
		size_type find(const CharT* s, size_type pos, size_type count) const	{ return find(basic_string_view(s, count), pos); }
		size_type find(const CharT* s, size_type pos = 0) const					{ return find(basic_string_view(s), pos); }

		size_type rfind(basic_string_view v, size_type pos = npos) const
		{
			if (v.empty())
				return pos > size() ? size() : pos;

			if (v.size() > size())
				return npos;

			pos = std::min(pos, size() - v.size());

			for (size_type i = 0; i <= pos; ++i)
				if (basic_string_view(_data + pos - i, v.size()).compare(v) == 0)
					return pos - i;

			return npos;
		}

		size_type rfind(CharT ch, size_type pos = npos) const					{ return rfind(basic_string_view(&ch, 1), pos); }
		size_type rfind(const CharT* s, size_type pos, size_type count) const	{ return rfind(basic_string_view(s, count), pos); }
		size_type rfind(const CharT* s, size_type pos = npos) const				{ return rfind(basic_string_view(s), pos); }

		size_type find_first_of(basic_string_view v, size_type pos = 0) const
		{
			if (v.empty())
				return npos;

			for (; pos < size(); ++pos)
				for (const_iterator it = v.begin(); it != v.end(); ++it)
					if (_data[pos] == *it)
						return pos;

			return npos;
		}

		size_type find_first_of(CharT c, size_type pos = 0) const						{ return find_first_of(basic_string_view(&c, 1), pos); }
		size_type find_first_of(const CharT* s, size_type pos, size_type count) const	{ return find_first_of(basic_string_view(s, count), pos); }
		size_type find_first_of(const CharT* s, size_type pos = 0) const				{ return find_first_of(basic_string_view(s), pos); }

		size_type find_last_of(basic_string_view v, size_type pos = npos) const
		{
			if (v.empty())
				return npos;

			if (pos == npos)
				pos = size() - 1;

			for (size_type i = 0; i <= pos; ++i)
				for (const_iterator it = v.begin(); it != v.end(); ++it)
					if (_data[pos - i] == *it)
						return pos - i;

			return npos;
		}

		size_type find_last_of(CharT c, size_type pos = npos) const						{ return find_last_of(basic_string_view(&c, 1), pos); }
		size_type find_last_of(const CharT* s, size_type pos, size_type count) const	{ return find_last_of(basic_string_view(s, count), pos); }
		size_type find_last_of(const CharT* s, size_type pos = npos) const				{ return find_last_of(basic_string_view(s), pos); }

		size_type find_first_not_of(basic_string_view v, size_type pos = 0) const
		{
			if (v.empty())
				return 0;

			for (; pos < size(); ++pos)
			{
				bool contains = false;
				for (const_iterator it = v.begin(); it != v.end(); ++it)
				{
					if (_data[pos] == *it)
					{
						contains = true;
						break;
					}
				}
				if (!contains)
					return pos;
			}

			return npos;
		}

		size_type find_first_not_of(CharT c, size_type pos = 0) const						{ return find_first_not_of(basic_string_view(&c, 1), pos); }
		size_type find_first_not_of(const CharT* s, size_type pos, size_type count) const	{ return find_first_not_of(basic_string_view(s, count), pos); }
		size_type find_first_not_of(const CharT* s, size_type pos = 0) const				{ return find_first_not_of(basic_string_view(s), pos); }

		size_type find_last_not_of(basic_string_view v, size_type pos = npos) const
		{
			if (v.empty())
				return npos;

			if (pos == npos)
				pos = size() - 1;

			for (size_type i = 0; i <= pos; ++i)
			{
				bool contains = false;
				for (const_iterator it = v.begin(); it != v.end(); ++it)
				{
					if (_data[pos - i] == *it)
					{
						contains = true;
						break;
					}
				}
				if (!contains)
					return pos - i;
			}

			return npos;
		}

		size_type find_last_not_of(CharT c, size_type pos = npos) const						{ return find_last_not_of(basic_string_view(&c, 1), pos); }
		size_type find_last_not_of(const CharT* s, size_type pos, size_type count) const	{ return find_last_not_of(basic_string_view(s, count), pos); }
		size_type find_last_not_of(const CharT* s, size_type pos = npos) const				{ return find_last_not_of(basic_string_view(s), pos); }

		bool operator == (basic_string_view<CharT,Traits> rhs) const	{ return compare(rhs) == 0; }
		bool operator != (basic_string_view<CharT,Traits> rhs) const	{ return compare(rhs) != 0; }
		bool operator <  (basic_string_view<CharT,Traits> rhs) const	{ return compare(rhs) < 0; }
		bool operator <= (basic_string_view<CharT,Traits> rhs) const	{ return compare(rhs) <= 0; }
		bool operator >  (basic_string_view<CharT,Traits> rhs) const	{ return compare(rhs) > 0; }
		bool operator >= (basic_string_view<CharT,Traits> rhs) const	{ return compare(rhs) >= 0; }
	};




	template < typename CharT, typename Traits >
	void swap(basic_string_view<CharT, Traits>& lhs, basic_string_view<CharT, Traits>& rhs)
	{ lhs.swap(rhs); }


	typedef basic_string_view<char> string_view;

}

#endif
