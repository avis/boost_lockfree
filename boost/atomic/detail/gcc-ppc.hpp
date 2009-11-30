#ifndef BOOST_DETAIL_ATOMIC_GCC_PPC_HPP
#define BOOST_DETAIL_ATOMIC_GCC_PPC_HPP

#include <boost/atomic/memory_order.hpp>
#include <boost/atomic/detail/base.hpp>
#include <boost/atomic/detail/builder.hpp>

/*
  Refer to: Motorola: "Programming Environments Manual for 32-Bit
  Implementations of the PowerPC Architecture", Appendix E:
  "Synchronization Programming Examples" for an explanation of what is
  going on here (can be found on the web at various places by the
  name "MPCFPE32B.pdf", Google is your friend...)
 */

namespace boost {
namespace detail {
namespace atomic {

/* note: the __asm__ constraint "b" instructs gcc to use any register
except r0; this is required because r0 is not allowed in
some places. Since I am sometimes unsure if it is allowed
or not just play it safe and avoid r0 entirely -- ppc isn't
exactly register-starved, so this really should not matter :) */

static inline void __fence_before(memory_order order)
{
	switch(order) {
		case memory_order_release:
		case memory_order_acq_rel:
		case memory_order_seq_cst:
			__asm__ __volatile__ ("sync" ::: "memory");
		default:;
	}
}

static inline void __fence_after(memory_order order)
{
	switch(order) {
		case memory_order_acquire:
		case memory_order_acq_rel:
		case memory_order_seq_cst:
			__asm__ __volatile__ ("isync" ::: "memory");
		case memory_order_consume:
			__asm__ __volatile__ ("" ::: "memory");
		default:;
	}
}

template<typename T>
class __atomic_ppc_4 {
public:
	typedef T integral_type;
	explicit __atomic_ppc_4(T v) : i(v) {}
	__atomic_ppc_4() {}
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
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		__fence_before(order);
		int success;
		__asm__ __volatile__(
			"addi %1,0,0\n"
			"lwarx %0,0,%2\n"
			"cmpw %0, %3\n"
			"bne- 1f\n"
			"stwcx. %4,0,%2\n"
			"bne- 1f\n"
			"addi %1,0,1\n"
			"1:"
				: "=&b" (expected), "=&b" (success)
				: "b" (&i), "b" (expected), "b" ((int)desired)
			);
		__fence_after(order);
		return success;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	inline T fetch_add_var(T c, memory_order order) volatile
	{
		__fence_before(order);
		T original, tmp;
		__asm__ __volatile__(
			"1: lwarx %0,0,%2\n"
			"add %1,%0,%3\n"
			"stwcx. %1,0,%2\n"
			"bne- 1b\n"
			: "=&b" (original), "=&b" (tmp)
			: "b" (&i), "b" (c)
			: "cc");
		__fence_after(order);
		return original;
	}
	inline T fetch_inc(memory_order order) volatile
	{
		__fence_before(order);
		T original, tmp;
		__asm__ __volatile__(
			"1: lwarx %0,0,%2\n"
			"addi %1,%0,1\n"
			"stwcx. %1,0,%2\n"
			"bne- 1b\n"
			: "=&b" (original), "=&b" (tmp)
			: "b" (&i)
			: "cc");
		__fence_after(order);
		return original;
	}
	inline T fetch_dec(memory_order order) volatile
	{
		__fence_before(order);
		T original, tmp;
		__asm__ __volatile__(
			"1: lwarx %0,0,%2\n"
			"addi %1,%0,-1\n"
			"stwcx. %1,0,%2\n"
			"bne- 1b\n"
			: "=&b" (original), "=&b" (tmp)
			: "b" (&i)
			: "cc");
		__fence_after(order);
		return original;
	}
private:
	T i;
};

#if defined(__powerpc64__)

#warning Untested code -- please inform me if it works

template<typename T>
class __atomic_ppc_8 {
public:
	typedef T integral_type;
	explicit __atomic_ppc_8(T v) : i(v) {}
	__atomic_ppc_8() {}
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
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		__fence_before(order);
		int success;
		__asm__ __volatile__(
			"addi %1,0,0\n"
			"ldarx %0,0,%2\n"
			"cmpw %0, %3\n"
			"bne- 1f\n"
			"stdcx. %4,0,%2\n"
			"bne- 1f\n"
			"addi %1,0,1\n"
			"1:"
				: "=&b" (expected), "=&b" (success)
				: "b" (&i), "b" (expected), "b" ((int)desired)
			);
		__fence_after(order);
		return success;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	inline T fetch_add_var(T c, memory_order order) volatile
	{
		__fence_before(order);
		T original, tmp;
		__asm__ __volatile__(
			"1: ldarx %0,0,%2\n"
			"add %1,%0,%3\n"
			"stdcx. %1,0,%2\n"
			"bne- 1b\n"
			: "=&b" (original), "=&b" (tmp)
			: "b" (&i), "b" (c)
			: "cc");
		__fence_after(order);
		return original;
	}
	inline T fetch_inc(memory_order order) volatile
	{
		__fence_before(order);
		T original, tmp;
		__asm__ __volatile__(
			"1: ldarx %0,0,%2\n"
			"addi %1,%0,1\n"
			"stdcx. %1,0,%2\n"
			"bne- 1b\n"
			: "=&b" (original), "=&b" (tmp)
			: "b" (&i)
			: "cc");
		__fence_after(order);
		return original;
	}
	inline T fetch_dec(memory_order order) volatile
	{
		__fence_before(order);
		T original, tmp;
		__asm__ __volatile__(
			"1: ldarx %0,0,%2\n"
			"addi %1,%0,-1\n"
			"stdcx. %1,0,%2\n"
			"bne- 1b\n"
			: "=&b" (original), "=&b" (tmp)
			: "b" (&i)
			: "cc");
		__fence_after(order);
		return original;
	}
private:
	T i;
};
#endif

template<typename T>
class __platform_atomic<T, 4> : public __build_atomic_from_typical<__build_exchange<__atomic_ppc_4<T> > > {
public:
	typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc_4<T> > > super;
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic(void) {}
};

template<typename T>
class __platform_atomic<T, 1>: public __build_atomic_from_larger_type<__atomic_ppc_4<uint32_t>, T> {
public:
	typedef __build_atomic_from_larger_type<__atomic_ppc_4<uint32_t>, T> super;
	
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic(void) {}
};

template<typename T>
class __platform_atomic<T, 2>: public __build_atomic_from_larger_type<__atomic_ppc_4<uint32_t>, T> {
public:
	typedef __build_atomic_from_larger_type<__atomic_ppc_4<uint32_t>, T> super;
	
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic(void) {}
};

#if defined(__powerpc64__)
template<typename T>
class __platform_atomic<T, 8> : public __build_atomic_from_typical<__build_exchange<__atomic_ppc_8<T> > > {
public:
	typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc_8<T> > > super;
	explicit __platform_atomic(T v) : super(v) {}
	__platform_atomic(void) {}
};

typedef __build_exchange<__atomic_ppc_8<void *> > __platform_atomic_address;
#else
typedef __build_exchange<__atomic_ppc_4<void *> > __platform_atomic_address;
#endif

}
}
}

#endif
