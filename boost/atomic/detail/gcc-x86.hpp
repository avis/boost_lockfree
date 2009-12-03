#ifndef BOOST_DETAIL_ATOMIC_GCC_X86_HPP
#define BOOST_DETAIL_ATOMIC_GCC_X86_HPP

#include <boost/atomic/memory_order.hpp>
#include <boost/atomic/detail/base.hpp>
#include <boost/atomic/detail/builder.hpp>

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
class __atomic_x86_1 {
public:
	explicit __atomic_x86_1(T v) : i(v) {}
	__atomic_x86_1() {}
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
	bool compare_exchange_strong(T &e, T d, memory_order order=memory_order_seq_cst) volatile
	{
		T prev=e;
		__asm__ __volatile__("lock cmpxchgb %1, %2\n" : "=a" (prev) : "q" (d), "m" (i), "a" (e) : "memory");
		bool success=(prev==e);
		e=prev;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T r, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("xchgb %0, %1\n" : "=r" (r) : "m"(i), "0" (r) : "memory");
		return r;
	}
	T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("lock xaddb %0, %1" : "+r" (c), "+m" (i) :: "memory");
		return c;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	typedef T integral_type;
private:
	T i;
};

template<typename T>
class __platform_atomic_integral<T, 1> : public __build_atomic_from_add<__atomic_x86_1<T> > {
public:
	typedef __build_atomic_from_add<__atomic_x86_1<T> > super;
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};

template<typename T>
class __atomic_x86_2 {
public:
	explicit __atomic_x86_2(T v) : i(v) {}
	__atomic_x86_2() {}
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
	bool compare_exchange_strong(T &e, T d, memory_order order=memory_order_seq_cst) volatile
	{
		T prev=e;
		__asm__ __volatile__("lock cmpxchgw %1, %2\n" : "=a" (prev) : "q" (d), "m" (i), "a" (e) : "memory");
		bool success=(prev==e);
		e=prev;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T r, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("xchgw %0, %1\n" : "=r" (r) : "m"(i), "0" (r) : "memory");
		return r;
	}
	T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("lock xaddw %0, %1" : "+r" (c), "+m" (i) :: "memory");
		return c;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	typedef T integral_type;
private:
	T i;
};

template<typename T>
class __platform_atomic_integral<T, 2> : public __build_atomic_from_add<__atomic_x86_2<T> > {
public:
	typedef __build_atomic_from_add<__atomic_x86_2<T> > super;
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};

template<typename T>
class __atomic_x86_4 {
public:
	explicit __atomic_x86_4(T v) : i(v) {}
	__atomic_x86_4() {}
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
	bool compare_exchange_strong(T &e, T d, memory_order order=memory_order_seq_cst) volatile
	{
		T prev=e;
		__asm__ __volatile__("lock cmpxchgl %1, %2\n" : "=a" (prev) : "q" (d), "m" (i), "a" (e) : "memory");
		bool success=(prev==e);
		e=prev;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T r, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("xchgl %0, %1\n" : "=r" (r) : "m"(i), "0" (r) : "memory");
		return r;
	}
	T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("lock xaddl %0, %1" : "+r" (c), "+m" (i) :: "memory");
		return c;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	typedef T integral_type;
private:
	T i;
};

template<typename T>
class __platform_atomic_integral<T, 4> : public __build_atomic_from_add<__atomic_x86_4<T> > {
public:
	typedef __build_atomic_from_add<__atomic_x86_4<T> > super;
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};

#if defined(__amd64__)
template<typename T>
class __atomic_x86_8 {
public:
	explicit __atomic_x86_8(T v) : i(v) {}
	__atomic_x86_8() {}
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
	bool compare_exchange_strong(T &e, T d, memory_order order=memory_order_seq_cst) volatile
	{
		T prev=e;
		__asm__ __volatile__("lock cmpxchgq %1, %2\n" : "=a" (prev) : "q" (d), "m" (i), "a" (e) : "memory");
		bool success=(prev==e);
		e=prev;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T r, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("xchgq %0, %1\n" : "=r" (r) : "m"(i), "0" (r) : "memory");
		return r;
	}
	T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
	{
		__asm__ __volatile__("lock xaddq %0, %1" : "+r" (c), "+m" (i) :: "memory");
		return c;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	typedef T integral_type;
private:
	T i;
};
#elif defined(__i686__)

template<typename T>
class __atomic_x86_8 {
private:
	typedef __atomic_x86_8 this_type;
public:
	explicit __atomic_x86_8(T v) : i(v) {}
	__atomic_x86_8() {}
	
	bool compare_exchange_strong(T &e, T d, memory_order order=memory_order_seq_cst) volatile
	{
		T prev=e;
		__asm__ __volatile__("lock cmpxchg8b %3\n" :
			"=A" (prev) : "b" ((long)d), "c" ((long)(d>>32)), "m" (i), "0" (prev) : "memory");
		bool success=(prev==e);
		e=prev;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T r, memory_order order=memory_order_seq_cst) volatile
	{
		T prev=i;
		do {} while(!compare_exchange_strong(prev, r, order));
		return prev;
	}
	
	T load(memory_order order=memory_order_seq_cst) const volatile
	{
		/* this is a bit problematic -- there is no other
		way to atomically load a 64 bit value, but of course
		compare_exchange requires write access to the memory
		area */
		T expected=i;
		do { } while(!const_cast<this_type *>(this)->compare_exchange_strong(expected, expected, order));
		return expected;
	}
	void store(T v, memory_order order=memory_order_seq_cst) volatile
	{
		exchange(v, order);
	}
	T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
	{
		T expected=i, desired;;
		do {
			desired=expected+c;
		} while(!compare_exchange_strong(expected, desired, order));
		return expected;
	}
	
	bool is_lock_free(void) const volatile {return true;}
protected:
	typedef T integral_type;
private:
	T i;
};

#endif

#if defined(__amd64__) || defined(__i686__)
template<typename T>
class __platform_atomic_integral<T, 8> : public __build_atomic_from_add<__atomic_x86_8<T> >{
public:
	typedef __build_atomic_from_add<__atomic_x86_8<T> > super;
	explicit __platform_atomic_integral(T v) : super(v) {}
	__platform_atomic_integral(void) {}
};
#endif

#if defined(__i386__)
typedef __atomic_x86_4<void *> __platform_atomic_address;
#else
typedef __atomic_x86_8<void *> __platform_atomic_address;
#endif

}
}
}

#endif
