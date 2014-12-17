#ifndef STINGRAY_TOOLKIT_MEMBERTOVALUECOMPARER_H
#define STINGRAY_TOOLKIT_MEMBERTOVALUECOMPARER_H

#include <stingray/toolkit/function/function_info.h>
#include <stingray/toolkit/shared_ptr.h>

namespace stingray
{


	template < typename T >
	struct MemberExtractor;

	template < typename C, typename T >
	struct MemberExtractor<T (C::*)() const>
	{
		typedef T MemberType;
		typedef C ClassType;
		typedef MemberType (ClassType::*MemberPointer)() const;
		static MemberType GetValue(const ClassType &obj, MemberPointer ptr) { return (obj.*ptr)(); }
	};

	template < typename C, typename T >
	struct MemberExtractor<T C::*>
	{
		typedef T MemberType;
		typedef C ClassType;
		typedef MemberType ClassType::*MemberPointer;
		static MemberType GetValue(const ClassType &obj, MemberPointer ptr) { return obj.*ptr; }
	};

	struct AllowDereferencing
	{
		template <typename T> static const T& Process(const T& t)					{ return t; }
		template <typename T> static const T& Process(const shared_ptr<T>& t)		{ return *t; }
		template <typename T> static const T& Process(const shared_ptr<const T>& t)	{ return *t; }
	};

	struct NoDereferencing
	{
		template <typename T> static const T& Process(const T& t)				{ return t; }
	};


	template<typename MemberPointerT, typename ComparerT>
	struct CustomMemberComparerWrapper : public function_info<ComparerT>
	{
		typedef MemberExtractor<MemberPointerT> Extractor;
	private:
		MemberPointerT _memberPointer;
		ComparerT _comparer;
	public:
		CustomMemberComparerWrapper(MemberPointerT memberPointer, ComparerT comparer)
			: _memberPointer(memberPointer), _comparer(comparer)
		{}

		template <typename ClassType>
		typename function_info<ComparerT>::RetType Compare(const ClassType &lhs, const ClassType &rhs) const
		{
			return _comparer(Extractor::GetValue(lhs, _memberPointer), Extractor::GetValue(rhs, _memberPointer));
		}
	};


	template<typename MemberPointerT, typename ComparerT>
	CustomMemberComparerWrapper<MemberPointerT, ComparerT> CustomMemberComparer(MemberPointerT pointer, ComparerT comparer)
	{ return CustomMemberComparerWrapper<MemberPointerT, ComparerT>(pointer, comparer); }



	template <typename MemberPointer, typename Comparer = std::equal_to<typename MemberExtractor<MemberPointer>::MemberType>, typename DereferencingManager = AllowDereferencing>
	struct MemberExtractorComparer
	{
		typedef MemberExtractor<MemberPointer>		Extractor;
		typedef typename Extractor::MemberType		MemberType;
		typedef typename Extractor::ClassType		ClassType;
	private:
		MemberPointer	_ptr;
		Comparer		_comparer;
	public:
		MemberExtractorComparer(MemberPointer ptr, Comparer comparer = Comparer())
			: _ptr(ptr), _comparer(comparer)
		{}

		// Used for upper_bound
		template <typename T>
		bool operator ()(MemberType value, const T &obj) const
		{
			const ClassType& deref_obj(DereferencingManager::Process(obj));
			return _comparer(value, Extractor::GetValue(deref_obj, _ptr));
		}

		// Used for lower_bound
		template <typename T>
		bool operator ()(const T &obj, MemberType value) const
		{
			const ClassType& deref_obj(DereferencingManager::Process(obj));
			return _comparer(Extractor::GetValue(deref_obj, _ptr), value);
		}
	};

	template <typename MemberPointer, typename Comparer = std::equal_to<typename MemberExtractor<MemberPointer>::MemberType>, typename DereferencingManager = AllowDereferencing>
	struct MemberToValueComparer : public function_info<bool, typename TypeList<const typename MemberExtractor<MemberPointer>::ClassType &>::type>
	{
		typedef MemberExtractor<MemberPointer>		Extractor;
		typedef typename Extractor::MemberType		MemberType;
		typedef typename Extractor::ClassType		ClassType;
	private:
		MemberPointer	_ptr;
		MemberType		_value;
		Comparer		_comparer;
	public:
		MemberToValueComparer(MemberPointer ptr, MemberType memberValue, Comparer comparer = Comparer())
			: _ptr(ptr), _value(memberValue), _comparer(comparer)
		{}

		template <typename T>
		bool operator ()(const T &obj) const
		{
			const ClassType& deref_obj(DereferencingManager::Process(obj));
			return _comparer(Extractor::GetValue(deref_obj, _ptr), _value);
		}
	};

	template <template<typename> class Comparer, typename MemberPointer, typename ValueType>
	MemberToValueComparer<MemberPointer, Comparer<typename MemberExtractor<MemberPointer>::MemberType> > CompareMemberToValue(MemberPointer ptr, ValueType value)
	{
		return MemberToValueComparer<MemberPointer, Comparer<typename MemberExtractor<MemberPointer>::MemberType> >(ptr, value);
	}

	template <typename Comparer, typename MemberPointer, typename ValueType>
	MemberToValueComparer<MemberPointer, Comparer> CompareMemberToValue(MemberPointer ptr, ValueType value, Comparer cmp = Comparer())
	{ return MemberToValueComparer<MemberPointer, Comparer>(ptr, value, cmp); }


	template <template<typename> class Comparer, typename MemberPointer>
	MemberExtractorComparer<MemberPointer, Comparer<typename MemberExtractor<MemberPointer>::MemberType> > CompareMember(MemberPointer ptr)
	{
		return MemberExtractorComparer<MemberPointer, Comparer<typename MemberExtractor<MemberPointer>::MemberType> >(ptr, Comparer<typename MemberExtractor<MemberPointer>::MemberType>());
	}

	template <template<typename> class Comparer, typename DereferencingManager, typename MemberPointer>
	MemberExtractorComparer<MemberPointer, Comparer<typename MemberExtractor<MemberPointer>::MemberType>, DereferencingManager> CompareMember(MemberPointer ptr)
	{
		return MemberExtractorComparer<MemberPointer, Comparer<typename MemberExtractor<MemberPointer>::MemberType>, DereferencingManager >(ptr, Comparer<typename MemberExtractor<MemberPointer>::MemberType>());
	}

	template <typename MemberPointer>
	MemberExtractorComparer<MemberPointer, std::less<typename MemberExtractor<MemberPointer>::MemberType>, AllowDereferencing> CompareMemberLess(MemberPointer ptr)
	{
		return MemberExtractorComparer<MemberPointer, std::less<typename MemberExtractor<MemberPointer>::MemberType>, AllowDereferencing >(ptr, std::less<typename MemberExtractor<MemberPointer>::MemberType>());
	}


}

#endif
