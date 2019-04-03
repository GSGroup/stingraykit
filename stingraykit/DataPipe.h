#ifndef STINGRAYKIT_DATAPIPE_H
#define STINGRAYKIT_DATAPIPE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>

namespace stingray {

	/**
	 * @addtogroup core_parsers
	 * @{
	 */

	template<typename T>
	struct DataPipeStageTraits
	{
		typedef typename T::InputType	InputType;
		typedef typename T::OutputType	OutputType;
		typedef typename T::RetType		RetType;
	};

	template<typename InputType_, typename OutputType_, typename RetType_ = void>
	struct DataPipeStageBase
	{
		typedef InputType_	InputType;
		typedef OutputType_	OutputType;
		typedef RetType_	RetType;

		void Reset() { }
	};

	namespace Detail
	{
		template < typename Stages >
		struct DataPipeStage
		{
			typedef Tuple<Stages>										StagesTuple;
			typedef typename Stages::ValueT								Stage;
			typedef typename DataPipeStageTraits<Stage>::OutputType		OutputType;
			typedef typename DataPipeStageTraits<Stage>::RetType		RetType;
			typedef DataPipeStage<typename Stages::Next>				NextStage;

			template <typename InputType, typename Consumer>
			static RetType Process(StagesTuple& stages, const InputType& input, const Consumer& consumer)
			{ return stages.GetHead().Process(input, bind(&NextStage::template Process<OutputType, Consumer>, wrap_ref(stages.GetTail()), _1, wrap_ref(consumer))); }

			template <typename Consumer, typename EodFunc>
			static RetType EndOfData(StagesTuple& stages, const Consumer& consumer, const EodFunc& eod)
			{
				return stages.GetHead().EndOfData(
					bind(&NextStage::template Process<OutputType, Consumer>, wrap_ref(stages.GetTail()), wrap_ref(consumer), _1),
					bind(&NextStage::template EndOfData<Consumer, EodFunc>, wrap_ref(stages.GetTail()), wrap_ref(consumer), wrap_ref(eod)));
			}

			static void Reset(StagesTuple& stages)
			{ stages.GetHead().Reset(); NextStage::Reset(stages.GetTail()); }
		};

		template < >
		struct DataPipeStage<TypeListEndNode>
		{
			typedef Tuple<TypeListEndNode>		StagesTuple;

			template <typename InputType, typename Consumer>
			static typename Consumer::RetType Process(StagesTuple& stages, const InputType& input, const Consumer& consumer)
			{ return consumer(input); }

			template <typename Consumer, typename EodFunc>
			static typename EodFunc::RetType EndOfData(StagesTuple& stages, const Consumer& consumer, const EodFunc& eod)
			{ return eod(); }

			static void Reset(StagesTuple& stages)
			{ }
		};

		template < typename TypeList >
		struct GetTypeListLastItem
		{ typedef typename GetTypeListItem<TypeList, GetTypeListLength<TypeList>::Value - 1>::ValueT ValueT; };
	}

	template<typename Stages>
	struct DataPipe
	{
		typedef typename DataPipeStageTraits<typename Stages::ValueT>::InputType								InputType;
		typedef typename DataPipeStageTraits<typename Stages::ValueT>::RetType									RetType;
		typedef typename DataPipeStageTraits<typename Detail::GetTypeListLastItem<Stages>::ValueT>::OutputType	OutputType;

		typedef Tuple<Stages> StagesTuple;

		StagesTuple _stages;

	public:
		RetType Process(const InputType& data)
		{ return Process(data, EmptyConsumer()); }

		template <typename Consumer>
		RetType Process(const InputType& data, const Consumer& consumer)
		{ return Detail::DataPipeStage<Stages>::Process(_stages, data, consumer); }

		RetType EndOfData()
		{ return EndOfData(EmptyConsumer(), EmptyEod()); }

		template <typename Consumer, typename EodFunc>
		RetType EndOfData(const Consumer& consumer, const EodFunc& eod)
		{ return Detail::DataPipeStage<Stages>::EndOfData(_stages, consumer, eod); }

		void Reset()
		{ Detail::DataPipeStage<Stages>::Reset(_stages); }

		template < size_t Index_ >
		typename GetTypeListItem<Stages, Index_>::ValueT& GetStage()
		{ return _stages.template Get<Index_>(); }

		template < typename Type_ >
		Type_& GetStage(Dummy dummy = Dummy())
		{ return _stages.template Get<Type_>(); }

		template < typename Type_ >
		const Type_& GetStage(Dummy dummy = Dummy()) const
		{ return _stages.template Get<Type_>(); }

		template < typename Type_, size_t Index_ >
		Type_& GetStage(Dummy dummy = Dummy(), Dummy dummy2 = Dummy())
		{ return _stages.template Get<Type_, Index_>(); }

	private:
		struct EmptyConsumer
		{
			typedef void RetType;

			template <typename InputType>
			void operator () (const InputType&) const
			{ }
		};

		struct EmptyEod
		{
			typedef void RetType;

			void operator () () const
			{ }
		};
	};

	/** @} */

}


#endif

