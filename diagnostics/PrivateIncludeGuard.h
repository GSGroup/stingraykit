#ifndef STINGRAYKIT_DIAGNOSTICS_PRIVATEINCLUDEGUARD_H
#define STINGRAYKIT_DIAGNOSTICS_PRIVATEINCLUDEGUARD_H

#if !defined(STINGRAY_LIB_SOURCE)
static void _failer_()
{
	class ErrorClass;
	static ErrorClass ERROR__This_header_is_ought_to_be_private;
}
//#error "This header is ought to be private!"
#endif

#endif

