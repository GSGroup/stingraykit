#ifndef STINGRAYKIT_ACTIONTRANSACTION_H
#define STINGRAYKIT_ACTIONTRANSACTION_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <vector>

#include <stingraykit/function/function.h>


namespace stingray
{

	class ActionTransaction
	{
		STINGRAYKIT_NONCOPYABLE(ActionTransaction);

		typedef function<void ()> ActionFunc;
		typedef function<void ()> RollbackFunc;

		typedef std::vector<RollbackFunc> RollbackSequence;

	private:
		RollbackSequence	_rollbackSequence;
		bool				_active;

	public:
		ActionTransaction();
		~ActionTransaction();

		void Execute(const ActionFunc& actionFunc, const RollbackFunc& rollbackFunc);

		void Commit();
		void Rollback();
	};

}


#endif
