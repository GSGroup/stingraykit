#ifndef STINGRAYKIT_DYNAMIC_CASTER_H
#define STINGRAYKIT_DYNAMIC_CASTER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/MetaProgramming.h>
#include <stingraykit/exception.h>
#include <stingraykit/Dummy.h>
#include <stingraykit/TypeInfo.h>


namespace stingray
{

	namespace Detail
	{
		template <typename Src_, typename Dst_, typename Enabler = void>
		struct PointersCaster
		{
			static Dst_* Do(Src_* src)
			{
				// Src_ and Dst_ types must be complete
				(void)sizeof(Src_); (void)sizeof(Dst_);
				return dynamic_cast<Dst_*>(src);
			}
		};


		template <typename Src_, typename Dst_>
		struct PointersCaster<Src_, Dst_, typename EnableIf<Inherits<Src_, Dst_>::Value, void>::ValueT>
		{
			static Dst_* Do(Src_* src)
			{ return src; }
		};


		template <typename Src_, typename Dst_, typename Enabler = void>
		struct DynamicCastImpl;


		template <typename Src_, typename DstReference_>
		struct DynamicCastImpl<Src_, DstReference_, typename EnableIf<IsReference<DstReference_>::Value, void>::ValueT>
		{
			static DstReference_ Do(Src_& src)
			{
				typedef typename Dereference<DstReference_>::ValueT Dst_;
				Dst_* dst = PointersCaster<Src_, Dst_>::Do(&src);
				STINGRAYKIT_CHECK(dst, InvalidCastException(TypeInfo(src).GetName(), TypeInfo(typeid(Dst_)).GetName()));
				return *dst;
			}
		};


		template <typename SrcPtr_, typename DstPtr_>
		struct DynamicCastImpl<SrcPtr_, DstPtr_, typename EnableIf<IsPointer<SrcPtr_>::Value && IsPointer<DstPtr_>::Value, void>::ValueT>
		{
			static DstPtr_ Do(SrcPtr_ src)
			{ return PointersCaster<typename Depointer<SrcPtr_>::ValueT, typename Depointer<DstPtr_>::ValueT>::Do(src); }
		};


		template <typename Src_, typename Dst_>
		struct DynamicCastImpl<Src_, Dst_, typename EnableIf<IsPointer<Src_>::Value != IsPointer<Dst_>::Value, void>::ValueT>
		{
			// Explicitly prohibit casting if one of the types is a pointer and another one is not
		};
	}


	template <typename Dst_, typename Src_>
	Dst_ DynamicCast(Src_& src)
	{ return Detail::DynamicCastImpl<Src_, Dst_>::Do(src); }


	template <typename Dst_, typename Src_>
	Dst_ DynamicCast(const Src_& src)
	{ return Detail::DynamicCastImpl<const Src_, Dst_>::Do(src); }


	namespace Detail
	{
		template <typename Src_, typename Enabler = void>
		class DynamicCasterImpl
		{
		private:
			Src_& _src;

		public:
			explicit DynamicCasterImpl(Src_& src) : _src(src)
			{ }

			template <typename Dst_> operator Dst_& () const
			{ return DynamicCast<Dst_&, Src_>(_src); }
			template <typename Dst_> operator const Dst_& () const
			{ return DynamicCast<const Dst_ &, Src_>(_src); }
		};


		template <typename SrcPtr_>
		class DynamicCasterImpl<SrcPtr_, typename EnableIf<IsPointer<SrcPtr_>::Value, void>::ValueT>
		{
		private:
			SrcPtr_ _src;

		public:
			explicit DynamicCasterImpl(SrcPtr_ src) : _src(src)
			{ }

			template <typename Dst_> operator Dst_* () const
			{ return DynamicCast<Dst_*, SrcPtr_>(_src); }
		};
	}


	template <typename Src_>
	Detail::DynamicCasterImpl<Src_> dynamic_caster(Src_& src)
	{ return Detail::DynamicCasterImpl<Src_>(src); }


	template <typename Src_>
	Detail::DynamicCasterImpl<const Src_> dynamic_caster(const Src_& src)
	{ return Detail::DynamicCasterImpl<const Src_>(src); }


	namespace Detail
	{
		template <typename Src_>
		class CheckedDynamicCaster
		{
			Src_         _src;
			ToolkitWhere _where;

		public:
			CheckedDynamicCaster(const Src_& src, ToolkitWhere where) :
				_src(src), _where(where)
			{ }

			template <typename Dst_>
			operator Dst_() const
			{
				if (!_src)
					return null;

				Dst_ dst = DynamicCast<Dst_>(_src);
				if (dst)
					return dst;

				throw stingray::Detail::MakeException(InvalidCastException(TypeInfo(*_src).GetName(), TypeInfo(typeid(Dst_)).GetName()), _where);
			}
		};


		template <typename Src_>
		CheckedDynamicCaster<Src_> checked_dynamic_caster(const Src_& src, ToolkitWhere where)
		{ return CheckedDynamicCaster<Src_>(src, where); }
	}


#define STINGRAYKIT_CHECKED_DYNAMIC_CASTER(Expr_) stingray::Detail::checked_dynamic_caster(Expr_, STINGRAYKIT_WHERE)


}


#endif
