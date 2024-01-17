#ifndef STINGRAYKIT_CORE_NONCOPYABLE_H
#define STINGRAYKIT_CORE_NONCOPYABLE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

namespace stingray
{

#define STINGRAYKIT_NONASSIGNABLE(ClassName) \
	public: \
		ClassName& operator = (const ClassName&) = delete

#define STINGRAYKIT_NONCOPYABLE(ClassName) \
	public: \
		ClassName(const ClassName&) = delete; \
		STINGRAYKIT_NONASSIGNABLE(ClassName)

#define STINGRAYKIT_DEFAULTCOPYABLE(ClassName) \
	public: \
		ClassName(const ClassName&) = default; \
		ClassName& operator = (const ClassName&) = default

#define STINGRAYKIT_NONMOVABLE(ClassName) \
	public: \
		ClassName(ClassName&&) = delete; \
		ClassName& operator = (ClassName&&) = delete

#define STINGRAYKIT_DEFAULTMOVABLE(ClassName) \
	public: \
		ClassName(ClassName&&) = default; \
		ClassName& operator = (ClassName&&) = default


	class NonCopyable
	{
		STINGRAYKIT_NONCOPYABLE(NonCopyable);

	protected:
		NonCopyable()  { }
		~NonCopyable() { }
	};

}

#endif
