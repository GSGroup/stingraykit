#ifndef STINGRAYKIT_SYMBOLVISIBILITY_H
#define STINGRAYKIT_SYMBOLVISIBILITY_H

#ifdef STINGRAYKIT_USES_GCC_VISIBILITY
#	define STINGRAYKIT_SYMBOL_EXPORT __attribute__ ((visibility ("default")))
#	define STINGRAYKIT_SYMBOL_IMPORT __attribute__ ((visibility ("default")))
#	define STINGRAYKIT_SYMBOL_LOCAL __attribute__ ((visibility ("hidden")))
#else
#	define STINGRAYKIT_SYMBOL_EXPORT
#	define STINGRAYKIT_SYMBOL_IMPORT
#	define STINGRAYKIT_SYMBOL_LOCAL
#endif

#endif
