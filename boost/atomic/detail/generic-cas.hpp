#ifndef BOOST_DETAIL_ATOMIC_GENERIC_CAS_HPP
#define BOOST_DETAIL_ATOMIC_GENERIC_CAS_HPP

#include <stdint.h>

#include <boost/atomic/memory_order.hpp>
#include <boost/atomic/detail/base.hpp>
#include <boost/atomic/detail/builder.hpp>

/* fallback implementation for various compilation targets;
this is *not* efficient, particularly because all operations
are fully fenced (full memory barriers before and after
each operation) */

#if defined(__GNUC__)
	namespace boost { namespace detail { namespace atomic {
	static inline int32_t
	fenced_compare_exchange_strong_32(volatile int32_t *ptr, int32_t expected, int32_t desired)
	{
		return __sync_val_compare_and_swap_4(ptr, expected, desired);
	}
	#define BOOST_ATOMIC_HAVE_CAS32 1
	
	#if defined(__amd64__) || defined(__i686__)
	static inline int64_t
	fenced_compare_exchange_strong_64(int64_t *ptr, int64_t expected, int64_t desired)
	{
		return __sync_val_compare_and_swap_8(ptr, expected, desired);
	}
	#define BOOST_ATOMIC_HAVE_CAS64 1
	#endif
	}}}

#elif defined(__ICL) || defined(_MSC_VER)

	#if defined(_MSC_VER)
	#include <Windows.h>
	#include <intrin.h>
	#endif

	namespace boost { namespace detail { namespace atomic {
	static inline int32_t
	fenced_compare_exchange_strong(int32_t *ptr, int32_t expected, int32_t desired)
	{
		return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(ptr), desired, expected);
	}
	#define BOOST_ATOMIC_HAVE_CAS32 1
	#if defined(_WIN64)
	static inline int64_t
	fenced_compare_exchange_strong(int64_t *ptr, int64_t expected, int64_t desired)
	{
		return _InterlockedCompareExchange64(ptr, desired, expected);
	}
	#define BOOST_ATOMIC_HAVE_CAS64 1
	#endif
	}}}
	
#elif (defined(__ICC) || defined(__ECC))
	namespace boost { namespace detail { namespace atomic {
	static inline int32_t
	fenced_compare_exchange_strong_32(int32_t *ptr, int32_t expected, int32_t desired)
	{
		return _InterlockedCompareExchange((void*)ptr, desired, expected);
	}
	#define BOOST_ATOMIC_HAVE_CAS32 1
	#if defined(__x86_64)
	static inline int64_t
	fenced_compare_exchange_strong(int64_t *ptr, int64_t expected, int64_t desired)
	{
		return cas64<int>(ptr, expected, desired);
	}
	#define BOOST_ATOMIC_HAVE_CAS64 1
	#elif defined(__ECC)	//IA-64 version
	static inline int64_t
	fenced_compare_exchange_strong(int64_t *ptr, int64_t expected, int64_t desired)
	{
		return _InterlockedCompareExchange64((void*)ptr, desired, expected);
	}
	#define BOOST_ATOMIC_HAVE_CAS64 1
	#endif
	}}}
	
#elif (defined(__SUNPRO_CC) && defined(__sparc))
	#include <sys/atomic.h>
	namespace boost { namespace detail { namespace atomic {
	static inline int32_t
	fenced_compare_exchange_strong_32(int32_t *ptr, int32_t expected, int32_t desired)
	{
		return atomic_cas_32((volatile unsigned int*)ptr, expected, desired);
	}
	#define BOOST_ATOMIC_HAVE_CAS32 1
	
	/* FIXME: check for 64 bit mode */
	static inline int64_t
	fenced_compare_exchange_strong_64(int64_t *ptr, int64_t expected, int64_t desired)
	{
		return atomic_cas_64((volatile unsigned long long*)ptr, expected, desired);
	}
	#define BOOST_ATOMIC_HAVE_CAS64 1
	}}}
#endif


namespace boost { namespace detail { namespace atomic {

#ifdef BOOST_ATOMIC_HAVE_CAS32
template<typename T>
class __atomic_generic_cas_4 {
private:
	typedef __atomic_generic_cas_4 this_type;
public:
	explicit __atomic_generic_cas_4(T v) : i((int32_t)v) {}
	__atomic_generic_cas_4() {}
	T load(memory_order order=memory_order_seq_cst) const volatile
	{
		T expected=(T)i;
		do { } while(!const_cast<this_type *>(this)->compare_exchange_weak(expected, expected, order));
		return expected;
	}
	void store(T v, memory_order order=memory_order_seq_cst) volatile
	{
		exchange(v);
	}
	bool compare_exchange_strong(T &e, T d, memory_order order=memory_order_seq_cst) volatile
	{
		T found;
		found=(T)fenced_compare_exchange_strong_32(&i, (int32_t)e, (int32_t)d);
		bool success=(found==e);
		e=found;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T r, memory_order order=memory_order_seq_cst) volatile
	{
		T expected=(T)i;
		do { } while(!compare_exchange_weak(expected, r, order));
		return expected;
	}
	
	bool is_lock_free(void) const volatile {return true;}
	typedef T integral_type;
private:
	mutable int32_t i;
};

template<typename T>
class __platform_atomic<T, 4> : public __build_atomic_from_exchange<__atomic_generic_cas_4<T> > {
public:
	typedef __build_atomic_from_exchange<__atomic_generic_cas_4<T> > super;
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic(void) {}
};

template<typename T>
class __platform_atomic<T, 1>: public __build_atomic_from_larger_type<__atomic_generic_cas_4<int32_t>, T> {
public:
	typedef __build_atomic_from_larger_type<__atomic_generic_cas_4<int32_t>, T> super;
	
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic(void) {}
};

template<typename T>
class __platform_atomic<T, 2>: public __build_atomic_from_larger_type<__atomic_generic_cas_4<int32_t>, T> {
public:
	typedef __build_atomic_from_larger_type<__atomic_generic_cas_4<int32_t>, T> super;
	
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic(void) {}
};
#endif

typedef __platform_atomic<void *, sizeof(void *)> __platform_atomic_address;

} } }

#endif
