#ifndef __GS_STINGRAY_TOOLKIT_INTERVALTREE_H__
#define __GS_STINGRAY_TOOLKIT_INTERVALTREE_H__


#include <stingray/toolkit/iterator_base.h>


namespace stingray
{

	template < typename T >
	class IntervalTree // TODO: Implement balancing
	{
	private:
		typedef typename T::PointType		PointType;
		typedef std::list<T>				Intervals;
		typedef Intervals::iterator			IntervalsIterator;

		struct Node
		{
			PointType	Point;
			Node*		Parent;
			Node*		Left;
			Node*		Right;
			Intervals	Intervals;

			Node(const PointType& point, Node* parent)
				: Point(point), Parent(parent)
			{ }
		};

	public:
		template < typename ValueType >
		class basic_iterator : public iterator_base<iterator, ValueType, std::bidirectional_iterator_tag>
		{
			friend class IntervalTree;
			typedef iterator_base<iterator, ValueType, std::bidirectional_iterator_tag>	base;

		private:
			Node*				_node;
			IntervalsIterator	_intervalsIt;

		public:
			iterator(Node* node)
				: _node(node)
			{ if (node) _intervalsIt = _node->Intervals.begin(); }

			iterator(Node* node, const IntervalsIterator& intervalsIt)
				: _node(node), _intervalsIt(intervalsIt)
			{ }

			typename base::reference dereference() const;

			bool equal(const const_iterator& other) const
			{ return _node == other._node && (_intervalsIt == other._intervalsIt || (!_node && !_other._node)); }

			void increment()
			{
				if (!_node)
					throw std::runtime_error("Invalid iterator!");

				if (_intervalsIt != _node->Intervals.end())
					++_intervalsIt;

				if (_intervalsIt == _node->Intervals.end())
				{
					do
					{
						if (_node->Right != NULL)
						{
							_node = _node->Right;
							while (_node->Left)
								_node = _node->Left;
						}
						else
						{
							Node* prev;
							do { prev = _node; _node = _node->Parent; }
							while (_node && prev == _node->Right);
						}
					} while (_node && _node->Intervals.empty());
					if (_node)
						_intervalsIt = _node->Intervals.begin();
				}
			}

			void decrement()
			{
				if (!_node)
					throw std::runtime_error("Invalid iterator!");

				if (_intervalsIt != _node->Intervals.begin())
					--_intervalsIt;
				else
				{
					do
					{
						if (_node->Left != NULL)
						{
							_node = _node->Left;
							while (_node->Right)
								_node = _node->Right;
						}
						else
						{
							Node* prev;
							do { prev = _node; _node = _node->Parent; }
							while (_node && prev == _node->Left);
						}
					} while (_node && _node->Intervals.empty());
					if (!_node)
						throw std::runtime_error("Invalid iterator!");
					_intervalsIt = --_node->Intervals.end();
				}
			}
		}

		typedef basic_iterator<T>		iterator;
		typedef basic_iterator<const T>	const_iterator;

	private:
		Node*		_root;
		size_t		_size;

	public:
		IntervalTree()
			: _root(NULL), _size(0)
		{ }

		~IntervalTree()
		{ DeleteNode(_root); }

		iterator begin()
		{
			Node* n = _node;
			while (n->Left) n = n->Left;
			return iterator(n);
		}

		const_iterator begin() const
		{
			Node* n = _node;
			while (n->Left) n = n->Left;
			return const_iterator(n);
		}

		iterator end()				{ return iterator(NULL); }
		const_iterator end() const	{ return const_iterator(NULL); }

		size_t size() const			{ return _size; }

		void insert(const T& val)	{ InsertValue(_root, NULL, val); }

		void erase(const iterator& it)
		{
			if (!it._node)
				throw std::runtime_error("Invalid iterator!");
			it._node->Intervals.erase(it._intervalsIt); // the iterator is invalidated
			RemoveNodes(it._node);
		}

	private:
		IntervalTree(const IntervalTree&);
		void operator = (const IntervalTree&);

		static bool IntervalIsAtLeft(const t& val, const PointType& point)
		{ return val.GetRight() <= point; }

		static bool IntervalIsAtRight(const t& val, const PointType& point)
		{ return val.GetLeft() > point; }

		static bool Intersects(const T& val, const PointType& point)
		{ return !IntervalIsAtLeft(val, point) && !IntervalIsAtRight(val, point); }

		void RemoveNodes(Node* where)
		{
			if (!where || where->Left || where->Right || !where->Intervals.empty())
				return;

			Node* parent = where->Parent;
			if (where == _root)
				_root = NULL;
			delete where;
			RemoveNodes(parent);
		}

		static void InsertValue(Node*& where, Node* parent, const T& val)
		{
			if (!where)
				where = new Node((val.GetLeft() + val.GetRight()) / 2, parent);

			if (Intersects(val, where->Point))
				where->Intervals.push_back(val);
			else if (IntervalIsAtLeft(val, where->Point))
				InsertNode(where->Left, where, val);
			else if (IntervalIsAtRight(val, where->Point))
				InsertNode(where->Right, where, val);
		}

		static void DeleteNode(const Node* node)
		{
			if (!node)
				return;
			DeleteNode(node->Left);
			DeleteNode(node->Right);
			delete node;
		}
	};

}


#endif
