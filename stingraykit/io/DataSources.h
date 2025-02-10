#ifndef STINGRAYKIT_IO_DATASOURCES_H
#define STINGRAYKIT_IO_DATASOURCES_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class InterceptingDataConsumer final : public virtual IDataConsumer
	{
		using ProcessFunctionType = function<void (ConstByteData, const ICancellationToken&)>;
		using EodFunctionType = function<void (const ICancellationToken&)>;

	private:
		IDataConsumer&					_consumer;
		ProcessFunctionType				_processFunc;
		EodFunctionType					_eodFunc;

	public:
		InterceptingDataConsumer(IDataConsumer& consumer, const ProcessFunctionType& processFunc, const EodFunctionType& eodFunc)
			: _consumer(consumer), _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		size_t Process(ConstByteData data, const ICancellationToken& token) override
		{
			const size_t size = _consumer.Process(data, token);
			_processFunc(ConstByteData(data, 0, size), token);
			return size;
		}

		void EndOfData(const ICancellationToken& token) override
		{
			_consumer.EndOfData(token);
			_eodFunc(token);
		}
	};


	class InterceptingDataSource final : public virtual IDataSource
	{
		using ProcessFunctionType = function<void (ConstByteData, const ICancellationToken&)>;
		using EodFunctionType = function<void (const ICancellationToken&)>;

	private:
		IDataSourcePtr					_source;
		ProcessFunctionType				_processFunc;
		EodFunctionType					_eodFunc;

	public:
		InterceptingDataSource(const IDataSourcePtr& source, const ProcessFunctionType& processFunc, const EodFunctionType& eodFunc)
			: _source(STINGRAYKIT_REQUIRE_NOT_NULL(source)), _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		void Read(IDataConsumer& c, const ICancellationToken& token) override
		{ _source->ReadToFunction(Bind(&InterceptingDataSource::DoPush, this, wrap_ref(c), _1, _2), Bind(&InterceptingDataSource::EndOfData, this, wrap_ref(c), _1), token); }

	private:
		size_t DoPush(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			const size_t size = consumer.Process(data, token);
			_processFunc(ConstByteData(data, 0, size), token);
			return size;
		}

		void EndOfData(IDataConsumer& consumer, const ICancellationToken& token)
		{
			consumer.EndOfData(token);
			_eodFunc(token);
		}
	};
	STINGRAYKIT_DECLARE_PTR(InterceptingDataSource);


	class ReactiveDataSource final : public virtual IDataSource
	{
		class Consumer final : public virtual IDataConsumer
		{
		private:
			IDataConsumer&				_consumer;
			atomic<bool>				_endOfData;

		public:
			explicit Consumer(IDataConsumer& consumer)
				: _consumer(consumer), _endOfData(false)
			{ }

			bool IsEndOfData() const { return _endOfData; }

			size_t Process(ConstByteData data, const ICancellationToken& token) override { return _consumer.Process(data, token); }

			void EndOfData(const ICancellationToken& token) override
			{
				_endOfData = true;
				_consumer.EndOfData(token);
			}
		};

	private:
		IDataSourcePtr					_source;

	public:
		explicit ReactiveDataSource(const IDataSourcePtr& source)
			: _source(STINGRAYKIT_REQUIRE_NOT_NULL(source))
		{ }

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{
			Consumer reactiveConsumer(consumer);
			while (token && !reactiveConsumer.IsEndOfData())
				_source->Read(reactiveConsumer, token);
		}
	};
	STINGRAYKIT_DECLARE_PTR(ReactiveDataSource);


	class EmptyDataSource final : public virtual IDataSource
	{
	public:
		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{ consumer.EndOfData(token); }
	};
	STINGRAYKIT_DECLARE_PTR(EmptyDataSource);

}

#endif
