#ifndef STINGRAYKIT_GETMEMBER_H
#define STINGRAYKIT_GETMEMBER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/function_info.h>

namespace stingray
{

	namespace Detail
	{

		template < typename C, typename T >
		class MemberGetter : public function_info<typename RemoveConst<T>::ValueT, UnspecifiedParamTypes>
		{
			using base = function_info<typename RemoveConst<T>::ValueT, UnspecifiedParamTypes>;
			using MemberPointer = T C::*;

		private:
			MemberPointer	_member;

		public:
			explicit MemberGetter(MemberPointer member) : _member(member) { }

			typename base::RetType operator () (const C& c) const				{ return c.*_member; }
			typename base::RetType operator () (const shared_ptr<C>& c) const	{ return (*c).*_member; }
		};

	}

	template < typename C, typename T >
	Detail::MemberGetter<C, T> GetMember(T C::* const member) { return Detail::MemberGetter<C, T>(member); }

}

#endif
