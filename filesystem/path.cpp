#include <stingray/toolkit/filesystem/path.h>

#include <stingray/filesystem/FileSystemProvider.h>


namespace stingray
{
	path path::operator / (const path& tail) const
	{
		if (_pathString.empty())
			return tail;
		if (tail._pathString.empty() || tail._pathString == ".")
			return *this;

		return _pathString != "/" ?
			path(_pathString + "/" + tail._pathString):
			path(_pathString + tail._pathString);
	}

	std::string path::filename() const
	{
		const std::string::size_type pos = _pathString.rfind('/');
		return pos == std::string::npos? _pathString : _pathString.substr(pos + 1);
	}


	std::string path::stem() const
	{
		const std::string name = filename();
		if (name == "." || name == "..")
			return name;

		const std::string::size_type pos = name.rfind('.');
		return pos == std::string::npos? name : name.substr(0, pos);
	}


	std::string path::extension() const
	{
		const std::string name = filename();
		if (name == "." || name == "..")
			return "";

		const std::string::size_type pos = name.rfind('.');
		return pos == std::string::npos? "" : name.substr(pos);
	}


	path path::dirpath() const
	{
		const std::string::size_type pos = _pathString.rfind('/');
		return pos == std::string::npos ? "" : _pathString.substr(0, pos);
	}



	bool path::contains(const path& other) const
	{
		path this_path = normalize();
		path other_path = other.normalize();

		return this_path._pathString.size() <= other_path._pathString.size() &&
			std::equal(this_path._pathString.begin(), this_path._pathString.end(), other_path._pathString.begin());
	}


	path path::relative_to(const path& other) const
	{
		if (!other.contains(*this))
			TOOLKIT_THROW(std::runtime_error("The other path does not contain this one!"));

		path this_path = normalize();
		path other_path = other.normalize();

		path result;
		if (this_path._pathString.size() != other_path._pathString.size())
			result = this_path._pathString.substr(other_path._pathString.size() + 1);
		return result._pathString.empty() ? "." : result;
	}


	path path::normalize() const
	{
		std::string str = _pathString.empty()?
			FileSystemProvider::Get().GetCurrentDirectory()->GetPath().string():
			(
				_pathString[0] != '/' ?
				FileSystemProvider::Get().GetCurrentDirectory()->GetPath().string() + "/" + _pathString:
				_pathString
			);

		TOOLKIT_CHECK(!str.empty() && str[0] == '/', "GetCurrentDirectory() returned non-absolute path");

		size_t dst = 0, n = str.size();
		for(size_t src = 0; src < n; ++src)
		{
			if (str[src] == '/')
			{
				if (src + 1 < n && str[src + 1] == '.' && (src + 2 >= n || str[src + 2] == '/'))
				{
					++src;
					continue;
				}
				if (src + 2 < n && str[src + 1] == '.' && str[src + 2] == '.' && (src + 3 >= n || str[src + 3] == '/'))
				{
					src += 2;
					size_t pos = str.rfind('/', dst - 1);
					TOOLKIT_CHECK(pos != str.npos, "internal error, missing leading slash");
					dst = pos > 0? pos: 1; //save leading slash
					continue;
				}

				if (dst > 0 && str[dst - 1] == '/') //skipping repeating slashes
					continue;
			}

			if (dst != src)
				str[dst++] = str[src];
			else
				++dst;
		}

		while(dst > 1 && str[dst - 1] == '/')
			--dst;

		str.resize(dst);
		return str;
	}


	int path::path_depth() const
	{
		int result = 0;
		for (std::string::const_iterator i = _pathString.begin(); i != _pathString.end(); ++i)
			if (*i == '/')
				result++;
		return result;
	}


	bool path::empty() const
	{ return _pathString.empty(); }


	bool path::is_absolute() const
	{ return !_pathString.empty() && _pathString[0] == '/'; }

}
