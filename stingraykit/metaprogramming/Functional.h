#ifndef STINGRAYKIT_METAPROGRAMMING_FUNCTIONAL_H
#define STINGRAYKIT_METAPROGRAMMING_FUNCTIONAL_H

namespace stingray
{

	template < template <typename> class Predicate >
	struct Not
	{
		template < typename T >
		struct ValueT { static const bool Value = !Predicate<T>::Value; };
	};

	template <template <typename, typename> class Predicate, typename Right>
	struct BindRight
	{
		template <typename Left>
		struct ValueT { static const bool Value = Predicate<Left, Right>::Value; };
	};

}

#endif
