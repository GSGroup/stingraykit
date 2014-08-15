#ifndef STINGRAY_TOOLKIT_BITHREADCIRCULARBUFFER_H
#define STINGRAY_TOOLKIT_BITHREADCIRCULARBUFFER_H


#include <stingray/toolkit/ByteData.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/toolkit.h>

namespace stingray
{

	struct BithreadCircularBuffer
	{
		TOOLKIT_NONCOPYABLE(BithreadCircularBuffer);

	private:
		struct Impl;
		TOOLKIT_DECLARE_PTR(Impl);

		ImplPtr	_impl;

	public:
		struct Reader
		{
		private:
			ImplPtr			_impl;
			ConstByteData	_data;

		public:
			Reader(const ImplPtr& impl);
			Reader(const Reader&);
			~Reader();

			Reader& operator =(const Reader&);

			size_t size() const;
			ConstByteData::iterator begin();
			ConstByteData::iterator end();

			ConstByteData GetData() const;

			bool IsBufferEnd() const;

			void Pop(size_t bytes);
		};

		struct Writer
		{
		private:
			ImplPtr 	_impl;
			ByteData	_data;

		public:
			Writer(const ImplPtr& impl);
			Writer(const Writer&);
			~Writer();

			Writer& operator =(const Writer&);

			size_t size() const;
			ByteData::iterator begin();
			ByteData::iterator end();

			ByteData GetData() const;

			bool IsBufferEnd() const;

			void Push(size_t bytes);
		};

	public:
		BithreadCircularBuffer(size_t size);
		~BithreadCircularBuffer();

		size_t GetDataSize() const;
		size_t GetFreeSize() const;

		size_t GetTotalSize() const;

		Reader Read();
		Writer Write();
	};
	TOOLKIT_DECLARE_PTR(BithreadCircularBuffer);

}

#endif
