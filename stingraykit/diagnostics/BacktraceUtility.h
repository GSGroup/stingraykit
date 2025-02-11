#ifndef STINGRAYKIT_DIAGNOSTICS_BACKTRACEUTILITY_H
#define STINGRAYKIT_DIAGNOSTICS_BACKTRACEUTILITY_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/NumericRange.h>

namespace stingray
{

	struct DynamicLibraryInfo
	{
		SimpleNumericRange<uintptr_t>		AddressRange;
		std::string							LibraryPath;

		DynamicLibraryInfo(uintptr_t addressBegin, uintptr_t addressEnd, const std::string& libraryPath);

		std::string ToString() const;
	};


	struct BacktraceUtility
	{
		static std::vector<DynamicLibraryInfo> GetDynamicLibraries();
	};

}

#endif
