#ifndef BOOST_DETAIL_ATOMIC_GCC_PPC_HPP
#define BOOST_DETAIL_ATOMIC_GCC_PPC_HPP

#include <boost/atomic/detail/builder.hpp>
#include <boost/atomic/detail/fallback.hpp>

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
			__asm__ __volatile__ ("eieio" ::: "memory");
		default:;
	}
}

static inline void __fence_after(memory_order order)
{
	switch(order) {
		case memory_order_acquire:
		case memory_order_acq_rel:
		case memory_order_seq_cst:
			__asm__ __volatile__ ("eieio" ::: "memory");
		default:;
	}
}

template<typename T>
class __atomic_ppc {
public:
	explicit __atomic_ppc(T v) : i(v) {}
	__atomic_ppc() {}
	T load(memory_order order=memory_order_seq_cst) const volatile
	{
		T v=*reinterpret_cast<volatile const int *>(&i);
		__fence_after(order);
		return v;
	}
	void store(T v, memory_order order=memory_order_seq_cst) volatile
	{
		__fence_before(order);
		*reinterpret_cast<volatile int *>(&i)=(int)v;
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
	inline T fetch_add_var(long c, memory_order order) volatile
	{
		__fence_before(order);
		int original, tmp;
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
		int original, tmp;
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
		int original, tmp;
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
	typedef T IntegralType;
private:
	int i;
};

class __atomic_ppc_address {
public:
	explicit __atomic_ppc_address(void * v) : i(v) {}
	__atomic_ppc_address() {}
	void *load(memory_order order=memory_order_seq_cst) const volatile
	{
		void * v=*reinterpret_cast<void * volatile const *>(&i);
		__fence_after(order);
		return v;
	}
	void store(void *v, memory_order order=memory_order_seq_cst) volatile
	{
		__fence_before(order);
		*reinterpret_cast<void * volatile *>(&i)=v;
	}
	bool compare_exchange_weak(void * &expected, void * desired, memory_order order=memory_order_seq_cst) volatile
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
				: "b" (&i), "b" (expected), "b" (desired)
			);
		__fence_after(order);
		return success;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	typedef void * IntegralType;
private:
	void * i;
};

typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<char> > > __atomic_char;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<unsigned char> > > __atomic_uchar;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<unsigned short> > > __atomic_ushort;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<unsigned int> > > __atomic_uint;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<unsigned long> > > __atomic_ulong;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<signed char> > > __atomic_schar;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<short> > > __atomic_short;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<int> > > __atomic_int;
typedef __build_atomic_from_typical<__build_exchange<__atomic_ppc<long> > > __atomic_long;
typedef detail::atomic::__build_atomic_from_add<__atomic_fallback_integral<long long> > __atomic_llong;
typedef detail::atomic::__build_atomic_from_add<__atomic_fallback_integral<unsigned long long> > __atomic_ullong;

typedef detail::atomic::__build_atomic_ptr_from_minimal<__atomic_ppc_address > __atomic_address;

}
}
}

#endif
