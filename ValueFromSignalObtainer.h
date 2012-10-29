#ifndef STINGRAY_TOOLKIT_VALUEFROMSIGNALOBTAINER_H
#define STINGRAY_TOOLKIT_VALUEFROMSIGNALOBTAINER_H


#include <vector>

#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/unique_ptr.h>
#include <stingray/toolkit/CollectionBuilder.h>


namespace stingray
{


	template < typename CollectionType >
	class ValuesFromSignalCollector : public function_info<void(const typename CollectionType::value_type&)>
	{
		typedef CollectionBuilder<CollectionType>	Builder;
		typedef typename CollectionType::value_type	ValueType;
		typedef shared_ptr<Builder>					BuilderPtr;

	private:
		BuilderPtr	_val;

	public:
		ValuesFromSignalCollector() : _val(new Builder) { }
		
		void operator() (const ValueType& val) const { (*_val) % val; }
		void operator() (CollectionOp op, const ValueType& val) const { TOOLKIT_CHECK(op == CollectionOp::ItemAdded, "Invalid CollectionOp!"); (*_val) % val; }

		const CollectionType* operator -> () const	{ return &GetValues(); }
		const CollectionType& GetValues() const { return *_val; }
	};


	template < typename T >
	class VectorFromSignalCollector : public ValuesFromSignalCollector< std::vector<T> >
	{ };


	template < typename T >
	class ValueFromSignalObtainer : public function_info<void(const T&)>
	{
		typedef unique_ptr<T>		TPtr;
		typedef shared_ptr<TPtr>	TPtrPtr;

	private:
		TPtrPtr	_val;

	public:
		ValueFromSignalObtainer() : _val(new TPtr) { }
		
		void operator() (const T& val) const
		{
			TOOLKIT_CHECK(!*_val, "Value already set!");
			_val->reset(new T(val));
		}

		const T* operator -> () const	{ return *GetValue(); }

		const T& GetValue() const
		{
			TOOLKIT_CHECK(*_val, "Value is not set!");
			return **_val;
		}

		bool HasValue() const { return *_val; }
	};

}


#endif
