#ifndef STINGRAYKIT_STRING_HEX_H
#define STINGRAYKIT_STRING_HEX_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>

#include <limits>
#include <string>
#include <vector>

namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

	template < typename T, typename StringType >
	typename EnableIf<!IsSame<T, ByteArray>::Value, T>::ValueT FromHex(const StringType& str)
	{
		const size_t n = str.size();
		T r = T();

		for (size_t i = 0; i < n; ++i)
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
					STINGRAYKIT_THROW(FormatException(std::string("invalid char '") + str[i] + "' in hex string"));
			}
			r |= c << ((n - i - 1) * 4);
		}

		return r;
	}


	template < typename T, typename StringType >
	inline typename EnableIf<IsSame<T, ByteArray>::Value, T>::ValueT FromHex(const StringType& str)
	{
		const std::string::size_type n = str.size();
		const ByteArray::CollectionTypePtr r = make_shared_ptr<ByteArray::CollectionType>();
		std::string::size_type i = 0;

		const std::string prefix("0x");
		if (n >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0)
			i += prefix.size();

		if ((n - i) % 2 == 1)
		{
			r->push_back(FromHex<ByteArray::value_type>(str.substr(i, 1)));
			++i;
		}

		for (; i < n; i += 2)
			r->push_back(FromHex<ByteArray::value_type>(str.substr(i, 2)));

		return r;
	}


	template < typename T >
	typename EnableIf<!IsConvertible<T, ConstByteData>::Value, void>::ValueT ToHexImpl(string_ostream& r, T value, size_t width = 0, bool capital = false)
	{
		static const size_t maxWidth = sizeof(T) * 2;
		size_t start;
		if (width > maxWidth)
		{
			for (size_t i = maxWidth; i < width; ++i)
				r << "0";
			start = 0;
		}
		else
			start = maxWidth - width;

		bool seenNonZero = false;
		for (size_t i = 0; i < maxWidth; ++i)
		{
			char c = (value >> ((maxWidth - i - 1) * 4)) & 0x0f;
			seenNonZero = seenNonZero || c;
			if (seenNonZero || i >= start || i == maxWidth - 1)
				r << ((char)(c > 9 ? c + (capital? 'A' : 'a') - 10 : c + '0'));
		}
	}


	template < typename T >
	typename EnableIf<IsConvertible<T, ConstByteData>::Value, void>::ValueT ToHexImpl(string_ostream& r, T value, size_t width = 0, bool capital = false)
	{
		ConstByteData data(value);
		const size_t maxWidth = data.size() * sizeof(ConstByteData::value_type) * 2;
		for (size_t i = maxWidth; i < width; ++i)
			r << "0";

		for (ConstByteData::const_iterator it = data.begin(); it != data.end(); ++it)
			ToHexImpl(r, *it, sizeof(ConstByteData::value_type) * 2, capital);
	}


	template < typename T >
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


	class ShortHexDumpFormatter
	{
	private:
		const u8*	data;
		size_t		size;
		size_t		sizeLimit;

	public:
		ShortHexDumpFormatter(const void* data, size_t size, size_t sizeLimit = 16)
			: data(reinterpret_cast<const u8*>(data)), size(size), sizeLimit(sizeLimit)
		{ }

		std::string ToString() const
		{
			const u8 *src = data;
			string_ostream ss;
			ss << "{ ";
			size_t n = std::min(size, sizeLimit);
			for (size_t i = 0; i < n; ++i)
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


	class HexDumpFormatter
	{
	private:
		const u8*	data;
		size_t		size;
		size_t		width;

	public:
		HexDumpFormatter(const void *data, size_t size, size_t width = 16)
			: data(reinterpret_cast<const u8*>(data)), size(size), width(width)
		{ }

		std::string ToString() const
		{
			const u8 *src = data;
			string_ostream ss;
			for (size_t offset = 0; offset < size; offset += width, src += width)
			{
				ToHexImpl(ss, offset, 8);
				ss << ": ";
				size_t n = size - offset;
				if (n > width)
					n = width;

				size_t i;
				for (i = 0; i < n; ++i)
				{
					ToHexImpl(ss, (unsigned)src[i], 2);
					ss << " ";
				}
				if (i < width) {
					ss << std::string((width - i) * 3, ' ');
				}
				for (size_t i = 0; i < n; ++i)
				{
					ss << ((src[i] >= 0x20 && src[i] < 0x7f)? (char)src[i]: '.');
				}
				if (offset + width < size)
					ss << "\n";
			}
			return ss.str();
		}
	};


	inline HexDumpFormatter HexDump(const void* data, size_t size, size_t width = 16)
	{ return HexDumpFormatter(data, size, width); }

	template < typename T >
	inline HexDumpFormatter HexDump(const std::vector<T>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return HexDumpFormatter(data.empty() ? NULL : &data[0], std::min(size, data.size()) * sizeof(T), width); }

	template < typename T, size_t N >
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

	template < typename T, size_t N >
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
