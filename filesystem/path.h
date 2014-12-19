#ifndef STINGRAY_TOOLKIT_FILESYSTEM_PATH_H
#define STINGRAY_TOOLKIT_FILESYSTEM_PATH_H


#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	/**
	 * @addtogroup core_storage
	 * @{
	 */

	class path
	{
	private:
		std::string		_pathString;

	public:
		path(): _pathString() {}
		path(const char *pathString): _pathString(pathString) {}
		path(const std::string& pathString): _pathString(pathString) {}

		path operator / (const path& tail) const;

		bool operator < (const path& other) const
		{ return _pathString < other._pathString;  }

		const std::string& string() const { return _pathString; }

		std::string filename() const;
		std::string stem() const;
		std::string extension() const;

		path dirpath() const;
		bool contains(const path& other) const;
		path relative_to(const path& other) const;
		path normalize() const;
		int path_depth() const;
		bool empty() const;

		bool is_absolute() const;

		template<typename Archive>
		void Serialize(Archive& ar) const { ar.Serialize("path", _pathString); }

		template<typename Archive>
		void Deserialize(Archive& ar) { ar.Deserialize("path", _pathString); }

		static path FromString(const std::string& str) { return path(str); }
		std::string ToString() const { return this->string(); }

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(path);
	};

	/** @} */

}


#endif
