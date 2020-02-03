#ifndef STINGRAYKIT_CORE_FINAL_H
#define STINGRAYKIT_CORE_FINAL_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


namespace stingray
{

	// Usage:
	// class A : STINGRAYKIT_FINAL(A) { };

#define STINGRAYKIT_FINAL(ClassName_) private virtual stingray::Final<ClassName_>
#define STINGRAYKIT_NONPOLYMORPHIC(ClassName_) STINGRAYKIT_FINAL(ClassName_), public virtual stingray::NonPolymorphicMarker

	class NonPolymorphicMarker { };

	template < typename T >
	class Final
	{
#if defined __GNUC__ && __GNUG__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ > 6
		friend T;
#else
	protected:
#endif
		Final() { }
		Final(const Final&) { }
		~Final() { }

		Final& operator = (const Final&) { return *this; }
	};

}


#endif
