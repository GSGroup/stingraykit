#ifndef STINGRAY_TOOLKIT_STATIC_VISITOR_H
#define STINGRAY_TOOLKIT_STATIC_VISITOR_H

namespace stingray
{
	template<typename ResultType = void>
	struct static_visitor
	{
		typedef ResultType RetType;
	};
}

#endif
