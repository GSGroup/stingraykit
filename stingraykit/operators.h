#ifndef STINGRAYKIT_OPERATORS_H
#define STINGRAYKIT_OPERATORS_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#define STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(ClassName) \
		DETAIL_GENERATE_COMPARISON_OPERATORS_FROM_LESS(, ClassName)

#define STINGRAYKIT_GENERATE_CONSTEXPR_COMPARISON_OPERATORS_FROM_LESS(ClassName) \
		DETAIL_GENERATE_COMPARISON_OPERATORS_FROM_LESS(constexpr, ClassName)

#define DETAIL_GENERATE_COMPARISON_OPERATORS_FROM_LESS(ConstexprInlineDecl, ClassName) \
		ConstexprInlineDecl bool operator > (const ClassName& other) const \
		{ return other < *this; } \
		ConstexprInlineDecl bool operator <= (const ClassName& other) const \
		{ return !(other < *this); } \
		ConstexprInlineDecl bool operator >= (const ClassName& other) const \
		{ return !(*this < other); } \
		ConstexprInlineDecl bool operator != (const ClassName& other) const \
		{ return (other < *this) || (*this < other); } \
		ConstexprInlineDecl bool operator == (const ClassName& other) const \
		{ return !(other != *this); }


#define STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_COMPARE(ClassName) \
		DETAIL_GENERATE_COMPARISON_OPERATORS_FROM_COMPARE(, ClassName)

#define STINGRAYKIT_GENERATE_CONSTEXPR_COMPARISON_OPERATORS_FROM_COMPARE(ClassName) \
		DETAIL_GENERATE_COMPARISON_OPERATORS_FROM_COMPARE(constexpr, ClassName)

#define DETAIL_GENERATE_COMPARISON_OPERATORS_FROM_COMPARE(ConstexprInlineDecl, ClassName) \
		ConstexprInlineDecl bool operator < (const ClassName& other) const { return Compare(other) <  0; } \
		ConstexprInlineDecl bool operator > (const ClassName& other) const { return Compare(other) >  0; } \
		ConstexprInlineDecl bool operator <= (const ClassName& other) const { return Compare(other) <= 0; } \
		ConstexprInlineDecl bool operator >= (const ClassName& other) const { return Compare(other) >= 0; } \
		ConstexprInlineDecl bool operator == (const ClassName& other) const { return Compare(other) == 0; } \
		ConstexprInlineDecl bool operator != (const ClassName& other) const { return Compare(other) != 0; }


#define STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(ClassName) \
		DETAIL_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(, ClassName)

#define STINGRAYKIT_GENERATE_CONSTEXPR_EQUALITY_OPERATORS_FROM_EQUAL(ClassName) \
		DETAIL_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(constexpr, ClassName)

#define DETAIL_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(ConstexprInlineDecl, ClassName) \
		ConstexprInlineDecl bool operator != (const ClassName& other) const \
		{ return !(*this == other); }


#define STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(ClassName) \
		DETAIL_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(, ClassName)

#define STINGRAYKIT_GENERATE_CONSTEXPR_RELATIONAL_OPERATORS_FROM_LESS(ClassName) \
		DETAIL_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(constexpr, ClassName)

#define DETAIL_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(ConstexprInlineDecl, ClassName) \
		ConstexprInlineDecl bool operator > (const ClassName& other) const \
		{ return other < *this; } \
		ConstexprInlineDecl bool operator <= (const ClassName& other) const \
		{ return !(other < *this); } \
		ConstexprInlineDecl bool operator >= (const ClassName& other) const \
		{ return !(*this < other); }


#define STINGRAYKIT_GENERATE_NON_MEMBER_EQUALITY_OPERATORS_FROM_EQUAL(TemplateConstexprInlineDecl, LeftClassName, RightClassName) \
		TemplateConstexprInlineDecl \
		bool operator != (const LeftClassName& lhs, const RightClassName& rhs) \
		{ return !(lhs == rhs); }

#define STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(TemplateConstexprInlineDecl, LeftClassName, RightClassName) \
		TemplateConstexprInlineDecl \
		bool operator > (const LeftClassName& lhs, const RightClassName& rhs) \
		{ return rhs < lhs; } \
		TemplateConstexprInlineDecl \
		bool operator <= (const LeftClassName& lhs, const RightClassName& rhs) \
		{ return !(rhs < lhs); } \
		TemplateConstexprInlineDecl \
		bool operator >= (const LeftClassName& lhs, const RightClassName& rhs) \
		{ return !(lhs < rhs); }

#define STINGRAYKIT_GENERATE_NON_MEMBER_COMMUTATIVE_EQUALITY_OPERATORS_FROM_EQUAL(TemplateConstexprInlineDecl, FirstClassName, SecondClassName) \
		TemplateConstexprInlineDecl \
		bool operator == (const SecondClassName& lhs, const FirstClassName& rhs) \
		{ return rhs == lhs; } \
		TemplateConstexprInlineDecl \
		bool operator != (const FirstClassName& lhs, const SecondClassName& rhs) \
		{ return !(lhs == rhs); } \
		TemplateConstexprInlineDecl \
		bool operator != (const SecondClassName& lhs, const FirstClassName& rhs) \
		{ return !(rhs == lhs); }

#endif
