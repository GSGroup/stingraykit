#ifndef STINGRAYKIT_COMPARE_MEMBERTOVALUECOMPARER_H
#define STINGRAYKIT_COMPARE_MEMBERTOVALUECOMPARER_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>
#include <stingraykit/self_counter.h>

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
		static const MemberType& GetValue(const ClassType &obj, MemberPointer ptr) { return obj.*ptr; }
	};

	struct AllowDereferencing
	{
		template <typename T> static const T& Process(const T& t)					{ return t; }
		template <typename T> static const T& Process(const shared_ptr<T>& t)		{ return *t; }
		template <typename T> static const T& Process(const shared_ptr<const T>& t)	{ return *t; }
		template <typename T> static const T& Process(const self_count_ptr<T>& t)		{ return *t; }
		template <typename T> static const T& Process(const self_count_ptr<const T>& t)	{ return *t; }
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



	template <typename MemberPointer, typename Comparer = comparers::Equals, typename DereferencingManager = AllowDereferencing>
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

	template <typename MemberPointer, typename Comparer = comparers::Equals, typename DereferencingManager = AllowDereferencing>
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
	{ return MemberToValueComparer<MemberPointer, Comparer<typename MemberExtractor<MemberPointer>::MemberType> >(ptr, value); }

	template <typename Comparer, typename MemberPointer, typename ValueType>
	MemberToValueComparer<MemberPointer, Comparer> CompareMemberToValue(MemberPointer ptr, ValueType value, Comparer cmp = Comparer())
	{ return MemberToValueComparer<MemberPointer, Comparer>(ptr, value, cmp); }

	template <typename MemberPointer>
	MemberExtractorComparer<MemberPointer, comparers::Less, AllowDereferencing> CompareMemberLess(MemberPointer ptr)
	{ return MemberExtractorComparer<MemberPointer, comparers::Less, AllowDereferencing >(ptr); }


}

#endif
