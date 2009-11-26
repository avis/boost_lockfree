#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))

	#include <boost/atomic/detail/gcc-x86.hpp>

#elif defined(__GNUC__) && defined(__alpha__)

	#include <boost/atomic/detail/gcc-alpha.hpp>

#elif defined(__GNUC__) && (defined(__POWERPC__) || defined(__PPC__))

	#include <boost/atomic/detail/gcc-ppc.hpp>

/*
#elif defined(__GNUC__) && (defined(__sparc__))

	#include <boost/atomic/detail/gcc-sparc.hpp>

#elif defined(__GNUC__) && defined(__ia64__) && !defined(USE_FALLBACK_ATOMICS)

	#include <boost/atomic/detail/gcc-ia64.hpp>
*/

#else
	
	#error Atomic implementation missing for this platform

#endif
