#ifndef STINGRAYKIT_STRING_AHOCORASICK_H
#define STINGRAYKIT_STRING_AHOCORASICK_H

#include <stingraykit/collection/array.h>
#include <stingraykit/optional.h>

#include <algorithm>
#include <deque>

namespace stingray
{

	class AhoCorasick //array implementation
	{
		typedef	u8					CharType;

		static const size_t			None = ~(size_t)0u;
		static const size_t			Root = (size_t)0;

		struct TrieNode
		{
			array<size_t, 256>		Next;
			size_t					Fail;

			optional<size_t>		PatternIndex;

			TrieNode(): Fail(None)
			{ std::fill(Next.begin(), Next.end(), (size_t)None); }
		};
		std::deque<TrieNode>				_trie;
		std::deque<std::string>				_patterns;
		bool								_built;

	public:
		AhoCorasick() : _built(false)
		{ _trie.push_back(TrieNode()); }

		size_t Add(const std::string &pattern)
		{
			STINGRAYKIT_CHECK(!_built, "cannot add patterns after trie was built");
			size_t nodeIndex = Root;
			for(std::string::const_iterator it = pattern.begin(); it != pattern.end(); ++it)
			{
				CharType ch = static_cast<CharType>(*it);
				TrieNode &node = _trie[nodeIndex];
				size_t nextIndex = node.Next[ch];
				if (nextIndex == None)
				{
					node.Next[ch] = nextIndex = _trie.size();
					_trie.push_back(TrieNode());
				}
				nodeIndex = nextIndex;
			}

			size_t patternIndex = _patterns.size();
			_trie[nodeIndex].PatternIndex = patternIndex;
			_patterns.push_back(pattern);
			return patternIndex;
		}

		void Build()
		{
			STINGRAYKIT_CHECK(!_built, "cannot rebuild trie");
			_trie[Root].Fail = Root;
			std::deque<size_t> queue;
			{
				TrieNode & root = _trie[Root];
				for(size_t i = 0; i < root.Next.size(); ++i)
				{
					size_t nodeIndex = root.Next[i];
					if (nodeIndex != None && nodeIndex != Root)
					{
						_trie[nodeIndex].Fail = Root;
						queue.push_back(nodeIndex);
					}
					else
						root.Next[i] = Root;
				}
			}

			while(!queue.empty())
			{
				const size_t nodeIndex = queue.front();
				queue.pop_front();
				TrieNode & node = _trie[nodeIndex];
				for(size_t ch = 0; ch < node.Next.size(); ++ch)
				{
					size_t nextIndex = node.Next[ch];
					if (nextIndex != None)
					{
						queue.push_back(nextIndex);

						size_t failIndex = node.Fail;
						while(_trie[failIndex].Next[ch] == None)
							failIndex = _trie[failIndex].Fail;

						failIndex = _trie[failIndex].Next[ch];
						_trie[nextIndex].Fail = failIndex;
					}
				}
			}
			_built = true;
		}

		template<typename IteratorType, typename CallbackType>
		void Search(const IteratorType & begin, const IteratorType & end, const CallbackType & callback) const
		{
			STINGRAYKIT_CHECK(_built, "AhoCorasick requires Build() before searching");

			size_t nodeIndex = Root;
			for(IteratorType it = begin; it != end; ++it)
			{
				CharType ch = static_cast<CharType>(*it);

				while(_trie[nodeIndex].Next[ch] == None)
					nodeIndex = _trie[nodeIndex].Fail;

				nodeIndex = _trie[nodeIndex].Next[ch];
				if (nodeIndex == None)
					nodeIndex = Root;

				for (size_t failIndex = nodeIndex; failIndex != Root; failIndex = _trie[failIndex].Fail)
				{
					const TrieNode &node = _trie[failIndex];
					if (node.PatternIndex)
					{
						size_t patternIndex = *node.PatternIndex;
						const std::string &pattern = _patterns[patternIndex];
						size_t offset = std::distance(begin, it);
						callback(pattern, patternIndex, 1 + offset - pattern.size());
					}
				}
			}
		}

		template<typename StringType, typename CallbackType>
		void Search(const StringType & text, const CallbackType & callback) const
		{ Search(text.begin(), text.end(), callback); }
	};

}

#endif
