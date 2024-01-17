#ifndef STINGRAYKIT_IO_SELECTMODE_H
#define STINGRAYKIT_IO_SELECTMODE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/toolkit.h>

namespace stingray
{

	struct SelectMode
	{
		STINGRAYKIT_ENUM_VALUES(None = 0, Read = 1, Write = 2, Error = 4, Close = 8, Cancel = 16, Timeout = 32);
		STINGRAYKIT_DECLARE_ENUM_CLASS(SelectMode);
	};
	STINGRAYKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(SelectMode);

}

#endif
