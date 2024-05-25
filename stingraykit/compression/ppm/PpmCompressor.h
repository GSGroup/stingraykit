#ifndef STINGRAYKIT_COMPRESSION_PPM_PPMCOMPRESSOR_H
#define STINGRAYKIT_COMPRESSION_PPM_PPMCOMPRESSOR_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/array.h>
#include <stingraykit/compression/ArithmeticCoder.h>
#include <stingraykit/compression/ICompressionEngine.h>
#include <stingraykit/compression/ppm/PpmModel.h>

#include <deque>
#include <string.h>

namespace stingray
{

	template <size_t BufferSizeBytes>
	class BitsToBytesBuffer
	{
		static const size_t BufferSizeBits = BufferSizeBytes * 8;

	private:
		array<u8, BufferSizeBytes>  _buffer;
		size_t                 _writeOffset;
		size_t                 _readOffset;
		bool                   _dataIsContiguous;

	public:
		BitsToBytesBuffer() :
			_writeOffset(0), _readOffset(0),
			_dataIsContiguous(true)
		{ std::fill(_buffer.begin(), _buffer.end(), 0); }

		size_t GetDataSizeBits() const
		{ return _dataIsContiguous ? (_writeOffset - _readOffset) : (BufferSizeBits - _readOffset + _writeOffset); }

		size_t GetFreeSizeBits() const
		{ return BufferSizeBits - GetDataSizeBits(); }

		void AddBit(bool bit)
		{
			_buffer[BitsToBytes(_writeOffset)] |= (bit << (7 - (_writeOffset & 7)));
			++_writeOffset;

			if (_writeOffset == BufferSizeBits)
			{
				_writeOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}
		}

		template <typename DataConsumer_>
		bool ConsumeBytes(const DataConsumer_& consumer)
		{ return DoConsume(consumer); }

		template <typename DataConsumer_>
		bool EndOfData(const DataConsumer_& consumer)
		{
			_writeOffset = BytesToBits(BitsToBytes(_writeOffset + 7));
			while (GetDataSizeBits() != 0)
				if (!DoConsume(consumer))
					return false;
			return true;
		}

	private:
		static size_t BitsToBytes(size_t bits)  { return bits >> 3; }
		static size_t BytesToBits(size_t bytes) { return bytes << 3; }

		template <typename DataConsumer_>
		bool DoConsume(const DataConsumer_& consumer)
		{
			size_t size = _dataIsContiguous ? (BitsToBytes(_writeOffset) - BitsToBytes(_readOffset)) : (BufferSizeBytes - BitsToBytes(_readOffset));
			size_t consumedSize = consumer(ConstByteData(_buffer, BitsToBytes(_readOffset), size));

			::memset(_buffer.data() + BitsToBytes(_readOffset), 0, consumedSize);

			_readOffset += BytesToBits(consumedSize);
			if (_readOffset == BufferSizeBits)
			{
				_readOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}
			return consumedSize != 0;
		}
	};


	template <typename PpmConfig_, typename ModelConfigList_>
	class PpmCompressor : public virtual ICompressor
	{
		static const size_t ModelsCount = GetTypeListLength<ModelConfigList_>::Value;
		static const size_t BufferSize = 1024;
		static const size_t MaxBitsPerSymbol = ModelsCount * 32; // actually, it is ModelsCount * log(MaxProbabilityScale) / log(2), but better be safe

		typedef PpmImpl<PpmConfig_>                             Impl;
		typedef typename Impl::template Model<ModelConfigList_> Model;

	private:
		shared_ptr<const Model>           _model;
		IDataSourcePtr                    _source;
		std::deque<typename Impl::Symbol> _context;
		BitsToBytesBuffer<BufferSize>     _bitBuffer;
		ArithmeticCoder                   _coder;

	public:
		PpmCompressor(const shared_ptr<const Model>& model, const IDataSourcePtr& source) : _model(model), _source(source)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(Bind(&PpmCompressor::DoProcess, this, wrap_ref(consumer), _1, _2), Bind(&PpmCompressor::DoEndOfData, this, wrap_ref(consumer), _1), token); }

	private:
		size_t DoProcess(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			size_t offset = 0;
			for ( ; offset != data.size(); ++offset)
				if (!Encode(consumer, data[offset], token))
					break;
			return offset;
		}

		void DoEndOfData(IDataConsumer& consumer, const ICancellationToken& token)
		{
			if (!ConsumeData(consumer, token))
				return;

			_model->Predict(_context, null, Bind(&PpmCompressor::DoEncode, this, _1, _2, _3));

			_coder.EndOfData(Bind(&PpmCompressor::AddBit, this, _1));
			if (!_bitBuffer.EndOfData(Bind(&IDataConsumer::Process, &consumer, _1, wrap_const_ref(token))))
				return;
			consumer.EndOfData(token);
		}

		bool Encode(IDataConsumer& consumer, typename Impl::Symbol symbol, const ICancellationToken& token)
		{
			if (!ConsumeData(consumer, token))
				return false;

			_model->Predict(_context, symbol, Bind(&PpmCompressor::DoEncode, this, _1, _2, _3));

			_context.push_back(symbol);
			if (_context.size() > Model::ContextSize)
				_context.pop_front();
			return true;
		}

		void DoEncode(u32 symbolLow, u32 symbolHigh, u32 scale)
		{ _coder.Encode(symbolLow, symbolHigh, scale, Bind(&PpmCompressor::AddBit, this, _1)); }

		void AddBit(bool bit)
		{ _bitBuffer.AddBit(bit); }

		bool ConsumeData(IDataConsumer& consumer, const ICancellationToken& token)
		{
			while (_bitBuffer.GetFreeSizeBits() < MaxBitsPerSymbol)
				if (!_bitBuffer.ConsumeBytes(Bind(&IDataConsumer::Process, &consumer, _1, wrap_const_ref(token))))
					return false;
			return true;
		}
	};

}

#endif
