#ifndef STINGRAYKIT_COLLECTION_DICTIONARYVALUESSET_H
#define STINGRAYKIT_COLLECTION_DICTIONARYVALUESSET_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/ISet.h>

namespace stingray
{

	template < typename KeyType_, typename ValueType_ >
	class DictionaryValuesSet : public virtual ISet<ValueType_>
	{
		typedef shared_ptr<IDictionary<KeyType_, ValueType_> >		SrcDictionaryPtr;
		typedef typename GetConstReferenceType<ValueType_>::ValueT	ConstValueTypeRef;
		typedef function< KeyType_ (ConstValueTypeRef) >			Converter;

	private:
		SrcDictionaryPtr	_dict;
		Converter			_converter;

	public:
		DictionaryValuesSet(const SrcDictionaryPtr& dict, const Converter& converter)
			:	_dict(dict),
				_converter(converter)
		{ }

		virtual ~DictionaryValuesSet()
		{ }

		virtual shared_ptr<IEnumerator<ValueType_> > GetEnumerator() const
		{ return ValuesEnumerator(_dict->GetEnumerator()); }

		virtual shared_ptr<IEnumerable<ValueType_> > Reverse() const
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		virtual bool Contains(const ValueType_& value) const
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		virtual int GetCount() const
		{ return _dict->GetCount(); }

		virtual bool IsEmpty() const
		{ return _dict->IsEmpty(); }

		virtual void Add(const ValueType_& value)
		{ _dict->Set(_converter(value), value); }

		virtual void Clear()
		{ _dict->Clear(); }

		virtual void Remove(const ValueType_& value)
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		virtual bool TryRemove(const ValueType_& value)
		{ STINGRAYKIT_THROW(NotImplementedException()); }
	};


	template < typename DictionaryType, typename Converter >
	shared_ptr<DictionaryValuesSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> > GetDictionaryValuesSet(const shared_ptr<DictionaryType>& dict, const Converter& converter)
	{ return make_shared<DictionaryValuesSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> >(dict, converter); }

}

#endif
