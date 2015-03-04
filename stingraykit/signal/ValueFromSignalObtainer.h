#ifndef STINGRAYKIT_SIGNAL_VALUEFROMSIGNALOBTAINER_H
#define STINGRAYKIT_SIGNAL_VALUEFROMSIGNALOBTAINER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <vector>

#include <stingraykit/collection/CollectionBuilder.h>
#include <stingraykit/function/function_info.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/unique_ptr.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

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
		void operator() (CollectionOp op, const ValueType& val) const { STINGRAYKIT_CHECK(op == CollectionOp::Added, "Invalid CollectionOp!"); (*_val) % val; }

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
			STINGRAYKIT_CHECK(!*_val, "Value already set!");
			_val->reset(new T(val));
		}

		const T* operator -> () const	{ return *GetValue(); }

		const T& GetValue() const
		{
			STINGRAYKIT_CHECK(*_val, "Value is not set!");
			return **_val;
		}

		bool HasValue() const { return *_val; }
	};

	/** @} */

}


#endif
