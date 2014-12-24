#ifndef STINGRAY_TOOLKIT_STRING_HEXDUMP_H
#define STINGRAY_TOOLKIT_STRING_HEXDUMP_H

#include <vector>
#include <limits>

#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/collection/ByteData.h>
#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/Types.h>
#include <stingray/toolkit/collection/array.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

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
