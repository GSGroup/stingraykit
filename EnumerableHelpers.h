#ifndef STINGRAY_TOOLKIT_ENUMERABLEHELPERS_H
#define STINGRAY_TOOLKIT_ENUMERABLEHELPERS_H


#include <stingray/toolkit/IEnumerable.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct EmptyEnumerator : public virtual IEnumerator<T>
	{
		virtual bool Valid() const	{ return false; }
		virtual T Get() const		{ TOOLKIT_THROW(NotSupportedException()); }
		virtual void Next()			{ TOOLKIT_THROW(NotSupportedException()); }
	};


	class EmptyEnumeratorProxy
	{
	public:
		template< typename U >
		operator shared_ptr<IEnumerator<U> >() const
		{ return make_shared<EmptyEnumerator<U> >(); }
	};


	inline EmptyEnumeratorProxy MakeEmptyEnumerator()
	{ return EmptyEnumeratorProxy(); }


	template < typename T >
	struct OneItemEnumerator : public virtual IEnumerator<T>
	{
	private:
		bool	_valid;
		T		_value;

	public:
		OneItemEnumerator(typename GetConstReferenceType<T>::ValueT value)
			: _valid(true), _value(value)
		{ }

		virtual bool Valid() const			{ return _valid; }
		virtual void Next()					{ _valid = false; }

		virtual T Get() const
		{
			if (!_valid)
				TOOLKIT_THROW(std::runtime_error("Invalid enumerator!"));
			return _value;
		}
	};


	template< typename T >
	class OneItemEnumeratorProxy
	{
	private:
		T	_item;

	public:
		explicit OneItemEnumeratorProxy(const T& item)
			: _item(item)
		{ }

		template< typename U >
		operator shared_ptr<IEnumerator<U> >() const
		{ return make_shared<OneItemEnumerator<U> >(_item); }
	};


	template< typename T >
	OneItemEnumeratorProxy<T> MakeOneItemEnumerator(const T& item)
	{ return OneItemEnumeratorProxy<T>(item); }


	namespace Detail
	{
		template<typename EnumeratedT>
		class JoiningEnumerator : public virtual IEnumerator<EnumeratedT>
		{
			typedef shared_ptr<IEnumerator<EnumeratedT> > TargetEnumeratorPtr;

			TargetEnumeratorPtr _first, _second;

		public:
			JoiningEnumerator(const TargetEnumeratorPtr& first, const TargetEnumeratorPtr& second) :
				_first(TOOLKIT_REQUIRE_NOT_NULL(first)),
				_second(TOOLKIT_REQUIRE_NOT_NULL(second))
			{}

			virtual bool Valid() const
			{ return _first->Valid() || _second->Valid(); }

			virtual EnumeratedT Get() const
			{ return _first->Valid() ? _first->Get() : _second->Get(); }

			virtual void Next()
			{
				if (_first->Valid())
					_first->Next();
				else
					_second->Next();
			}
		};
	}

	template<typename EnumeratedT>
	shared_ptr<IEnumerator<EnumeratedT> > JoinEnumerators(const shared_ptr<IEnumerator<EnumeratedT> >& first, const shared_ptr<IEnumerator<EnumeratedT> >& second)
	{ return make_shared<Detail::JoiningEnumerator<EnumeratedT> >(first, second); }

	template<typename EnumeratedT>
	class EnumeratorJoiner
	{
		typedef shared_ptr<IEnumerator<EnumeratedT> >	EnumeratorPtr;

	private:
		EnumeratorPtr	_result;

	public:
		operator EnumeratorPtr () const { return _result; }
		EnumeratorJoiner& operator % (const EnumeratorPtr& e)
		{
			if (_result)
				_result = JoinEnumerators(_result, e);
			else
				_result = e;
			return *this;
		}
	};


	template < typename T >
	struct EmptyEnumerable : public virtual IEnumerable<T>
	{
		virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
		{ return make_shared<EmptyEnumerator<T> >(); }
	};


	class EmptyEnumerableProxy
	{
	public:
		template< typename U >
		operator shared_ptr<IEnumerable<U> >() const
		{ return make_shared<EmptyEnumerable<U> >(); }
	};


	inline EmptyEnumerableProxy MakeEmptyEnumerable()
	{ return EmptyEnumerableProxy(); }


	template < typename T >
	struct OneItemEnumerable : public virtual IEnumerable<T>
	{
	private:
		T		_value;

	public:
		OneItemEnumerable(typename GetConstReferenceType<T>::ValueT value)
			: _value(value)
		{ }

		virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
		{ return make_shared<OneItemEnumerator<T> >(_value); }
	};


	template< typename T >
	class OneItemEnumerableProxy
	{
	private:
		T	_item;

	public:
		explicit OneItemEnumerableProxy(const T& item)
			: _item(item)
		{ }

		template< typename U >
		operator shared_ptr<IEnumerable<U> >() const
		{ return make_shared<OneItemEnumerable<U> >(_item); }
	};


	template< typename T >
	OneItemEnumerableProxy<T> MakeOneItemEnumerable(const T& item)
	{ return OneItemEnumerableProxy<T>(item); }


	namespace Detail
	{
		template<typename EnumeratedT>
		class JoiningEnumerable : public virtual IEnumerable<EnumeratedT>
		{
			typedef shared_ptr<IEnumerable<EnumeratedT> > TargetEnumerablePtr;

			TargetEnumerablePtr _first, _second;

		public:
			JoiningEnumerable(const TargetEnumerablePtr& first, const TargetEnumerablePtr& second) :
				_first(TOOLKIT_REQUIRE_NOT_NULL(first)),
				_second(TOOLKIT_REQUIRE_NOT_NULL(second))
			{}

			virtual shared_ptr<IEnumerator<EnumeratedT> > GetEnumerator() const
			{ return JoinEnumerators(_first->GetEnumerator(), _second->GetEnumerator()); }
		};
	}


	template<typename EnumeratedT>
	shared_ptr<IEnumerable<EnumeratedT> > JoinEnumerables(const shared_ptr<IEnumerable<EnumeratedT> >& first, const shared_ptr<IEnumerable<EnumeratedT> >& second)
	{ return make_shared<Detail::JoiningEnumerable<EnumeratedT> >(first, second); }


	template<typename EnumeratedT>
	class EnumerableJoiner
	{
		typedef shared_ptr<IEnumerable<EnumeratedT> >	EnumerablePtr;

	private:
		EnumerablePtr	_result;

	public:
		operator EnumerablePtr () const { return _result; }
		EnumerableJoiner& operator % (const EnumerablePtr& e)
		{
			if (_result)
				_result = JoinEnumerables(_result, e);
			else
				_result = e;
			return *this;
		}
	};


	template< typename EnumeratorType_, typename ParamsTuple_ >
	struct SimpleEnumerable : public IEnumerable<typename EnumeratorType_::ItemType>
	{
	private:
		ParamsTuple_ _tuple;

	public:
		SimpleEnumerable(const ParamsTuple_& tuple) : _tuple(tuple)
		{}

		virtual shared_ptr<IEnumerator<typename EnumeratorType_::ItemType> > GetEnumerator() const
		{ return FunctorInvoker::Invoke(MakeShared<EnumeratorType_, ParamsTuple_::Size>(), _tuple); }
	};


	template< typename EnumeratorType_, typename ParamsTuple_ >
	shared_ptr<SimpleEnumerable<EnumeratorType_, ParamsTuple_> > MakeSimpleEnumerable(const ParamsTuple_& tuple)
	{ return make_shared<SimpleEnumerable<EnumeratorType_, ParamsTuple_> >(tuple); }

	/** @} */

}

#endif
