#ifndef STINGRAYKIT_STRING_UNICODE_H
#define STINGRAYKIT_STRING_UNICODE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>
#include <stingraykit/unique_ptr.h>

#ifdef HAVE_ICU_I18N
#	include <unicode/tblcoll.h>
#endif

namespace stingray
{

	class UnicodeCollator
	{
		STINGRAYKIT_NONCOPYABLE(UnicodeCollator);

	private:
#ifdef HAVE_ICU_I18N
		unique_ptr<icu::RuleBasedCollator>	_collator;
#else
		bool								_caseSensitive;
#endif

	public:
		UnicodeCollator(bool caseSensitive);
		~UnicodeCollator();

		int Compare(const std::string& str1, const std::string& str2) const;
	};
	STINGRAYKIT_DECLARE_PTR(UnicodeCollator);


	std::string Utf8ToLower(const std::string& str);


	template < bool CaseSensitive >
	class UnicodeCmp : public comparers::CmpComparerBase<UnicodeCmp<CaseSensitive> >
	{
	private:
		UnicodeCollatorPtr		_collator;

	public:
		UnicodeCmp() : _collator(make_shared_ptr<UnicodeCollator>(CaseSensitive))
		{ }

		int DoCompare(const std::string& str1, const std::string& str2) const
		{ return _collator->Compare(str1, str2); }
	};
	STINGRAYKIT_DECLARE_TEMPLATE_COMPARERS(Unicode, bool CaseSensitive, CaseSensitive);

}

#endif
