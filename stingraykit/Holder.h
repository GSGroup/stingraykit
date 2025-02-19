#ifndef STINGRAYKIT_HOLDER_H
#define STINGRAYKIT_HOLDER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>

namespace stingray
{

	template < typename ValueType >
	class ScopedHolder
	{
		STINGRAYKIT_NONCOPYABLE(ScopedHolder);

	public:
		using ValuePassingType = typename GetParamPassingType<ValueType>::ValueT;

		using CleanupFuncType = function<void (ValuePassingType)>;

	private:
		ValueType		_handle;
		CleanupFuncType	_cleanupFunc;
		bool			_valid;

	public:
		explicit ScopedHolder(const CleanupFuncType& cleanupFunc)
			: _cleanupFunc(cleanupFunc), _valid(false)
		{ }

		ScopedHolder(ValuePassingType handle, const CleanupFuncType& cleanupFunc)
			: _handle(handle), _cleanupFunc(cleanupFunc), _valid(true)
		{ }

		ScopedHolder(ScopedHolder&& other)
			: _handle(std::move(other._handle)), _cleanupFunc(std::move(other._cleanupFunc)), _valid(other._valid)
		{ other._valid = false; }

		~ScopedHolder()
		{ STINGRAYKIT_TRY_NO_MESSAGE(Cleanup()); }

		ScopedHolder& operator = (ScopedHolder&& other)
		{
			ScopedHolder tmp(std::move(other));
			std::swap(_handle, tmp._handle);
			std::swap(_cleanupFunc, tmp._cleanupFunc);
			std::swap(_valid, tmp._valid);
			return *this;
		}

		bool Valid() const
		{ return _valid; }

		ValuePassingType Get() const
		{
			Check();
			return _handle;
		}

		ValueType Release()
		{
			Check();
			_valid = false;
			return _handle;
		}

		void Clear()
		{ Cleanup(); }

		void Set(ValuePassingType handle)
		{
			Cleanup();
			_handle = handle;
			_valid = true;
		}

	private:
		void Check() const
		{ STINGRAYKIT_CHECK(_valid, NotInitializedException()); }

		void Cleanup()
		{
			if (_valid)
				_cleanupFunc(_handle);
			_valid = false;
		}
	};


	template < >
	class ScopedHolder<void>
	{
		STINGRAYKIT_NONCOPYABLE(ScopedHolder);

	public:
		using CleanupFuncType = function<void ()>;

	private:
		CleanupFuncType	_cleanupFunc;
		bool			_valid;

	public:
		explicit ScopedHolder(const CleanupFuncType& cleanupFunc, bool valid = false)
			: _cleanupFunc(cleanupFunc), _valid(valid)
		{ }

		ScopedHolder(ScopedHolder&& other)
			: _cleanupFunc(std::move(other._cleanupFunc)), _valid(other._valid)
		{ other._valid = false; }

		~ScopedHolder()
		{ STINGRAYKIT_TRY_NO_MESSAGE(Cleanup()); }

		ScopedHolder& operator = (ScopedHolder&& other)
		{
			ScopedHolder tmp(std::move(other));
			std::swap(_cleanupFunc, tmp._cleanupFunc);
			std::swap(_valid, tmp._valid);
			return *this;
		}

		bool Valid() const		{ return _valid; }
		void Clear()			{ Cleanup(); _valid = false; }
		void Set()				{ Cleanup(); _valid = true; }

	private:
		void Cleanup() const	{ if (_valid) _cleanupFunc(); }
	};


	template < typename ValueType >
	class SharedHolder
	{
		using Impl = ScopedHolder<ValueType>;
		STINGRAYKIT_DECLARE_PTR(Impl);

		using ValuePassingType = typename GetParamPassingType<ValueType>::ValueT;

		using CleanupFuncType = function<void (ValuePassingType)>;

	private:
		ImplPtr	_impl;

	public:
		SharedHolder()
		{ }

		explicit SharedHolder(const CleanupFuncType& cleanupFunc)
			: _impl(make_shared_ptr<Impl>(cleanupFunc))
		{ }

		SharedHolder(ValuePassingType handle, const CleanupFuncType& cleanupFunc)
			: _impl(make_shared_ptr<Impl>(handle, cleanupFunc))
		{ }

		~SharedHolder()
		{ }

		bool Valid() const					{ return _impl && _impl->Valid(); }
		ValuePassingType Get() const		{ return _impl->Get(); }
		void Clear()						{ _impl->Clear(); }
		void Set(ValuePassingType handle)	{ _impl->Set(handle); }
	};


	template < >
	class SharedHolder<void>
	{
		using Impl = ScopedHolder<void>;
		STINGRAYKIT_DECLARE_PTR(Impl);

		using CleanupFuncType = function<void ()>;

	private:
		ImplPtr	_impl;

	public:
		SharedHolder()
		{ }

		explicit SharedHolder(const CleanupFuncType& cleanupFunc)
			: _impl(make_shared_ptr<Impl>(cleanupFunc))
		{ }

		~SharedHolder()
		{ }

		bool Valid() const	{ return _impl && _impl->Valid(); }
		void Clear()		{ _impl->Clear(); }
		void Set()			{ _impl->Set(); }

		static SharedHolder<void> CreateInitialized(const CleanupFuncType& cleanupFunc)
		{
			SharedHolder<void> result(cleanupFunc);
			result.Set();
			return result;
		}
	};

}

#endif
