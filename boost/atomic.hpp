#ifndef __BOOST_ATOMIC_HPP
#define __BOOST_ATOMIC_HPP

#include <boost/atomic/memory_order.hpp>
#include <boost/atomic/platform.hpp>
#include <boost/atomic/detail/base.hpp>
#include <boost/atomic/detail/generic-atomic.hpp>

namespace boost {

template<typename T>
class atomic : public detail::atomic::__atomic<T> {
public:
	typedef detail::atomic::__atomic<T> super;
	
	atomic() {}
	explicit atomic(T v) : super(v) {}
private:
	atomic(const atomic &);
	void operator=(const atomic &);
};


template<>
class atomic<bool> : private detail::atomic::__atomic<bool> {
public:
	typedef detail::atomic::__atomic<bool> super;
	
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
class atomic<void *> : private detail::atomic::__atomic<void *, sizeof(void *), int> {
public:
	typedef detail::atomic::__atomic<void *, sizeof(void *), int> super;
	
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

/* FIXME: pointer arithmetic still missing */
/* FIXME: specialize void * pointer separately to forbid arithmetic */

template<typename T>
class atomic<T *> : private detail::atomic::__atomic<T *, sizeof(T *), int> {
public:
	typedef detail::atomic::__atomic<T *, sizeof(T *), int> super;
	
	atomic() {}
	explicit atomic(T * p) : super(p) {}
	using super::load;
	using super::store;
	using super::compare_exchange_strong;
	using super::compare_exchange_weak;
	using super::exchange;
	using super::is_lock_free;
	
	operator T *(void) const volatile {return load();}
	T * operator=(T * v) volatile {store(v); return v;}
	
private:
	atomic(const atomic &);
	void * operator=(const atomic &);
};

class atomic_flag : private atomic<int> {
public:
	typedef atomic<int> super;
	using super::is_lock_free;
	
	atomic_flag(bool initial_state) : super(initial_state?1:0) {}
	atomic_flag() {}
	
	bool test_and_set(memory_order order=memory_order_seq_cst)
	{
		return super::exchange(1, order);
	}
	void clear(memory_order order=memory_order_seq_cst)
	{
		super::store(0, order);
	}
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
