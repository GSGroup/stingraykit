#ifndef STINGRAY_TOOLKIT_DOUBLE_MAP_H__
#define STINGRAY_TOOLKIT_DOUBLE_MAP_H__

#include <stdexcept>

namespace stingray
{

#define BIMAP_DECLARE_TYPES(T, O) \
public: \
	typedef T##_collection_type_ T##_collection_type; \
private: \
	T##_collection_type _##T; \
public: \
	typedef typename T##_collection_type::key_type			T##_key_type; \
	typedef typename T##_collection_type::mapped_type		T##_mapped_type; \
	typedef typename T##_collection_type::value_type		T##_value_type; \
	typedef typename T##_collection_type::iterator			T##_iterator; \
	typedef typename T##_collection_type::const_iterator	T##_const_iterator; \
	typedef typename T##_collection_type::reverse_iterator	T##_reverse_iterator; \
	typedef typename T##_collection_type::const_reverse_iterator	T##_const_reverse_iterator; \
\
	inline T##_iterator T##_begin()				{ return _##T.begin(); } \
	inline T##_iterator T##_end()				{ return _##T.end(); } \
	inline T##_const_iterator T##_begin() const	{ return _##T.begin(); } \
	inline T##_const_iterator T##_end()	const	{ return _##T.end(); } \
\
	inline T##_reverse_iterator T##_rbegin()				{ return _##T.rbegin(); } \
	inline T##_reverse_iterator T##_rend()					{ return _##T.rend(); } \
	inline T##_const_reverse_iterator T##_rbegin() const	{ return _##T.rbegin(); } \
	inline T##_const_reverse_iterator T##_rend()	const	{ return _##T.rend(); } \
\
	T##_iterator T##_find(const T##_key_type &key)				{ return _##T.find(key); } \
	T##_const_iterator T##_find(const T##_key_type &key) const	{ return _##T.find(key); } \
\
	void T##_insert(const T##_key_type &key, const T##_mapped_type &value) \
	{ \
		T##_iterator i ( _##T.find(key) ); \
		if (i != _##T.end()) throw std::runtime_error(#T " key is not unique"); \
		O##_iterator j ( _##O.find(value) ); \
		if (j != _##O.end()) throw std::runtime_error(#O " key is not unique"); \
		_##T.insert(T##_value_type(key, value)); \
		_##O.insert(O##_value_type(value, key)); \
	} \
\
	void T##_erase(const T##_iterator &i) \
	{ \
		if (i != _##T.end()) \
		{ \
			_##T.erase(i); \
			_##O.erase(i->second); \
		} \
	} \
\
	void T##_erase(const T##_key_type &key) \
	{ \
		T##_iterator i ( _##T.find(key) ); \
		erase(i); \
	}

	template <typename left_collection_type_, typename right_collection_type_>
	class bimap
	{
		BIMAP_DECLARE_TYPES(left, right);
		BIMAP_DECLARE_TYPES(right, left);

		inline bool empty() const { return _left.empty(); }
		inline void clear() { _left.clear(); _right.clear(); }
	};

#undef BIMAP_DECLARE_TYPES
}

#endif
