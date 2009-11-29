#ifndef BOOST_DETAIL_ATOMIC_BASE_HPP
#define BOOST_DETAIL_ATOMIC_BASE_HPP

#include <boost/atomic/detail/fallback.hpp>
#include <boost/atomic/detail/builder.hpp>

namespace boost {
namespace detail {
namespace atomic {

template<typename T, unsigned short Size=sizeof(T)>
class __platform_atomic : public __build_atomic_from_exchange<__fallback_atomic<T> > {
public:
	typedef __build_atomic_from_exchange<__fallback_atomic<T> > super;
	
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic() {}
protected:
	typedef typename super::integral_type integral_type;
};

}
}
}

#endif
