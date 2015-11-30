#ifndef STINGRAYKIT_COMPRESSION_PPM_PPMDECOMPRESSOR_H
#define STINGRAYKIT_COMPRESSION_PPM_PPMDECOMPRESSOR_H

#include <stingraykit/collection/array.h>
#include <stingraykit/compression/ArithmeticDecoder.h>
#include <stingraykit/compression/ICompressionEngine.h>
#include <stingraykit/compression/ppm/PpmModel.h>
#include <stingraykit/optional.h>

namespace stingray
{

	template <size_t BufferSizeBytes>
	class BytesToBitsBuffer
	{
		static const size_t BufferSizeBits = BufferSizeBytes * 8;

	private:
		array<u8, BufferSizeBytes> _buffer;
		size_t                     _writeOffset;
		size_t                     _readOffset;
		bool                       _dataIsContiguous;

	public:
		BytesToBitsBuffer() :
			_writeOffset(0), _readOffset(0),
			_dataIsContiguous(true)
		{ }

		size_t GetDataSizeBits() const
		{ return _dataIsContiguous ? (_writeOffset - _readOffset) : (BufferSizeBits - _readOffset + _writeOffset); }

		size_t GetFreeSizeBits() const
		{ return BufferSizeBits - GetDataSizeBits(); }

		size_t GetFreeSizeBytes() const
		{ return BitsToBytes(GetFreeSizeBits()); }

		bool GetBit()
		{
			bool result = _buffer[BitsToBytes(_readOffset)] & (1 << (7 - (_readOffset & 7)));
			++_readOffset;

			if (_readOffset == BufferSizeBits)
			{
				_readOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}
			return result;
		}

		size_t AddBytes(ConstByteData data)
		{
			size_t freeSize = _dataIsContiguous ? (BufferSizeBytes - BitsToBytes(_writeOffset)) : (BitsToBytes(_readOffset) - BitsToBytes(_writeOffset));
			size_t copySize = std::min(data.size(), freeSize);
			::memcpy(_buffer.data() + BitsToBytes(_writeOffset), data.data(), copySize);

			_writeOffset += BytesToBits(copySize);
			if (_writeOffset == BufferSizeBits)
			{
				_writeOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}
			return copySize;
		}

	private:
		static size_t BitsToBytes(size_t bits)  { return bits >> 3; }
		static size_t BytesToBits(size_t bytes) { return bytes << 3; }
	};


	template <typename PpmConfig_, typename ModelConfigList_>
	class PpmDecompressor : public virtual IDecompressor
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
		BytesToBitsBuffer<BufferSize>     _bitBuffer;
		optional<ArithmeticDecoder>       _decoder;
		bool                              _endOfData;

		// TODO: add proper output buffer
		optional<u8>                      _outputBuffer;

	public:
		PpmDecompressor(const shared_ptr<const Model>& model, const IDataSourcePtr& source) : _model(model), _source(source), _endOfData(false)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(bind(&PpmDecompressor::DoProcess, this, ref(consumer), _1, _2), bind(&PpmDecompressor::DoEndOfData, this, ref(consumer), _1), token); }

	private:
		size_t DoProcess(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			size_t consumed = ProcessInput(data);
			while (_bitBuffer.GetDataSizeBits() >= MaxBitsPerSymbol && !_endOfData)
			{
				if (!ProcessOutput(consumer, token))
					break;

				Decode();

				consumed += ProcessInput(ConstByteData(data, consumed));
			}
			return consumed;
		}

		bool DoEndOfData(IDataConsumer& consumer, const ICancellationToken& token)
		{
			while (!_endOfData)
			{
				if (!ProcessOutput(consumer, token))
					return false;

				Decode();
			}
			consumer.EndOfData(token);
			return true;
		}

		size_t ProcessInput(ConstByteData data)
		{
			size_t consumed = 0;
			while (consumed != data.size() && _bitBuffer.GetFreeSizeBytes() != 0)
			{
				size_t added = _bitBuffer.AddBytes(ConstByteData(data, consumed));
				if (added == 0)
					break;
				consumed += added;
			}
			return consumed;
		}

		bool ProcessOutput(IDataConsumer& consumer, const ICancellationToken& token)
		{
			if (_outputBuffer)
			{
				if (consumer.Process(ConstByteData(&*_outputBuffer, 1), token) != 1)
					return false;
				_outputBuffer.reset();
			}
			return true;
		}

		void Decode()
		{
			if (!_decoder)
			{
				_decoder.emplace(bind(&PpmDecompressor::GetBit, this));
				return;
			}

			STINGRAYKIT_CHECK(!_endOfData, "Data after EOD!");
			optional<typename Impl::Symbol> symbol = _model->Decode(_context, *_decoder, bind(&PpmDecompressor::GetBit, this));

			_endOfData = !symbol;
			if (!symbol)
				return;

			_context.push_back(*symbol);
			if (_context.size() > Model::ContextSize)
				_context.pop_front();
			_outputBuffer = *symbol;
		}

		bool GetBit()
		{ return _bitBuffer.GetBit(); }
	};

}

#endif
