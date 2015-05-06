#ifndef STINGRAYKIT_COLLECTION_REFCOUNTINGMAP_H
#define STINGRAYKIT_COLLECTION_REFCOUNTINGMAP_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/optional.h>
#include <stingraykit/thread/Thread.h>

namespace stingray
{

	template<typename Key_, typename Value_>
	class RefCountingMap
	{
		struct ValueHolder
		{
			size_t	References;
			Value_	Value;

			ValueHolder(size_t references, const Value_& value) :
				References(references), Value(value)
			{ }
		};

		typedef std::map<Key_, ValueHolder>					Impl;
		typedef function<Value_(const Key_&)>				AddFunc;
		typedef function<void(const Key_&, const Value_&)>	RemoveFunc;

		struct Iterator : public iterator_base<Iterator, std::pair<const Key_&, Value_&>, std::bidirectional_iterator_tag>
		{
			typedef iterator_base<Iterator, std::pair<const Key_&, Value_&>, std::bidirectional_iterator_tag>	base;
			typedef std::pair<const Key_&, Value_&>																PairType;

		private:
			typename Impl::iterator		_implIt;
			mutable optional<PairType>	_pair;

		public:
			Iterator()
			{ }

			Iterator(typename Impl::iterator implIt) : _implIt(implIt)
			{ }

			typename base::reference dereference() const
			{
				if (!_pair)
					_pair.assign(PairType(_implIt->first, _implIt->second.Value));
				return *_pair;
			}

			bool equal(const Iterator &other) const
			{ return _implIt == other._implIt; }

			void increment()
			{
				++_implIt;
				_pair.reset();
			}

			typename Impl::iterator GetImpl() const
			{ return _implIt; }
		};

	public:
		typedef Iterator iterator;
		typedef Iterator const_iterator;

	private:
		AddFunc		_addFunc;
		RemoveFunc	_removeFunc;
		Mutex		_mutex;
		Impl		_impl;

	public:
		RefCountingMap(const AddFunc& addFunc, const RemoveFunc& removeFunc) :
			_addFunc(addFunc), _removeFunc(removeFunc)
		{ }

		bool empty() const				{ return _impl.empty(); }
		size_t size() const				{ return _impl.size(); }

		iterator begin()				{ return _impl.begin(); }
		iterator end()					{ return _impl.end(); }

		iterator find(const Key_& key)	{ return _impl.find(key); }

		iterator add(const Key_& key)
		{
			typename Impl::iterator it = _impl.find(key);
			if (it != _impl.end())
			{
				++(it->second.References);
				return it;
			}

			return _impl.insert(std::make_pair(key, ValueHolder(1, _addFunc(key)))).first;
		}

		size_t erase(const Key_& key)
		{
			typename Impl::iterator implIt = _impl.find(key);
			if (implIt == _impl.end())
				return 0;

			if (--(implIt->second.References) > 0)
				return 1;

			_removeFunc(implIt->first, implIt->second.Value);
			_impl.erase(implIt);
			return 1;
		}

		void erase(iterator it)
		{
			typename Impl::iterator implIt = it.GetImpl();
			if (--(implIt->second.References) > 0)
				return;

			_removeFunc(implIt->first, implIt->second.Value);
			_impl.erase(implIt);
		}

		void clear()
		{
			for (typename Impl::iterator it = _impl.begin(); it != _impl.end(); ++it)
				_removeFunc(it->first, it->second.Value);
			_impl.clear();
		}
	};

}

#endif
