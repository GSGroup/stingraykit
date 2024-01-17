#ifndef STINGRAYKIT_COMPARE_MEMBERTOVALUECOMPARER_H
#define STINGRAYKIT_COMPARE_MEMBERTOVALUECOMPARER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
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
		using MemberType = T;
		using ClassType = C;
		using MemberPointer = MemberType (ClassType::*)() const;

		static MemberType GetValue(const ClassType& obj, MemberPointer ptr) { return (obj.*ptr)(); }
	};

	template < typename C, typename T >
	struct MemberExtractor<T C::*>
	{
		using MemberType = T;
		using ClassType = C;
		using MemberPointer = MemberType ClassType::*;

		static const MemberType& GetValue(const ClassType& obj, MemberPointer ptr) { return obj.*ptr; }
	};


	struct AllowDereferencing
	{
		template < typename T > static const T& Process(const T& t)							{ return t; }
		template < typename T > static const T& Process(T* const t)							{ return *t; }
		template < typename T > static const T& Process(const T* const t)					{ return *t; }
		template < typename T > static const T& Process(const shared_ptr<T>& t)				{ return *t; }
		template < typename T > static const T& Process(const shared_ptr<const T>& t)		{ return *t; }
		template < typename T > static const T& Process(const self_count_ptr<T>& t)			{ return *t; }
		template < typename T > static const T& Process(const self_count_ptr<const T>& t)	{ return *t; }
		template < typename T > static const T& Process(const unique_ptr<T>& t)				{ return *t; }
		template < typename T > static const T& Process(const unique_ptr<const T>& t)		{ return *t; }
	};

	struct NoDereferencing
	{
		template < typename T > static const T& Process(const T& t)							{ return t; }
	};


	template < typename MemberPointer, typename Comparer >
	struct CustomMemberComparerWrapper : public comparers::ComparerInfo<Comparer>
	{
		using Extractor = MemberExtractor<MemberPointer>;

	private:
		MemberPointer	_ptr;
		Comparer		_comparer;

	public:
		CustomMemberComparerWrapper(MemberPointer ptr, Comparer comparer)
			: _ptr(ptr), _comparer(comparer)
		{ }

		template < typename ClassType >
		typename comparers::ComparerInfo<Comparer>::RetType operator () (const ClassType& lhs, const ClassType& rhs) const
		{ return _comparer(Extractor::GetValue(lhs, _ptr), Extractor::GetValue(rhs, _ptr)); }
	};


	template < typename MemberPointer, typename Comparer >
	CustomMemberComparerWrapper<MemberPointer, Comparer> CustomMemberComparer(MemberPointer ptr, Comparer comparer)
	{ return CustomMemberComparerWrapper<MemberPointer, Comparer>(ptr, comparer); }



	template < typename MemberPointer, typename Comparer = comparers::Equals, typename DereferencingManager = AllowDereferencing >
	struct MemberExtractorComparer : public comparers::ComparerInfo<Comparer>
	{
		using Extractor = MemberExtractor<MemberPointer>;
		using MemberType = typename Extractor::MemberType;
		using ClassType = typename Extractor::ClassType;

	private:
		MemberPointer	_ptr;
		Comparer		_comparer;

	public:
		MemberExtractorComparer(MemberPointer ptr, Comparer comparer = Comparer())
			: _ptr(ptr), _comparer(comparer)
		{ }

		// Used for upper_bound
		template < typename T >
		typename comparers::ComparerInfo<Comparer>::RetType operator () (MemberType value, const T& obj) const
		{
			const ClassType& derefObj(DereferencingManager::Process(obj));
			return _comparer(value, Extractor::GetValue(derefObj, _ptr));
		}

		// Used for lower_bound
		template < typename T >
		typename comparers::ComparerInfo<Comparer>::RetType operator () (const T& obj, MemberType value) const
		{
			const ClassType& derefObj(DereferencingManager::Process(obj));
			return _comparer(Extractor::GetValue(derefObj, _ptr), value);
		}
	};


	template <typename MemberPointer>
	MemberExtractorComparer<MemberPointer, comparers::Less, AllowDereferencing> CompareMemberLess(MemberPointer ptr)
	{ return MemberExtractorComparer<MemberPointer, comparers::Less, AllowDereferencing >(ptr); }


	template < typename MemberPointer, typename ValueType, typename Comparer = comparers::Equals, typename DereferencingManager = AllowDereferencing >
	struct MemberToValueComparer : public comparers::ComparerInfo<Comparer>
	{
		using Extractor = MemberExtractor<MemberPointer>;
		using ClassType = typename Extractor::ClassType;

	private:
		MemberPointer	_ptr;
		ValueType		_value;
		Comparer		_comparer;

	public:
		MemberToValueComparer(MemberPointer ptr, ValueType value, Comparer comparer = Comparer())
			: _ptr(ptr), _value(value), _comparer(comparer)
		{ }

		template < typename T >
		typename comparers::ComparerInfo<Comparer>::RetType operator () (const T& obj) const
		{
			const ClassType& derefObj(DereferencingManager::Process(obj));
			return _comparer(Extractor::GetValue(derefObj, _ptr), _value);
		}
	};


	template < template < typename > class Comparer, typename MemberPointer, typename ValueType >
	MemberToValueComparer<MemberPointer, ValueType, Comparer<typename MemberExtractor<MemberPointer>::MemberType>> CompareMemberToValue(MemberPointer ptr, ValueType value)
	{ return MemberToValueComparer<MemberPointer, ValueType, Comparer<typename MemberExtractor<MemberPointer>::MemberType>>(ptr, value); }

	template < typename Comparer, typename MemberPointer, typename ValueType >
	MemberToValueComparer<MemberPointer, ValueType, Comparer> CompareMemberToValue(MemberPointer ptr, ValueType value, Comparer cmp = Comparer())
	{ return MemberToValueComparer<MemberPointer, ValueType, Comparer>(ptr, value, cmp); }

}

#endif
