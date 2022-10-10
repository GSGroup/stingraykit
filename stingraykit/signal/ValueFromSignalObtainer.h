#ifndef STINGRAYKIT_SIGNAL_VALUEFROMSIGNALOBTAINER_H
#define STINGRAYKIT_SIGNAL_VALUEFROMSIGNALOBTAINER_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/CollectionBuilder.h>
#include <stingraykit/signal/signal_connector.h>
#include <stingraykit/Tuple.h>
#include <stingraykit/unique_ptr.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	template < typename CollectionType >
	class ValuesFromSignalCollector
	{
		STINGRAYKIT_NONCOPYABLE(ValuesFromSignalCollector);

	private:
		using Builder = CollectionBuilder<CollectionType>;

		class Invoker : public function_info<void, UnspecifiedParamTypes>
		{
		private:
			ValuesFromSignalCollector&	_inst;

		public:
			explicit Invoker(ValuesFromSignalCollector& inst) : _inst(inst) { }

			template < typename U >
			void operator () (U&& val) const { _inst._val % std::forward<U>(val); }

			template < typename U >
			void operator () (CollectionOp op, U&& val) const
			{
				STINGRAYKIT_CHECK(op == CollectionOp::Added, ArgumentException("op", op));
				_inst._val % std::forward<U>(val);
			}
		};

	private:
		Builder				_val;

	public:
		ValuesFromSignalCollector() { }

		auto GetInvoker() { return Invoker(*this); }

		template < typename U >
		operator function<void (U)> () { return GetInvoker(); }

		template < typename U >
		operator function<void (CollectionOp op, U)> () { return GetInvoker(); }

		const CollectionType* operator -> () const	{ return &GetValues(); }
		const CollectionType& GetValues() const { return _val; }
	};


	template < typename T >
	class VectorFromSignalCollector : public ValuesFromSignalCollector< std::vector<T> >
	{ };


	template < typename CollectionType, typename T >
	CollectionType GetValuesFromSignal(const signal_connector<void (T)>& connector)
	{
		ValuesFromSignalCollector<CollectionType> collector;
		connector.SendCurrentState(collector);
		return collector.GetValues();
	}


	template < typename CollectionType, typename T >
	CollectionType GetValuesFromSignal(const signal_connector<void (CollectionOp, T)>& connector)
	{
		ValuesFromSignalCollector<CollectionType> collector;
		connector.SendCurrentState(collector);
		return collector.GetValues();
	}


	template < typename T >
	std::vector<typename Decay<T>::ValueT> GetVectorFromSignal(const signal_connector<void (T)>& connector)
	{ return GetValuesFromSignal<std::vector<typename Decay<T>::ValueT> >(connector); }


	template < typename T >
	std::vector<typename Decay<T>::ValueT> GetVectorFromSignal(const signal_connector<void (CollectionOp, T)>& connector)
	{ return GetValuesFromSignal<std::vector<typename Decay<T>::ValueT> >(connector); }


	template < typename... Ts >
	class TupleFromSignalObtainer
	{
		STINGRAYKIT_NONCOPYABLE(TupleFromSignalObtainer);

	private:
		using Types = TypeList<Ts...>;
		using TupleType = Tuple<Types>;

		class Invoker : public function_info<void, UnspecifiedParamTypes>
		{
		private:
			TupleFromSignalObtainer&	_inst;

		public:
			explicit Invoker(TupleFromSignalObtainer& inst) : _inst(inst) { }

			template < typename... Us >
			void operator () (Us&&... vals) const
			{
				STINGRAYKIT_CHECK(!_inst._vals, "Value already set!");
				_inst._vals.emplace(std::forward<Us>(vals)...);
			}
		};

	private:
		optional<TupleType>		_vals;

	public:
		TupleFromSignalObtainer() { }

		auto GetInvoker() { return Invoker(*this); }

		template < typename... Us >
		operator function<void (Us...)> () { return GetInvoker(); }

		template < size_t Index >
		const typename GetTypeListItem<Types, Index>::ValueT& GetValue() const
		{
			STINGRAYKIT_CHECK(_vals, "Value is not set!");
			return _vals->template Get<Index>();
		}

		const TupleType& GetValues() const
		{
			STINGRAYKIT_CHECK(_vals, "Value is not set!");
			return *_vals;
		}

		bool HasValues() const { return _vals.is_initialized(); }
	};


	template < typename... Ts >
	optional<Tuple<TypeList<typename Decay<Ts>::ValueT...>>> GetTupleFromSignal(const signal_connector<void (Ts...)>& connector)
	{
		TupleFromSignalObtainer<typename Decay<Ts>::ValueT...> obtainer;
		connector.SendCurrentState(obtainer);
		return obtainer.HasValues() ? make_optional_value(obtainer.GetValues()) : null;
	}


	template < typename... Ts >
	bool HasTupleInSignal(const signal_connector<void (Ts...)>& connector)
	{
		TupleFromSignalObtainer<typename Decay<Ts>::ValueT...> obtainer;
		connector.SendCurrentState(obtainer);
		return obtainer.HasValues();
	}


	template < typename T >
	class ValueFromSignalObtainer
	{
		STINGRAYKIT_NONCOPYABLE(ValueFromSignalObtainer);

	private:
		class Invoker : public function_info<void, UnspecifiedParamTypes>
		{
		private:
			ValueFromSignalObtainer&	_inst;

		public:
			explicit Invoker(ValueFromSignalObtainer& inst) : _inst(inst) { }

			template < typename U >
			void operator () (U&& val) const
			{
				STINGRAYKIT_CHECK(!_inst._val, "Value already set!");
				_inst._val.emplace(std::forward<U>(val));
			}
		};

	private:
		optional<T>			_val;

	public:
		ValueFromSignalObtainer() { }

		auto GetInvoker() { return Invoker(*this); }

		template < typename U >
		operator function<void (U)> () { return GetInvoker(); }

		const T* operator -> () const { return &GetValue(); }

		const T& GetValue() const
		{
			STINGRAYKIT_CHECK(_val, "Value is not set!");
			return *_val;
		}

		bool HasValue() const { return _val.is_initialized(); }
	};


	template < >
	class ValueFromSignalObtainer<void>
	{
		STINGRAYKIT_NONCOPYABLE(ValueFromSignalObtainer);

	private:
		class Invoker : public function_info<void, UnspecifiedParamTypes>
		{
		private:
			ValueFromSignalObtainer&	_inst;

		public:
			explicit Invoker(ValueFromSignalObtainer& inst) : _inst(inst) { }

			void operator () () const
			{
				STINGRAYKIT_CHECK(!_inst._val, "Value already set!");
				_inst._val = true;
			}
		};

	private:
		bool				_val;

	public:
		ValueFromSignalObtainer() : _val(false) { }

		auto GetInvoker() { return Invoker(*this); }

		operator function<void ()> () { return GetInvoker(); }

		bool HasValue() const { return _val; }
	};


	template < typename T >
	typename If<IsNullable<typename Decay<T>::ValueT>::Value, typename Decay<T>::ValueT, optional<typename Decay<T>::ValueT> >::ValueT GetValueFromSignal(const signal_connector<void (T)>& connector)
	{
		typedef typename If<IsNullable<typename Decay<T>::ValueT>::Value, typename Decay<T>::ValueT, optional<typename Decay<T>::ValueT> >::ValueT ResultType;

		ValueFromSignalObtainer<typename Decay<T>::ValueT> obtainer;
		connector.SendCurrentState(obtainer);
		return obtainer.HasValue() ? ResultType(obtainer.GetValue()) : null;
	}


	template < typename T >
	bool HasValueInSignal(const signal_connector<void (T)>& connector)
	{
		ValueFromSignalObtainer<typename Decay<T>::ValueT> obtainer;
		connector.SendCurrentState(obtainer);
		return obtainer.HasValue();
	}


	inline bool HasValueInSignal(const signal_connector<void ()>& connector)
	{
		ValueFromSignalObtainer<void> obtainer;
		connector.SendCurrentState(obtainer);
		return obtainer.HasValue();
	}

	/** @} */

}

#endif
