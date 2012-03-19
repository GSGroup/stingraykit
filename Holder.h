#ifndef __GS_DVRLIB_TOOLKIT_HOLDER_H__
#define __GS_DVRLIB_TOOLKIT_HOLDER_H__


#include <dvrlib/toolkit/bind.h>
#include <dvrlib/toolkit/shared_ptr.h>

namespace dvrlib
{


	template < typename NativeType >
	class ScopedHolder
	{
		TOOLKIT_NONCOPYABLE(ScopedHolder);
		typedef function<void(const NativeType&)>		CleanupFuncType;

	private:
		NativeType		_handle;
		CleanupFuncType	_cleanupFunc;
		bool			_valid;

	public:
		ScopedHolder(const CleanupFuncType& cleanupFunc)
			: _cleanupFunc(cleanupFunc), _valid(false)
		{}

		ScopedHolder(const NativeType& handle, const CleanupFuncType& cleanupFunc)
			: _handle(handle), _cleanupFunc(cleanupFunc), _valid(true)
		{}

		~ScopedHolder()			{ Cleanup(); }

		bool Valid() const		{ return _valid; }

		NativeType Get() const	{ Check();					return _handle; }
		NativeType Release()	{ Check(); _valid = false;	return _handle; }

		void Reset(const NativeType& handle)
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
		typedef typename Impl::CleanupFuncType CleanupFuncType;

	private:
		ImplPtr	_impl;

	public:
		SharedHolder()
		{}

		SharedHolder(const NativeType& handle, const CleanupFuncType& cleanupFunc)
			: _impl(make_shared<Impl>(handle, cleanupFunc))
		{}

		~SharedHolder()
		{}

		NativeType Get() const { return _impl->Get(); }
	};


}


#endif
