// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/BacktraceUtility.h>

#include <stingraykit/string/Hex.h>
#include <stingraykit/string/RegexUtils.h>
#include <stingraykit/string/StringReader.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/Holder.h>
#include <stingraykit/SystemException.h>

#include <stdio.h>
#include <unistd.h>

namespace stingray
{

	namespace
	{

		const std::regex DynamicLibraryInfoRegex(R"(([0-9a-f]{8})-([0-9a-f]{8})\sr\-xp\s[0-9a-f]{8}\s[0-9a-f]{2}:[0-9a-f]{2}\s[\d]{8}\s+\/.*?([^\/]+\.so.*?))");

		std::string ReadFileToString(const std::string& name)
		{
			ScopedHolder<FILE*> holder(&fclose);

			FILE* file = fopen(name.c_str(), "rb");
			STINGRAYKIT_CHECK(file, SystemException(StringBuilder() % "fopen(" % name % ")"));

			holder.Set(file);

			std::string buffer;
			size_t offset = 0;
			size_t readSize = 0;
			while (!feof(file))
			{
				if (offset == buffer.size())
					buffer.resize(buffer.size() + 65536);

				readSize = fread(&buffer[offset], 1, buffer.size() - offset, file);
				STINGRAYKIT_CHECK(!ferror(file), InputOutputException(StringBuilder() % "Error occurred while reading file '" % name % "'"));

				offset += readSize;
			}

			buffer.resize(offset);

			return buffer;
		}

	}


	DynamicLibraryInfo::DynamicLibraryInfo(uintptr_t addressBegin, uintptr_t addressEnd, const std::string& libraryPath)
		:	AddressRange(addressBegin, addressEnd),
			LibraryPath(libraryPath)
	{ }


	std::string DynamicLibraryInfo::ToString() const
	{ return StringBuilder() % Hex(AddressRange.GetStart()) % " " % Hex(AddressRange.GetEnd()) % " " % LibraryPath; }


	std::vector<DynamicLibraryInfo> BacktraceUtility::GetDynamicLibraries()
	{
		std::vector<DynamicLibraryInfo> info;
		const std::string data = ReadFileToString(StringBuilder() % "/proc/" % getpid() % "/maps");

		if (data.empty())
			return info;

		StringReader reader(data);
		while (!reader.IsEndOfText())
		{
			const string_view line = reader.ReadLine();
			svmatch match;
			if (std::regex_match(line.begin(), line.end(), match, DynamicLibraryInfoRegex))
				info.emplace_back(FromHex<uintptr_t>(svmatch_str(match, 1)), FromHex<uintptr_t>(svmatch_str(match, 2)), match.str(3));
		}

		return info;
	}

}
