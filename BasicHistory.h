#ifndef STINGRAY_TOOLKIT_BASICHISTORY_H
#define STINGRAY_TOOLKIT_BASICHISTORY_H


#include <algorithm>
#include <deque>
#include <functional>


namespace stingray
{

	template < typename T >
	class BasicHistory
	{
		typedef std::deque<T>		HistoryDeque;
		typedef std::deque<int>		LastHistoryIndices;

		HistoryDeque				_history;
		LastHistoryIndices			_lastIndices;
		size_t						_maxHistorySize;
		size_t						_lastHistoryIndicesCount;

	public:
		BasicHistory(size_t maxHistorySize, size_t lastHistoryIndicesCount = 2)
			: _maxHistorySize(maxHistorySize), _lastHistoryIndicesCount(lastHistoryIndicesCount)
		{ }

		void Add(const T& val)
		{
			if (!_lastIndices.empty())
			{
				int li = *_lastIndices.rbegin();
				if (li >= 0 && (int)_history.size() > li && _history[li] == val)
					return;
			}

			_history.push_back(val);
			int index_delta = 0;
			for (;_history.size() > _maxHistorySize; ++index_delta)
				_history.pop_front();

			for (LastHistoryIndices::iterator it = _lastIndices.begin(); it != _lastIndices.end(); ++it)
				*it -= index_delta;
			_lastIndices.erase(std::remove_if(_lastIndices.begin(), _lastIndices.end(), bind(std::less<size_t>(), _1, 0)), _lastIndices.end());

			AddLastIndex(_history.size() - 1);
		}

		bool ToggleLast(T& outVal) // TODO: replace with more generic accessors
		{
			if (_lastIndices.size() < 2)
				return false;

			int prev = _lastIndices[_lastIndices.size() - 2];
			if (prev < 0 || (int)_history.size() <= prev)
				return false;

			AddLastIndex(prev);
			outVal = _history[prev];
			return true;
		}

	private:
		void AddLastIndex(size_t index)
		{
			_lastIndices.push_back(index);
			while (_lastIndices.size() > _lastHistoryIndicesCount)
				_lastIndices.pop_front();
		}
	};
	
}


#endif
