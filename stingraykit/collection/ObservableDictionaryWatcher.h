#ifndef STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYWATCHER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYWATCHER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IObservableDictionary.h>

#include <stingraykit/FunctionToken.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_, typename CompareType_ >
	class ObservableDictionaryWatcher
	{
	public:
		typedef typename GetParamPassingType<KeyType_>::ValueT		KeyPassingType;
		typedef typename GetParamPassingType<ValueType_>::ValueT	ValuePassingType;

		typedef IReadonlyObservableDictionary<KeyType_, ValueType_>	DictionaryType;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		typedef function<void (CollectionOp, ValuePassingType)>		WatchValueFunc;

	private:
		KeyType_					_key;

		TaskLifeToken				_taskLifeToken;
		WatchValueFunc				_handler;

		ThreadlessTokenPool			_connections;

	public:
		ObservableDictionaryWatcher(const DictionaryTypePtr& dict, KeyPassingType key, const WatchValueFunc& handler, const ITaskExecutorPtr& worker = null)
			:	_key(key),
				_handler(worker ? MakeAsyncFunction(worker, handler, _taskLifeToken.GetExecutionTester()) : handler)
		{
			_connections += MakeToken<FunctionToken>(bind(&TaskLifeToken::Release, ref(_taskLifeToken)));

			MutexLock l(dict->GetSyncRoot());
			_connections += dict->OnChanged().connect(bind(&ObservableDictionaryWatcher::DictionaryChangedHandler, this, _1, _2, _3), false);

			ValueType_ value;
			if (dict->TryGet(_key, value))
				STINGRAYKIT_TRY_NO_MESSAGE(_handler(CollectionOp::Added, value));
		}

	private:
		void DictionaryChangedHandler(CollectionOp op, KeyPassingType key, ValuePassingType value)
		{
			if (CompareType_()(_key, key))
				_handler(op, value);
		}
	};

	/** @} */

}

#endif
