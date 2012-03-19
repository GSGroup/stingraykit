#ifndef __GS_DVRLIB_TOOLKIT_BYTESTREAM_H__
#define __GS_DVRLIB_TOOLKIT_BYTESTREAM_H__

#include <vector>

#include <dvrlib/toolkit/toolkit.h>
#include <dvrlib/toolkit/exception.h>

namespace dvrlib
{
	
	namespace Detail
	{

		template < typename T >
		struct ByteStreamSerializer
		{
			template < typename OutputIterator >
			static void Serialize(OutputIterator& it, const T& val)
			{
				CompileTimeAssert < IsFixedWidthIntType<T>::Value > ERROR_Only_Fixed_Width_Integers_Can_Be_Serialized_Safely;

				for (size_t i = 0; i < sizeof(T); ++i, ++it)
					*it = (val >> 8 * (sizeof(T) - i - 1)) & 0xFF;
			}

			template < typename InputIterator >
			static void Deserialize(InputIterator& it, T& value)
			{
				CompileTimeAssert < IsFixedWidthIntType<T>::Value > ERROR_Only_Fixed_Width_Integers_Can_Be_Deserialized_Safely;

				value = T();
				for (size_t i = 0; i < sizeof(T); ++i, ++it)
					value = (value << 8) | *it;
			}
		};

	}

	class ByteOStream
	{
		TOOLKIT_NONCOPYABLE(ByteOStream);

	private:
		std::vector<u8>	_data;

	public:
		ByteOStream() { }

		void Clear()							{ _data.clear(); }
		const std::vector<u8>& GetData() const	{ return _data; }
		
		template < typename T >
		ByteOStream& operator << (const T& val)
		{
			std::back_insert_iterator<std::vector<u8> > out = std::back_inserter(_data);
			Detail::ByteStreamSerializer<T>::template Serialize(out, val);
			return *this;
		}

		template<typename IteratorType>
		ByteOStream& Write(IteratorType from, const IteratorType& to)
		{
			std::copy(from, to, std::back_inserter(_data));
			return *this;
		}
	};

	class ByteIStream
	{
		TOOLKIT_NONCOPYABLE(ByteIStream);

		typedef std::vector<u8>::const_iterator iterator_type;
		typedef std::iterator_traits<iterator_type> iterator_traits;

	private:
		iterator_type _it;
		iterator_type _begin;
		iterator_type _end;

	public:
		ByteIStream(const std::vector<u8>& data) 
			: _it(data.begin()), _begin(data.begin()), _end(data.end())
		{ }
		
		template < typename T >
		ByteIStream& operator >> (T& val)
		{
			TOOLKIT_INDEX_CHECK(std::distance(_it, _end) >= (iterator_traits::difference_type)sizeof(T));

			Detail::ByteStreamSerializer<T>::template Deserialize(_it, val);
			return *this;
		}

		template < typename IteratorType >
		void Read(IteratorType it, size_t count)
		{
			TOOLKIT_INDEX_CHECK(std::distance(_it, _end) >= (iterator_traits::difference_type)count);

			std::copy(_it, _it + count, it);
			_it += count;
		}

		void Ignore(size_t count)
		{
			TOOLKIT_INDEX_CHECK(std::distance(_it, _end) >= (iterator_traits::difference_type)count);

			_it += count;
		}
	};

}

#endif
