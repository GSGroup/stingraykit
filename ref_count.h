#ifndef STINGRAY_TOOLKIT_REF_COUNT_H
#define STINGRAY_TOOLKIT_REF_COUNT_H


#include <assert.h>
#include <stingray/toolkit/Atomic.h>
#include <stingray/toolkit/MetaProgramming.h>


/*! \cond GS_INTERNAL */

namespace stingray
{

	namespace Detail
	{
		void DoLogAddRef(const char* className, atomic_int_type refs, const void* ptrVal);
		void DoLogRelease(const char* className, atomic_int_type refs, const void* ptrVal);
	}


	template < typename UserDataType >
	class basic_ref_count_data
	{
		TOOLKIT_NONCOPYABLE(basic_ref_count_data);

	public:
		atomic_int_type		Value;
		atomic_int_type		SelfCount;
		UserDataType		UserData;

		FORCE_INLINE basic_ref_count_data(const UserDataType& userData)
			: Value(1), SelfCount(1), UserData(userData)
		{ }
	};

	template < >
	class basic_ref_count_data<NullType>
	{
		TOOLKIT_NONCOPYABLE(basic_ref_count_data);

	public:
		atomic_int_type		Value;
		atomic_int_type		SelfCount;

		FORCE_INLINE basic_ref_count_data()
			: Value(1), SelfCount(1)
		{ }

		FORCE_INLINE basic_ref_count_data(const NullType& userData)
			: Value(1), SelfCount(1)
		{ }
	};


	template < typename UserDataType >
	class basic_ref_count
	{
		typedef basic_ref_count_data<UserDataType>		Data;

		Data		*_value;

	public:
		FORCE_INLINE basic_ref_count(const NullPtrType&) : _value() {}
		FORCE_INLINE basic_ref_count() :
			_value(new Data)
		{}
		FORCE_INLINE basic_ref_count(const UserDataType& userData) :
			_value(new Data(userData))
		{}

		FORCE_INLINE basic_ref_count(const basic_ref_count& other) :
			_value(other._value)
		{ if (_value) add_ref_self(); }

		FORCE_INLINE ~basic_ref_count() { if (_value) release_self(); }

		FORCE_INLINE basic_ref_count& operator = (const basic_ref_count& other)
		{
			basic_ref_count tmp(other);
			swap(tmp);
			return *this;
		}

		FORCE_INLINE const UserDataType& GetUserData() const { assert(_value); return _value->UserData; }
		FORCE_INLINE bool IsNull() const { return !_value; }

		FORCE_INLINE atomic_int_type get() const	{ assert(_value); return _value->Value; }
		FORCE_INLINE atomic_int_type add_ref()	{ assert(_value); return Atomic::Inc(_value->Value); }
		FORCE_INLINE atomic_int_type release()	{ assert(_value); return Atomic::Dec(_value->Value); }
		FORCE_INLINE bool release_if_unique()	{ assert(_value); return Atomic::CompareAndExchange(_value->Value, 1, 0) == 1; }

		atomic_int_type add_ref(const char* className, const void* ptrVal)
		{ atomic_int_type res = add_ref(); Detail::DoLogAddRef(className, res, ptrVal); return res; }

		atomic_int_type release(const char* className, const void* ptrVal)
		{ atomic_int_type res = release(); Detail::DoLogRelease(className, res, ptrVal); return res; }

		bool release_if_unique(const char* className, const void* ptrVal)
		{
			bool res = release_if_unique();
			if (res)
				Detail::DoLogRelease(className, 0, ptrVal);
			return res;
		}

		FORCE_INLINE void swap(basic_ref_count& other)
		{
			std::swap(_value, other._value);
		}

		const Data* get_ptr() const
		{ return _value; }

	private:
		FORCE_INLINE atomic_int_type add_ref_self()
		{
			assert(_value);
			atomic_int_type result = Atomic::Inc(_value->SelfCount);
			assert(result > 0);
			return result;
		}
		FORCE_INLINE atomic_int_type release_self()
		{
			assert(_value);
			atomic_int_type result = Atomic::Dec(_value->SelfCount);
			assert(result >= 0);
			if (result == 0)
				delete _value;
			return result;
		}
	};


	typedef basic_ref_count<NullType>		ref_count;


}

/*! \endcond */


#endif
