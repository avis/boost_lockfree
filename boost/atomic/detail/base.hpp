#ifndef BOOST_DETAIL_ATOMIC_BASE_HPP
#define BOOST_DETAIL_ATOMIC_BASE_HPP

#include <boost/atomic/detail/fallback.hpp>
#include <boost/atomic/detail/builder.hpp>
#include <boost/atomic/detail/valid_integral_types.hpp>

namespace boost {
namespace detail {
namespace atomic {

template<typename T, unsigned short Size=sizeof(T)>
class __platform_atomic : public __fallback_atomic<T> {
public:
	typedef __fallback_atomic<T> super;
	
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic() {}
protected:
	typedef typename super::integral_type integral_type;
};

template<typename T, unsigned short Size=sizeof(T)>
class __platform_atomic_integral : public build_atomic_from_exchange<__fallback_atomic<T> > {
public:
	typedef build_atomic_from_exchange<__fallback_atomic<T> > super;
	
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral() {}
protected:
	typedef typename super::integral_type integral_type;
};

/**/

template<typename T, unsigned short Size=sizeof(T), typename Int=typename is_integral_type<T>::test>
class __atomic;

template<typename T, unsigned short Size>
class __atomic<T, Size, void> : private detail::atomic::__platform_atomic<T> {
public:
	typedef detail::atomic::__platform_atomic<T> super;
	
	__atomic() {}
	explicit __atomic(T v) : super(v) {}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
	using super::load;
	using super::store;
	using super::compare_exchange_strong;
	using super::compare_exchange_weak;
	using super::exchange;
	
private:
	__atomic(const __atomic &);
	void operator=(const __atomic &);
};

template<typename T, unsigned short Size>
class __atomic<T, Size, int> : private detail::atomic::__platform_atomic_integral<T> {
public:
	typedef detail::atomic::__platform_atomic_integral<T> super;
	typedef typename super::integral_type integral_type;
	
	__atomic() {}
	explicit __atomic(T v) : super(v) {}
	
	using super::is_lock_free;
	using super::load;
	using super::store;
	using super::compare_exchange_strong;
	using super::compare_exchange_weak;
	using super::exchange;
	using super::fetch_add;
	using super::fetch_sub;
	using super::fetch_and;
	using super::fetch_or;
	using super::fetch_xor;
	
	operator integral_type(void) const volatile {return load();}
	integral_type operator=(integral_type v) volatile {store(v); return v;}	
	
	integral_type operator&=(integral_type c) volatile {return fetch_and(c)&c;}
	integral_type operator|=(integral_type c) volatile {return fetch_or(c)|c;}
	integral_type operator^=(integral_type c) volatile {return fetch_xor(c)^c;}
	
	integral_type operator+=(integral_type c) volatile {return fetch_add(c)+c;}
	integral_type operator-=(integral_type c) volatile {return fetch_sub(c)-c;}
	
	integral_type operator++(void) volatile {return fetch_add(1)+1;}
	integral_type operator++(int) volatile {return fetch_add(1);}
	integral_type operator--(void) volatile {return fetch_sub(1)-1;}
	integral_type operator--(int) volatile {return fetch_sub(1);}
	
private:
	__atomic(const __atomic &);
	void operator=(const __atomic &);
};


}
}
}

#endif
