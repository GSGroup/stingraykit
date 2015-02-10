#ifndef STINGRAYKIT_METAPROGRAMMING_YESNO_H
#define STINGRAYKIT_METAPROGRAMMING_YESNO_H

#include <stingraykit/metaprogramming/CompileTimeAssert.h>

namespace stingray
{

	struct YesType { char dummy; };
	struct NoType { YesType dummy[2]; };

	namespace
	{ CompileTimeAssert< sizeof(YesType) != sizeof(NoType) >	ERROR__yes_and_no_types_do_not_work; }

}

#endif
