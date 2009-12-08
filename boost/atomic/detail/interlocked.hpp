#include <boost/detail/interlocked.hpp>

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
		default:;
	}
}

static inline void __fence_after(memory_order order)
{
	switch(order) {
		case memory_order_acquire:
		case memory_order_acq_rel:
		case memory_order_seq_cst:
		default:;
	}
}
static inline void __fence_after_load(memory_order order)
{
	switch(order) {
		case memory_order_seq_cst:
			long tmp;
			BOOST_INTERLOCKED_EXCHANGE(&tmp, 0);
		case memory_order_acquire:
		case memory_order_acq_rel:
		default:;
	}
}


template<typename T>
class atomic_interlocked_32 {
public:
	explicit atomic_interlocked_32(T v) : i(v) {}
	atomic_interlocked_32() {}
	T load(memory_order order=memory_order_seq_cst) const volatile
	{
		T v=*reinterpret_cast<volatile const T *>(&i);
		__fence_after_load(order);
		return v;
	}
	void store(T v, memory_order order=memory_order_seq_cst) volatile
	{
		if (order!=memory_order_seq_cst) {
			__fence_before(order);
			*reinterpret_cast<volatile T *>(&i)=v;
		} else {
			exchange(v);
		}
	}
	bool compare_exchange_strong(
		T &expected,
		T desired,
		memory_order success_order,
		memory_order failure_order) volatile
	{
		T prev=expected;
		expected=(T)BOOST_INTERLOCKED_COMPARE_EXCHANGE((long *)(&i), (long)desired, (long)expected);
		bool success=(prev==expected);
		return success;
	}
	bool compare_exchange_weak(
		T &expected,
		T desired,
		memory_order success_order,
		memory_order failure_order) volatile
	{
		return compare_exchange_strong(expected, desired, success_order, failure_order);
	}
	T exchange(T r, memory_order order=memory_order_seq_cst) volatile
	{
		return (T)BOOST_INTERLOCKED_EXCHANGE((long *)&i, (long)r);
	}
	T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
	{
		return (T)BOOST_INTERLOCKED_EXCHANGE_ADD((long *)&i, c);
	}
	
	bool is_lock_free(void) const volatile {return true;}
	
	typedef T integral_type;
private:
	T i;
};

template<typename T>
class platform_atomic_integral<T, 4> : public build_atomic_from_add<atomic_interlocked_32<T> > {
public:
	typedef build_atomic_from_add<atomic_interlocked_32<T> > super;
	explicit platform_atomic_integral(T v) : super(v) {}
	platform_atomic_integral(void) {}
};

template<typename T>
class platform_atomic_integral<T, 1>: public build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> {
public:
	typedef build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> super;
	
	explicit platform_atomic_integral(T v) : super(v) {}
	platform_atomic_integral(void) {}
};

template<typename T>
class platform_atomic_integral<T, 2>: public build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> {
public:
	typedef build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> super;
	
	explicit platform_atomic_integral(T v) : super(v) {}
	platform_atomic_integral(void) {}
};

}
}
}
