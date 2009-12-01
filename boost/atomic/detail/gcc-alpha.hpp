#ifndef BOOST_DETAIL_ATOMIC_GCC_ALPHA_HPP
#define BOOST_DETAIL_ATOMIC_GCC_ALPHA_HPP

#include <boost/atomic/memory_order.hpp>
#include <boost/atomic/detail/base.hpp>
#include <boost/atomic/detail/builder.hpp>

/*
  Refer to http://h71000.www7.hp.com/doc/82final/5601/5601pro_004.html
  (HP OpenVMS systems documentation) and the alpha reference manual.
 */

/*
	NB: The most natural thing would be to write the increment/decrement
	operators along the following lines:
	
	__asm__ __volatile__(
		"1: ldl_l %0,%1 \n"
		"addl %0,1,%0 \n"
		"stl_c %0,%1 \n"
		"beq %0,1b\n"
		: "=&b" (tmp)
		: "m" (value)
		: "cc"
	);
	
	However according to the comments on the HP website and matching
	comments in the Linux kernel sources this defies branch prediction,
	as the cpu assumes that backward branches are always taken; so
	instead copy the trick from the Linux kernel, introduce a forward
	branch and back again.
	
	I have, however, had a hard time measuring the difference between
	the two versions in microbenchmarks -- I am leaving it in nevertheless
	as it apparently does not hurt either.
*/

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
			__asm__ __volatile__ ("mb" ::: "memory");
		default:;
	}
}

static inline void __fence_after(memory_order order)
{
	switch(order) {
		case memory_order_acquire:
		case memory_order_acq_rel:
		case memory_order_seq_cst:
			__asm__ __volatile__ ("mb" ::: "memory");
		default:;
	}
}

template<typename T>
class __atomic_alpha_4 {
public:
	typedef T integral_type;
	explicit __atomic_alpha_4(T v) : i(v) {}
	__atomic_alpha_4() {}
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
		int current, success;
		__asm__ __volatile__(
			"1: ldl_l %2, %4\n"
			"cmpeq %2, %0, %3\n"
			"mov %2, %0\n"
			"beq %3, 3f\n"
			"stl_c %1, %4\n"
			"2:\n"
			
			".subsection 2\n"
			"3: mov %3, %1\n"
			"br 2b\n"
			".previous\n"
			
			: "+&r" (expected), "+&r" (desired), "=&r"(current), "=&r"(success)
			: "m" (i)
			:
		);
		__fence_after(order);
		return desired;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	inline T fetch_add_var(T c, memory_order order) volatile
	{
		__fence_before(order);
		int original, modified;
		__asm__ __volatile__(
			"1: ldl_l %0, %2\n"
			"addl %0, %3, %1\n"
			"stl_c %1, %2\n"
			"beq %1, 2f\n"
			
			".subsection 2\n"
			"2: br 1b\n"
			".previous\n"
			
			: "=&r" (original), "=&r" (modified)
			: "m" (i), "r" (c)
			:
		);
		__fence_after(order);
		return original;
	}
	inline T fetch_inc(memory_order order) volatile
	{
		__fence_before(order);
		int original, modified;
		__asm__ __volatile__(
			"1: ldl_l %0, %2\n"
			"addl %0, 1, %1\n"
			"stl_c %1, %2\n"
			"beq %1, 2f\n"
			
			".subsection 2\n"
			"2: br 1b\n"
			".previous\n"
			
			: "=&r" (original), "=&r" (modified)
			: "m" (i)
			:
		);
		__fence_after(order);
		return original;
	}
	inline T fetch_dec(memory_order order) volatile
	{
		__fence_before(order);
		int original, modified;
		__asm__ __volatile__(
			"1: ldl_l %0, %2\n"
			"subl %0, 1, %1\n"
			"stl_c %1, %2\n"
			"beq %1, 2f\n"
			
			".subsection 2\n"
			"2: br 1b\n"
			".previous\n"
			
			: "=&r" (original), "=&r" (modified)
			: "m" (i)
			:
		);
		__fence_after(order);
		return original;
	}
private:
	T i;
};

template<typename T>
class __atomic_alpha_8 {
public:
	typedef T integral_type;
	explicit __atomic_alpha_8(T v) : i(v) {}
	__atomic_alpha_8() {}
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
		__fence_before(order);
		int current, success;
		__asm__ __volatile__(
			"1: ldq_l %2, %4\n"
			"cmpeq %2, %0, %3\n"
			"mov %2, %0\n"
			"beq %3, 3f\n"
			"stq_c %1, %4\n"
			"2:\n"
			
			".subsection 2\n"
			"3: mov %3, %1\n"
			"br 2b\n"
			".previous\n"
			
			: "+&r" (expected), "+&r" (desired), "=&r"(current), "=&r"(success)
			: "m" (i)
			:
		);
		__fence_after(order);
		return desired;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	inline T fetch_add_var(T c, memory_order order) volatile
	{
		__fence_before(order);
		T original, modified;
		__asm__ __volatile__(
			"1: ldq_l %0, %2\n"
			"addl %0, %3, %1\n"
			"stq_c %1, %2\n"
			"beq %1, 2f\n"
			
			".subsection 2\n"
			"2: br 1b\n"
			".previous\n"
			
			: "=&r" (original), "=&r" (modified)
			: "m" (i), "r" (c)
			:
		);
		__fence_after(order);
		return original;
	}
	inline T fetch_inc(memory_order order) volatile
	{
		__fence_before(order);
		T original, modified;
		__asm__ __volatile__(
			"1: ldq_l %0, %2\n"
			"addl %0, 1, %1\n"
			"stq_c %1, %2\n"
			"beq %1, 2f\n"
			
			".subsection 2\n"
			"2: br 1b\n"
			".previous\n"
			
			: "=&r" (original), "=&r" (modified)
			: "m" (i)
			:
		);
		__fence_after(order);
		return original;
	}
	inline T fetch_dec(memory_order order) volatile
	{
		__fence_before(order);
		T original, modified;
		__asm__ __volatile__(
			"1: ldq_l %0, %2\n"
			"subl %0, 1, %1\n"
			"stq_c %1, %2\n"
			"beq %1, 2f\n"
			
			".subsection 2\n"
			"2: br 1b\n"
			".previous\n"
			
			: "=&r" (original), "=&r" (modified)
			: "m" (i)
			:
		);
		__fence_after(order);
		return original;
	}
private:
	T i;
};

template<typename T>
class __platform_atomic_integral<T, 4> : public __build_atomic_from_typical<__build_exchange<__atomic_alpha_4<T> > > {
public:
	typedef __build_atomic_from_typical<__build_exchange<__atomic_alpha_4<T> > > super;
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};

template<typename T>
class __platform_atomic_integral<T, 8> : public __build_atomic_from_typical<__build_exchange<__atomic_alpha_8<T> > > {
public:
	typedef __build_atomic_from_typical<__build_exchange<__atomic_alpha_8<T> > > super;
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};

template<typename T>
class __platform_atomic_integral<T, 1>: public __build_atomic_from_larger_type<__atomic_alpha_4<uint32_t>, T> {
public:
	typedef __build_atomic_from_larger_type<__atomic_alpha_4<uint32_t>, T> super;
	
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};

template<typename T>
class __platform_atomic_integral<T, 2>: public __build_atomic_from_larger_type<__atomic_alpha_4<uint32_t>, T> {
public:
	typedef __build_atomic_from_larger_type<__atomic_alpha_4<uint32_t>, T> super;
	
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};

typedef __build_exchange<__atomic_alpha_8<void *> > __platform_atomic_address;

}
}
}

#endif
