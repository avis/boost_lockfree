#ifndef BOOST_DETAIL_ATOMIC_GCC_X86_HPP
#define BOOST_DETAIL_ATOMIC_GCC_X86_HPP

#include <boost/atomic/detail/builder.hpp>
#include <boost/atomic/detail/fallback.hpp>

namespace boost {
namespace detail {
namespace atomic {

static inline void __fence_before(memory_order order)
{
	switch(order) {
		case memory_order_consume:
		case memory_order_release:
		case memory_order_acq_rel:
		case memory_order_seq_cst:
			__asm__ __volatile__ ("" ::: "memory");
		default:;
	}
}

static inline void __fence_after(memory_order order)
{
	switch(order) {
		case memory_order_acquire:
		case memory_order_acq_rel:
		case memory_order_seq_cst:
			__asm__ __volatile__ ("" ::: "memory");
		default:;
	}
}

template<typename T>
class __atomic_x86_base {
public:
	explicit __atomic_x86_base(T v) : i(v) {}
	__atomic_x86_base() {}
	T load(memory_order order=memory_order_seq_cst) const volatile
	{
		T v=*reinterpret_cast<volatile const T *>(&i);
		__fence_after(order);
		return v;
	}
	void store(T v, memory_order order=memory_order_seq_cst) volatile
	{
		__fence_before(order);
		*reinterpret_cast<volatile T *>(&i)=v;
	}
	
protected:
	typedef T IntegralType;
private:
	T i;
};

template<typename T>
class __atomic_x86 {
public:
	explicit __atomic_x86(T v) : i(v) {}
	__atomic_x86() {}
	T load(memory_order order=memory_order_seq_cst) const volatile
	{
		T v=*reinterpret_cast<volatile const T *>(&i);
		__fence_after(order);
		return v;
	}
	void store(T v, memory_order order=memory_order_seq_cst) volatile
	{
		__fence_before(order);
		*reinterpret_cast<volatile T *>(&i)=v;
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		T prev=expected;
		switch(sizeof(T)) {
			case 1:
				__asm__ __volatile__(
					"lock cmpxchgb %1, %2\n"
					: "=a" (prev)
					: "q" (desired), "m" (i), "a" (expected)
					: "memory"
				);
				break;
			case 2:
				__asm__ __volatile__(
					"lock cmpxchgw %1, %2\n"
					: "=a" (prev)
					: "q" (desired), "m" (i), "a" (expected)
					: "memory"
				);
				break;
			case 4:
				__asm__ __volatile__(
					"lock cmpxchgl %1, %2\n"
					: "=a" (prev)
					: "q" (desired), "m" (i), "a" (expected)
					: "memory"
				);
				break;
#if defined(__amd64__)
			case 8:
				__asm__ __volatile__(
					"lock cmpxchgq %1, %2\n"
					: "=a" (prev)
					: "q" (desired), "m" (i), "a" (expected)
					: "memory"
				);
				break;
#endif	
		}
		bool success=(prev==expected);
		expected=prev;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		switch(sizeof(T)) {
			case 1:
				__asm__ __volatile__(
					"lock xchgb %0, %1\n"
					: "=r" (replacement)
					: "m"(i), "0" (replacement)
					: "memory"
				);
				break;
			case 2:
				__asm__ __volatile__(
					"lock xchgw %0, %1\n"
					: "=r" (replacement)
					: "m"(i), "0" (replacement)
					: "memory"
				);
				break;
			case 4:
				__asm__ __volatile__(
					"lock xchgl %0, %1\n"
					: "=r" (replacement)
					: "m"(i), "0" (replacement)
					: "memory"
				);
				break;
#if defined(__amd64__)
			case 8:
				__asm__ __volatile__(
					"lock xchgq %0, %1\n"
					: "=r" (replacement)
					: "m"(i), "0" (replacement)
					: "memory"
				);
				break;
#endif	
		}
		return replacement;
	}
	T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
	{
		switch(sizeof(T)) {
			case 1:
				__asm__ __volatile__("lock xaddb %0, %1" : "+r" (c), "+m" (i) :: "memory");
				break;
			case 2:
				__asm__ __volatile__("lock xaddw %0, %1" : "+r" (c), "+m" (i) :: "memory");
				break;
			case 4:
				__asm__ __volatile__("lock xaddl %0, %1" : "+r" (c), "+m" (i) :: "memory");
				break;
#if defined(__amd64__)
			case 8:
				__asm__ __volatile__("lock xaddq %0, %1" : "+r" (c), "+m" (i) :: "memory");
				break;
#endif	
		}
		return c;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	typedef T IntegralType;
private:
	T i;
};

typedef detail::atomic::__build_atomic_from_add<__atomic_x86<char> > __atomic_char;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<signed char> > __atomic_schar;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<unsigned char> > __atomic_uchar;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<short> > __atomic_short;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<unsigned short> > __atomic_ushort;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<int> > __atomic_int;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<unsigned int> > __atomic_uint;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<long> > __atomic_long;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<unsigned long> > __atomic_ulong;

#if defined(__amd64__)
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<long long> > __atomic_llong;
typedef detail::atomic::__build_atomic_from_add<__atomic_x86<unsigned long long> > __atomic_ullong;
#else
typedef detail::atomic::__build_atomic_from_add<__atomic_fallback_integral<long long> > __atomic_llong;
typedef detail::atomic::__build_atomic_from_add<__atomic_fallback_integral<unsigned long long> > __atomic_ullong;
#endif

//typedef detail::atomic::__build_atomic_ptr_from_typical<x86_ptr_base> __atomic_address;
typedef detail::atomic::__build_atomic_ptr_from_typical<__atomic_x86<void *> > __atomic_address;

}
}
}

#endif
