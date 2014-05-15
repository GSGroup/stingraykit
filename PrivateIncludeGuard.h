#ifndef STINGRAY_TOOLKIT_PRIVATEINCLUDEGUARD_H
#define STINGRAY_TOOLKIT_PRIVATEINCLUDEGUARD_H

#if !defined(STINGRAY_LIB_SOURCE)
static void _failer_()
{
	class ErrorClass;
	static ErrorClass ERROR__This_header_is_ought_to_be_private;
}
//#error "This header is ought to be private!"
#endif

#endif

