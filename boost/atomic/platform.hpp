#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))

	#include <boost/atomic/detail/gcc-x86.hpp>

#elif defined(__GNUC__) && defined(__alpha__)

	#include <boost/atomic/detail/gcc-alpha.hpp>

#elif defined(__GNUC__) && (defined(__POWERPC__) || defined(__PPC__))

	#include <boost/atomic/detail/gcc-ppc.hpp>

#else
	
	#include <boost/atomic/detail/generic-cas.hpp>

#endif
