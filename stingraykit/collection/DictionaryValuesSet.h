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
		typedef KeyType_											KeyType;
		typedef ValueType_											ValueType;
		typedef typename GetConstReferenceType<ValueType>::ValueT	ConstValueTypeRef;

		typedef IDictionary<KeyType_, ValueType_>					SrcDictionary;
		typedef typename SrcDictionary::PairType					PairType;
		typedef shared_ptr<SrcDictionary>							SrcDictionaryPtr;

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

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{ return ValuesEnumerator(_dict->GetEnumerator()); }

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{ return ValuesEnumerable(_dict->Reverse()); }

		virtual bool Contains(const ValueType& value) const
		{ return _dict->ContainsKey(_converter(value)); }

		virtual size_t GetCount() const
		{ return _dict->GetCount(); }

		virtual bool IsEmpty() const
		{ return _dict->IsEmpty(); }

		virtual void Add(const ValueType& value)
		{ _dict->Set(_converter(value), value); }

		virtual void Clear()
		{ _dict->Clear(); }

		virtual void Remove(const ValueType& value)
		{ TryRemove(value); }

		virtual bool TryRemove(const ValueType& value)
		{ return _dict->TryRemove(_converter(value)); }
	};


	template < typename DictionaryType, typename Converter >
	shared_ptr<DictionaryValuesSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> > GetDictionaryValuesSet(const shared_ptr<DictionaryType>& dict, const Converter& converter)
	{ return make_shared<DictionaryValuesSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> >(dict, converter); }

}

#endif
