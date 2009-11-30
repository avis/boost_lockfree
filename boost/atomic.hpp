#ifndef __BOOST_ATOMIC_HPP
#define __BOOST_ATOMIC_HPP

#include <boost/atomic/memory_order.hpp>
#include <boost/atomic/platform.hpp>
#include <boost/atomic/detail/valid_integral_types.hpp>

namespace boost {

template<typename T>
class atomic : private detail::atomic::__platform_atomic<T> {
public:
	typedef detail::atomic::__platform_atomic<T> super;
	typedef typename super::integral_type integral_type;
	
	atomic() {detail::atomic::valid_atomic_type<T> verify_valid_atomic_integral;}
	explicit atomic(T v) : super(v) {detail::atomic::valid_atomic_type<T> verify_valid_atomic_integral;}
	
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
	using super::is_lock_free;
	
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
	atomic(const atomic &);
	void operator=(const atomic &);
};

template<>
class atomic<bool> : private detail::atomic::__platform_atomic<bool> {
public:
	typedef detail::atomic::__platform_atomic<bool> super;
	
	atomic() {}
	explicit atomic(bool v) : super(v) {}
	
	using super::load;
	using super::store;
	using super::compare_exchange_strong;
	using super::compare_exchange_weak;
	using super::exchange;
	using super::is_lock_free;
	
	operator bool(void) const volatile {return load();}
	bool operator=(bool v) volatile {store(v); return v;}	
private:
	atomic(const atomic &);
	void operator=(const atomic &);
};

template<>
class atomic<void *> : private detail::atomic::__platform_atomic_address {
public:
	typedef detail::atomic::__platform_atomic_address super;
	
	atomic() {}
	explicit atomic(void * p) : super(p) {}
	using super::load;
	using super::store;
	using super::compare_exchange_strong;
	using super::compare_exchange_weak;
	using super::exchange;
	using super::is_lock_free;
	
	operator void *(void) const volatile {return load();}
	void * operator=(void * v) volatile {store(v); return v;}
	
private:
	atomic(const atomic &);
	void * operator=(const atomic &);
};

template<typename T>
class atomic<T *> : private atomic<void *> {
public:
	typedef atomic<void *> super;
	
	using super::is_lock_free;
	
	atomic(void) {}
	explicit atomic(T * v) : super(v) {}
	
	T *load(memory_order order=memory_order_seq_cst) const volatile
	{
		return static_cast<T *>(super::load(order));
	}
	void store(T * v, memory_order order=memory_order_seq_cst) volatile
	{
		super::store(v, order);
	}
	bool compare_exchange_weak(T *&expected, T *desired, memory_order order=memory_order_seq_cst) volatile
	{
		void **expected_void = reinterpret_cast<void **>(&expected);
		return super::compare_exchange_weak(*expected_void, static_cast<void *>(desired), order);
	}
	bool compare_exchange_strong(T *&expected, T *desired, memory_order order=memory_order_seq_cst) volatile
	{
		void **expected_void = reinterpret_cast<void **>(&expected);
		return super::compare_exchange_strong(*expected_void, static_cast<void *>(desired), order);
	}
	T *exchange(T *replacement, memory_order order=memory_order_seq_cst) volatile
	{
		return static_cast<T *>(super::exchange(static_cast<void *>(replacement)));
	}
	
	operator T *(void) const {return load();}
	T *operator=(T *v) {store(v); return v;}
private:
	atomic(const atomic &);
	void operator=(const atomic &);
};

typedef atomic<char> atomic_char;
typedef atomic<unsigned char> atomic_uchar;
typedef atomic<signed char> atomic_schar;
typedef atomic<uint8_t> atomic_uint8_t;
typedef atomic<int8_t> atomic_int8_t;
typedef atomic<unsigned short> atomic_ushort;
typedef atomic<short> atomic_short;
typedef atomic<uint16_t> atomic_uint16_t;
typedef atomic<int16_t> atomic_int16_t;
typedef atomic<unsigned int> atomic_uint;
typedef atomic<int> atomic_int;
typedef atomic<uint32_t> atomic_uint32_t;
typedef atomic<int32_t> atomic_int32_t;
typedef atomic<unsigned long> atomic_ulong;
typedef atomic<long> atomic_long;
typedef atomic<uint64_t> atomic_uint64_t;
typedef atomic<int64_t> atomic_int64_t;
typedef atomic<unsigned long long> atomic_ullong;
typedef atomic<long long> atomic_llong;
typedef atomic<void*> atomic_address;
typedef atomic<bool> atomic_bool;

}

#endif
