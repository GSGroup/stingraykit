#ifndef STINGRAYKIT_STATIC_VISITOR_H
#define STINGRAYKIT_STATIC_VISITOR_H

namespace stingray
{
	/**
	 * @addtogroup toolkit_general_variants
	 * @{
	 */

	template<typename ResultType = void>
	struct static_visitor
	{
		typedef ResultType RetType;
	};

	/** @} */
}

#endif
