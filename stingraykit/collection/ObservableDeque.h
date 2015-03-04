#ifndef STINGRAYKIT_COLLECTION_OBSERVABLEDEQUE_H
#define STINGRAYKIT_COLLECTION_OBSERVABLEDEQUE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <deque>

#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template<typename T>
	class ObservableDeque;

	template <typename T, typename Container>
	class ObservableIterator : public iterator_base<ObservableIterator<T, Container>, const T, std::random_access_iterator_tag>
	{
		typedef iterator_base<ObservableIterator<T, Container>, const T, std::random_access_iterator_tag> base;
	public:
		typedef typename base::iterator_category	iterator_category;
		typedef typename base::value_type			value_type;
		typedef typename base::difference_type		difference_type;
		typedef typename base::pointer				pointer;
		typedef	typename base::reference			reference;
	private:
		typedef typename Container::iterator RawIterator;
		RawIterator _iter;
		explicit ObservableIterator(const RawIterator &iter)
			: _iter(iter)
		{}
		friend class ObservableDeque<T>;
	public:
		ObservableIterator() {}
		ObservableIterator(const ObservableIterator &other)
			: _iter(other._iter)
		{}
		reference dereference() const										{ return *_iter; }
		bool equal(const ObservableIterator &other) const					{ return _iter == other._iter; }
		void increment()													{ ++_iter; }
		void decrement()													{ --_iter; }
		void advance(difference_type n)										{ _iter += n; }
		difference_type distance_to(const ObservableIterator &other) const	{ return other._iter - _iter; }
	};

	template<typename T>
	class ObservableDeque
	{
		STINGRAYKIT_NONCOPYABLE(ObservableDeque);

	private:
		typedef std::deque<T> ContainerType;
		ContainerType _container;

	public:
		typedef typename ContainerType::value_type		value_type;
		typedef ObservableIterator<T, ContainerType>	iterator;
		typedef ObservableIterator<T, ContainerType>	const_iterator;

		ObservableDeque(): CollectionChanged(bind(&ObservableDeque::send_state, this, _1)) {}
		~ObservableDeque()	{ signal_locker l(CollectionChanged); erase(begin(), end()); }

		iterator begin() const				{ return iterator(const_cast<ContainerType&>(_container).begin()); }
		iterator end() const				{ return iterator(const_cast<ContainerType&>(_container).end()); }

		size_t size() const					{ return _container.size(); }
		bool empty() const					{ return _container.empty(); }
		const T& operator[](size_t n) const	{ return _container[n]; }
		void replace(size_t idx, const T &t)
		{
			STINGRAYKIT_CHECK(idx < size(), "replace: invalid index");
			_container[idx] = t;
			CollectionChanged(CollectionOp::Updated, idx, t);
		}

		const T& front() const		{ STINGRAYKIT_CHECK(!empty(), std::runtime_error("ObservableCollection is empty!")); return _container.front(); }
		const T& back() const		{ STINGRAYKIT_CHECK(!empty(), std::runtime_error("ObservableCollection is empty!")); return _container.back(); }

		void push_back(const T &t)	{ _container.push_back(t);	CollectionChanged(CollectionOp::Added, _container.size() - 1, t); }
		void push_front(const T &t)	{ _container.push_front(t);	CollectionChanged(CollectionOp::Added, 0, t); }

		void pop_back()
		{
			STINGRAYKIT_CHECK(!empty(), std::runtime_error("ObservableCollection is empty!"));
			T item(back());
			_container.pop_back();
			CollectionChanged(CollectionOp::Removed, _container.size(), item);
		}

		void pop_front()
		{
			STINGRAYKIT_CHECK(!empty(), std::runtime_error("ObservableCollection is empty!"));
			T item(front());
			_container.pop_front();
			CollectionChanged(CollectionOp::Removed, 0, item);
		}

		iterator insert(iterator position, const T &t)
		{
			size_t index = begin().distance_to(position);
			iterator result(_container.insert(position._iter, t));
			CollectionChanged(CollectionOp::Added, index, t);
			return result;
		}

		template<typename InputIterator>
		void insert(iterator position, InputIterator first, InputIterator last)
		{
			size_t index = begin().distance_to(position);
			_container.insert(position._iter, first, last);
			for (; first != last; ++first, ++index)
				CollectionChanged(CollectionOp::Added, index, *first);
		}

		iterator erase(iterator position)
		{
			T item(*position);
			size_t index = begin().distance_to(position);
			iterator result(_container.erase(position._iter));
			CollectionChanged(CollectionOp::Removed, index, item);
			return result;
		}

		iterator erase(iterator first, iterator last)
		{
			size_t index = begin().distance_to(first);
			std::vector<T> local_copy(first, last);
			iterator result(_container.erase(first._iter, last._iter));
			for (size_t n = 0; n < local_copy.size(); ++n, ++index)
				CollectionChanged(CollectionOp::Removed, index, local_copy[n]);
			return result;
		}

		void clear() { erase(begin(), end()); }

		signal<void(CollectionOp, size_t, const T&)>	CollectionChanged;

	private:
		void send_state(const function<void (CollectionOp, size_t, const T&)> &slot)
		{
			for(size_t i = 0; i < size(); ++i)
				slot(CollectionOp::Added, i, _container[i]);
		}


	};

	/** @} */

}
#endif
