#ifndef STINGRAY_TOOLKIT_HOLDER_H
#define STINGRAY_TOOLKIT_HOLDER_H


#include <stingray/toolkit/bind.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	template < typename NativeType >
	class ScopedHolder
	{
		TOOLKIT_NONCOPYABLE(ScopedHolder);
		typedef function<void(NativeType)>		CleanupFuncType;

	private:
		NativeType		_handle;
		CleanupFuncType	_cleanupFunc;
		bool			_valid;

	public:
		ScopedHolder(const CleanupFuncType& cleanupFunc)
			: _cleanupFunc(cleanupFunc), _valid(false)
		{}

		ScopedHolder(NativeType handle, const CleanupFuncType& cleanupFunc)
			: _handle(handle), _cleanupFunc(cleanupFunc), _valid(true)
		{}

		~ScopedHolder()			{ Cleanup(); }

		bool Valid() const		{ return _valid; }

		NativeType Get() const	{ Check();					return _handle; }
		NativeType Release()	{ Check(); _valid = false;	return _handle; }

		void Clear()
		{
			Cleanup();
			_valid = false;
		}

		void Set(NativeType handle)
		{
			Cleanup();
			_handle = handle;
			_valid = true;
		}

	private:
		void Check() const { TOOLKIT_CHECK(_valid, std::logic_error("ScopedHolder is not valid!")); }
		void Cleanup() const
		{
			if (_valid)
				_cleanupFunc(_handle);
		}
	};


	template < typename NativeType >
	class SharedHolder
	{
		typedef ScopedHolder<NativeType> Impl;
		TOOLKIT_DECLARE_PTR(Impl);
		typedef function<void(NativeType)>		CleanupFuncType;

	private:
		ImplPtr	_impl;

	public:
		SharedHolder()
		{}

		SharedHolder(const CleanupFuncType& cleanupFunc)
			: _impl(make_shared<Impl>(cleanupFunc))
		{}

		SharedHolder(NativeType handle, const CleanupFuncType& cleanupFunc)
			: _impl(make_shared<Impl>(handle, cleanupFunc))
		{}

		~SharedHolder()
		{}

		bool Valid() const		{ return _impl && _impl->Valid(); }

		NativeType Get() const	{ return _impl->Get(); }

		void Clear()
		{ _impl->Clear(); }

		void Set(NativeType handle)
		{ _impl->Set(handle); }
	};

}


#endif
