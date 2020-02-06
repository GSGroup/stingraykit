#ifndef STINGRAYKIT_STRING_STRINGFORMAT_H
#define STINGRAYKIT_STRING_STRINGFORMAT_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/StringUtils.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace Detail
	{

		template < typename TupleParams >
		struct TupleToStringHelper
		{
		public:
			static void ItemToString(string_ostream& result, const Tuple<TupleParams>& tuple, size_t index, size_t width)
			{
				if (index != 0)
					TupleToStringHelper<typename TupleParams::Next>::ItemToString(result, tuple.GetTail(), index - 1, width);
				else if (width == 0)
					ToString(result, tuple.GetHead()); //quick no width variant
				else
				{
					const std::string itemStr = ToString(tuple.GetHead());

					if (itemStr.size() < width)
						result << std::string(width - itemStr.size(), '0') << itemStr;
					else
						result << itemStr;
				}
			}
		};


		template < >
		struct TupleToStringHelper<TypeListEndNode>
		{
			static void ItemToString(string_ostream&, const Tuple<TypeListEndNode>& tuple, size_t index, size_t width)
			{ STINGRAYKIT_THROW("Tuple item index mismatch!"); }
		};


		template < typename FormatFragments_, typename TupleParams >
		static void StringFormatImpl(string_ostream& result, FormatFragments_ formatFragments, const Tuple<TupleParams>& params)
		{
			STINGRAYKIT_CHECK((Count(formatFragments) % 2) == 1, "Format mismatch: no corresponding %");

			for (size_t idx = 0; formatFragments.Valid(); ++idx, formatFragments.Next())
			{
				const StringRef fragment = *formatFragments;

				if ((idx % 2) == 0)
					result << fragment.str();
				else if (fragment.empty())
					result << "%";
				else
				{
					const size_t pos = fragment.find('$');
					const size_t index = FromString<size_t>(fragment.substr(0, pos));
					STINGRAYKIT_CHECK(index > 0, "Format mismatch: parameters indices start from 1!");
					const size_t width = (pos == std::string::npos) ? 0 : FromString<size_t>(fragment.substr(pos + 1));

					TupleToStringHelper<TupleParams>::ItemToString(result, params, index - 1, width);
				}
			}
		}

	}


	template < typename... Ts >
	std::string StringFormat(const std::string& format, const Ts&... args)
	{
		string_ostream ss;
		typedef typename TypeListTransform<TypeList<Ts...>, AddConstLvalueReference>::ValueT TupleParams;
		Detail::StringFormatImpl(ss, Split(format, "%"), Tuple<TupleParams>(args...));
		return ss.str();
	}

}

#endif
