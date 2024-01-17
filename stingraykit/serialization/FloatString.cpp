// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/serialization/FloatString.h>

#include <stingraykit/SystemException.h>

#include <stdio.h>

namespace stingray
{

	FloatString::FloatString(double value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%lg", value);
		STINGRAYKIT_CHECK(r > 0, "cannot serialize double value");

		_str = buf;
	}


	double FloatString::ToDouble() const
	{
		double value;
		STINGRAYKIT_CHECK(sscanf(_str.c_str(), "%lg", &value) == 1, SystemException("cannot parse double value " + _str));
		return value;
	}

}
