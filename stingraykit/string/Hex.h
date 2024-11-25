#ifndef STINGRAYKIT_STRING_HEX_H
#define STINGRAYKIT_STRING_HEX_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

	template < typename T >
	typename EnableIf<!IsSame<T, ByteArray>::Value, T>::ValueT FromHex(string_view str)
	{
		STINGRAYKIT_CHECK(!str.empty(), ArgumentException("str"));

		const T MaxValueForShift = std::numeric_limits<T>::max() >> 4;

		const string_view::size_type size = str.size();
		T result = T();

		for (string_view::size_type index = 0; index < size; ++index)
		{
			const char ch = str[index];
			STINGRAYKIT_CHECK((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'), FormatException(str.copy()));
		}

		for (string_view::size_type index = 0; index < size; ++index)
		{
			char ch = str[index];

			if (ch >= '0' && ch <= '9')
				ch -= '0';
			else
			{
				ch &= ~ 0x20;
				ch = ch - 'A' + 10;
			}

			if (index > 0)
			{
				STINGRAYKIT_CHECK(result <= MaxValueForShift, IndexOutOfRangeException(str, "0", std::string(sizeof(T) * 2, 'f')));
				result <<= 4;
			}

			result |= ch;
		}

		return result;
	}


	template < typename T >
	typename EnableIf<IsSame<T, ByteArray>::Value, T>::ValueT FromHex(string_view str)
	{
		const ByteArray::CollectionTypePtr result = make_shared_ptr<ByteArray::CollectionType>();

		const string_view::size_type size = str.size();
		string_view::size_type index = 0;

		const string_view prefix("0x");
		if (size >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0)
			index += prefix.size();

		try
		{
			if ((size - index) % 2 == 1)
			{
				result->push_back(FromHex<ByteArray::value_type>(str.substr(index, 1)));
				++index;
			}

			for (; index < size; index += 2)
				result->push_back(FromHex<ByteArray::value_type>(str.substr(index, 2)));
		}
		catch (const FormatException&)
		{ STINGRAYKIT_THROW(FormatException(str.copy())); }

		return result;
	}


	namespace Detail
	{

		template < typename T >
		typename EnableIf<!IsConvertible<T, ConstByteData>::Value, void>::ValueT ToHexImpl(string_ostream& result, T value, size_t width = 0, bool capital = false)
		{
			const size_t maxWidth = sizeof(T) * 2;
			size_t start = 0;

			if (width > maxWidth)
			{
				for (size_t index = maxWidth; index < width; ++index)
					result << "0";
			}
			else
				start = maxWidth - width;

			bool seenNonZero = false;
			for (size_t index = 0; index < maxWidth; ++index)
			{
				const char ch = (value >> ((maxWidth - index - 1) * 4)) & 0x0f;
				seenNonZero = seenNonZero || ch;

				if (seenNonZero || index >= start || index == maxWidth - 1)
					result << ((char)(ch > 9 ? ch + (capital ? 'A' : 'a') - 10 : ch + '0'));
			}
		}

		template < typename T >
		typename EnableIf<IsConvertible<T, ConstByteData>::Value, void>::ValueT ToHexImpl(string_ostream& result, T value, size_t width = 0, bool capital = false)
		{
			const ConstByteData data(value);

			const size_t maxWidth = data.size() * sizeof(ConstByteData::value_type) * 2;
			for (size_t index = maxWidth; index < width; ++index)
				result << "0";

			for (ConstByteData::value_type byte : data)
				ToHexImpl(result, byte, sizeof(ConstByteData::value_type) * 2, capital);
		}

	}


	template < typename T >
	std::string ToHex(T value, size_t width = 0, bool capital = false, bool add0xPrefix = false)
	{
		string_ostream result;

		if (add0xPrefix)
			result << "0x";

		Detail::ToHexImpl(result, value, width, capital);
		return result.str();
	}


	namespace Detail
	{

		template < typename T >
		class HexFormatter
		{
			const T&			_value;
			size_t				_width;

		public:
			HexFormatter(const T& value, size_t width)
				: _value(value), _width(width)
			{ }

			std::string ToString() const
			{
				using CastTo = typename IntType<sizeof(T) * 8, false>::ValueT;
				static_assert(sizeof(CastTo) >= sizeof(T), "T is bigger than CastTo");
				static_assert(sizeof(u64) >= sizeof(T), "T is bigger than u64");

				string_ostream result;
				ToHexImpl(result, (u64)(CastTo)_value, _width);
				return result.str();
			}
		};

	}


	template < typename T >
	Detail::HexFormatter<T> Hex(const T& val, size_t width = 0)
	{ return Detail::HexFormatter<T>(val, width); }


	namespace Detail
	{

		class ShortHexDumpFormatter
		{
		private:
			const u8*			_data;
			size_t				_size;
			size_t				_sizeLimit;

		public:
			ShortHexDumpFormatter(const void* data, size_t size, size_t sizeLimit = 16)
				: _data(reinterpret_cast<const u8*>(data)), _size(size), _sizeLimit(sizeLimit)
			{ }

			std::string ToString() const
			{
				const u8* src = _data;

				string_ostream result;
				result << "{ ";

				const size_t size = std::min(_size, _sizeLimit);
				for (size_t index = 0; index < size; ++index)
				{
					ToHexImpl(result, (unsigned)src[index], 2);
					result << " ";
				}

				if (size < _size)
					result << "... ";

				result << "}";
				return result.str();
			}
		};

		class HexDumpFormatter
		{
		private:
			const u8*			_data;
			size_t				_size;
			size_t				_width;

		public:
			HexDumpFormatter(const void* data, size_t size, size_t width = 16)
				: _data(reinterpret_cast<const u8*>(data)), _size(size), _width(width)
			{ }

			std::string ToString() const
			{
				const u8* src = _data;

				string_ostream result;

				for (size_t offset = 0; offset < _size; offset += _width, src += _width)
				{
					ToHexImpl(result, offset, 8);
					result << ": ";

					const size_t size = std::min(_size - offset, _width);
					size_t index = 0;

					for (; index < size; ++index)
					{
						ToHexImpl(result, (unsigned)src[index], 2);
						result << " ";
					}

					if (index < _width)
						result << std::string((_width - index) * 3, ' ');

					for (size_t index2 = 0; index2 < size; ++index2)
						result << ((src[index2] >= 0x20 && src[index2] < 0x7f) ? (char)src[index2] : '.');

					if (offset + _width < _size)
						result << "\n";
				}

				return result.str();
			}
		};

	}


	inline Detail::HexDumpFormatter HexDump(const void* data, size_t size, size_t width = 16)
	{ return Detail::HexDumpFormatter(data, size, width); }

	template < typename T >
	Detail::HexDumpFormatter HexDump(const std::vector<T>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return Detail::HexDumpFormatter(data.data(), std::min(size, data.size()) * sizeof(T), width); }

	template < typename T, size_t N >
	Detail::HexDumpFormatter HexDump(const array<T, N>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return Detail::HexDumpFormatter(data.data(), std::min(size, data.size()) * sizeof(T), width); }

	template < typename T >
	Detail::HexDumpFormatter HexDump(const BasicByteData<T>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return Detail::HexDumpFormatter(data.data(), std::min(size, data.size()) * sizeof(T), width); }

	template < typename T >
	Detail::HexDumpFormatter HexDump(const BasicByteArray<T>& data, size_t size = std::numeric_limits<size_t>::max(), size_t width = 16)
	{ return Detail::HexDumpFormatter(data.data(), std::min(size, data.size()) * sizeof(T), width); }


	inline Detail::ShortHexDumpFormatter ShortHexDump(const void* data, size_t size, size_t sizeLimit = 16)
	{ return Detail::ShortHexDumpFormatter(data, size, sizeLimit); }

	template < typename T >
	Detail::ShortHexDumpFormatter ShortHexDump(const std::vector<T>& data, size_t sizeLimit = 16)
	{ return Detail::ShortHexDumpFormatter(data.data(), std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	template < typename T, size_t N >
	Detail::ShortHexDumpFormatter ShortHexDump(const array<T, N>& data, size_t sizeLimit = 16)
	{ return Detail::ShortHexDumpFormatter(data.data(), std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	template < typename T >
	Detail::ShortHexDumpFormatter ShortHexDump(const BasicByteData<T>& data, size_t sizeLimit = 16)
	{ return Detail::ShortHexDumpFormatter(data.data(), std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	template < typename T >
	Detail::ShortHexDumpFormatter ShortHexDump(const BasicByteArray<T>& data, size_t sizeLimit = 16)
	{ return Detail::ShortHexDumpFormatter(data.data(), std::min(sizeLimit, data.size() * sizeof(T)), sizeLimit); }

	/** @} */

}

#endif
