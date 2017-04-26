#ifndef STINGRAYKIT_STRING_HEX_H
#define STINGRAYKIT_STRING_HEX_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <limits>
#include <string>
#include <vector>

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/MetaProgramming.h>
#include <stingraykit/Types.h>
#include <stingraykit/collection/array.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

	template < typename T>
	T FromHex(const std::string &str)
	{
		size_t n = str.size();
		T r = T();
		for(size_t i = 0; i < n; ++i)
		{
			char c = str[i];
			if (c >= '0' && c <= '9')
				c -= '0';
			else
			{
				c &= ~ 0x20;
				if (c >= 'A' && c <= 'F')
					c = c - 'A' + 10;
				else
					throw std::runtime_error(std::string("invalid char '") + str[i] + "' in hex string");
			}
			r |= c << ((n - i - 1) * 4);
		}
		return r;
	}


	template<typename T>
	void ToHexImpl(string_ostream &r, T value, size_t width = 0, bool capital = false)
	{
		static const size_t max_width = sizeof(T) * 2;
		size_t start;
		if (width > max_width)
		{
			for(size_t i = max_width; i < width; ++i)
				r << "0";
			start = 0;
		}
		else
			start = max_width - width;

		bool seen_non_zero = false;
		for(size_t i = 0; i < max_width; ++i)
		{
			char c = (value >> ((max_width - i - 1) * 4)) & 0x0f;
			seen_non_zero = seen_non_zero || c;
			if (seen_non_zero || i >= start || i == max_width - 1)
				r << ((char)(c > 9? c + (capital? 'A': 'a') - 10: c + '0'));
		}
	}


	template<typename T>
	std::string ToHex(T value, size_t width = 0, bool capital = false, bool add0xPrefix = false)
	{
		string_ostream result;
		if (add0xPrefix)
			result << "0x";
		ToHexImpl(result, value, width, capital);
		return result.str();
	}


	template < typename T >
	class HexFormatter
	{
		const T&	_val;
		size_t		_width;

	public:
		inline explicit HexFormatter(const T& val, size_t width)
			: _val(val), _width(width)
		{ }

		std::string ToString() const
		{
			typedef typename IntType<sizeof(T) * 8, false>::ValueT CastTo;
			CompileTimeAssert<sizeof(CastTo) >= sizeof(T)> ERROR__T_is_bigger_than_CastTo;
			CompileTimeAssert<sizeof(u64) >= sizeof(T)> ERROR__T_is_bigger_than_u64;
			(void)ERROR__T_is_bigger_than_CastTo;
			(void)ERROR__T_is_bigger_than_u64;

			string_ostream ss;
			ToHexImpl(ss, (u64)(CastTo)_val, _width);
			return ss.str();
		}
	};


	template < typename T >
	inline HexFormatter<T> Hex(const T& val, size_t width = 0)
	{ return HexFormatter<T>(val, width); }


	struct ShortHexDumpFormatter
	{
		const u8*	data;
		size_t		size;
		size_t		sizeLimit;
		ShortHexDumpFormatter(const void *data, size_t size, size_t sizeLimit = 16) : data(reinterpret_cast<const u8*>(data)), size(size), sizeLimit(sizeLimit)
		{ }

		std::string ToString() const
		{
			const u8 *src = data;
			string_ostream ss;
			ss << "{ ";
			size_t n = std::min(size, sizeLimit);
			for(size_t i = 0; i < n; ++i)
			{
				ToHexImpl(ss, (unsigned)src[i], 2);
				ss << " ";
			}
			if (n < size)
				ss << "... ";
			ss << "}";
			return ss.str();
		}
	};


	struct HexDumpFormatter
	{
		const u8*	data;
		size_t		size;
		size_t		width;
		HexDumpFormatter(const void *data, size_t size, size_t width = 16) : data(reinterpret_cast<const u8*>(data)), size(size), width(width)
		{ }

		std::string ToString() const
		{
			const u8 *src = data;
			string_ostream ss;
			for(size_t offset = 0; offset < size; offset += width, src += width)
			{
				ToHexImpl(ss, offset, 8);
				ss << ": ";
				size_t n = size - offset;
				if (n > width)
					n = width;

				size_t i;
				for(i = 0; i < n; ++i)
				{
					ToHexImpl(ss, (unsigned)src[i], 2);
					ss << " ";
				}
				if (i < width) {
					ss << std::string((width - i) * 3, ' ');
				}
				for(size_t i = 0; i < n; ++i)
				{
					ss << ((src[i] >= 0x20 && src[i] < 0x7f)? (char)src[i]: '.');
				}
				if (offset + width < size)
					ss << "\n";
			}
			return ss.str();
		}
	};


	inline HexDumpFormatter HexDump(const void* data, size_t size, size_t width = 16) { return HexDumpFormatter(data, size, width); }

	template < typename T >
	inline HexDumpFormatter HexDump(const std::vector<T>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return HexDumpFormatter(data.empty() ? NULL : &data[0], std::min(size, data.size()) * sizeof(T), width); }

	template < typename T, size_t N>
	inline HexDumpFormatter HexDump(const array<T, N>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return HexDumpFormatter(data.empty() ? NULL : &data[0], std::min(size, data.size()) * sizeof(T), width); }

	template < typename T >
	inline HexDumpFormatter HexDump(const BasicByteData<T>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return HexDumpFormatter(data.empty() ? NULL : &data[0], std::min(size, data.size()) * sizeof(T), width); }

	template < typename T >
	inline HexDumpFormatter HexDump(const BasicByteArray<T>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return HexDumpFormatter(data.empty() ? NULL : &data[0], std::min(size, data.size()) * sizeof(T), width); }


	inline ShortHexDumpFormatter ShortHexDump(const void* data, size_t size, size_t sizeLimit = 16)
	{ return ShortHexDumpFormatter(data, size, sizeLimit); }

	template < typename T >
	inline ShortHexDumpFormatter ShortHexDump(const std::vector<T>& data, size_t sizeLimit = 16)
	{ return ShortHexDumpFormatter(data.empty() ? NULL : &data[0], std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	template < typename T, size_t N>
	inline ShortHexDumpFormatter ShortHexDump(const array<T, N>& data, size_t sizeLimit = 16)
	{ return ShortHexDumpFormatter(data.empty() ? NULL : &data[0], std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	template < typename T >
	inline ShortHexDumpFormatter ShortHexDump(const BasicByteData<T>& data, size_t sizeLimit = 16)
	{ return ShortHexDumpFormatter(data.empty() ? NULL : &data[0], std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	template < typename T >
	inline ShortHexDumpFormatter ShortHexDump(const BasicByteArray<T>& data, size_t sizeLimit = 16)
	{ return ShortHexDumpFormatter(data.empty() ? NULL : &data[0], std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	/** @} */

}

#endif
