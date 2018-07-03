#ifndef STINGRAYKIT_THREAD_SYNCVALUE_H
#define STINGRAYKIT_THREAD_SYNCVALUE_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/ParamPassingType.h>
#include <stingraykit/thread/Thread.h>

namespace stingray
{

	template < typename T >
	class SyncValue
	{
		typedef typename GetParamPassingType<T>::ValueT		ParamPassingType;

	public:
		class Lock
		{
			MutexLock	_lock;
			SyncValue&	_value;

		public:
			explicit Lock(SyncValue& val) : _lock(val._mutex), _value(val) { }
			Lock(const Lock& o) : _lock(o._value._mutex), _value(o._value) { }

			T* operator -> ()						{ return &_value._value; }
			const T* operator -> () const			{ return &_value._value; }
			operator T& ()							{ return _value._value; }
			operator const T& () const				{ return _value._value; }
			Lock& operator = (ParamPassingType val)	{ _value._value = val; return *this; }
		};

		class ConstLock
		{
			MutexLock			_lock;
			const SyncValue&	_value;

		public:
			explicit ConstLock(const SyncValue& val) : _lock(val._mutex), _value(val) { }
			ConstLock(const ConstLock& o) : _lock(o._value._mutex), _value(o._value) { }

			const T* operator -> () const	{ return &_value._value; }
			operator const T& () const		{ return _value._value; }
		};

	private:
		Mutex	_mutex;
		T		_value;

	public:
		explicit SyncValue(ParamPassingType value = T()) : _value(value) { }

		T Get() const									{ return ConstLock(*this); }

		Lock operator -> ()								{ return Lock(*this); }
		ConstLock operator -> () const					{ return ConstLock(*this); }
		Lock operator * ()								{ return Lock(*this); }
		ConstLock operator * () const					{ return ConstLock(*this); }

		SyncValue& operator = (ParamPassingType val)	{ Lock(*this) = val; return *this; }
	};

}

#endif
