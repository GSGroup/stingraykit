#ifndef STINGRAYKIT_SAFE_BOOL_H
#define STINGRAYKIT_SAFE_BOOL_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


namespace stingray 
{

	namespace Detail
	{
		class safe_bool_base
		{
		protected:
			void this_type_does_not_support_comparisons() const {}

			inline safe_bool_base() {}
			inline ~safe_bool_base() {}

			inline safe_bool_base(const safe_bool_base&) {}
			inline safe_bool_base& operator=(const safe_bool_base&) {return *this;}
		};
	}


	template <typename T = void> class safe_bool : private Detail::safe_bool_base {
		typedef void (safe_bool::*bool_type)() const;
	public:
		inline operator bool_type() const
		{
			return (static_cast<const T*>(this))->boolean_test()? &safe_bool::this_type_does_not_support_comparisons : 0;
		}

	protected:
		inline ~safe_bool() {}

	private:
		inline bool operator == (const safe_bool& other) const { return this == &other; }
		inline bool operator != (const safe_bool& other) const { return !(*this == other); }
	};

}

#endif
