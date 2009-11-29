#ifndef BOOST_DETAIL_ATOMIC_FALLBACK_HPP
#define BOOST_DETAIL_ATOMIC_FALLBACK_HPP

#include <boost/thread/mutex.hpp>

namespace boost {
namespace detail {
namespace atomic {

class atomic_guard {
public:
	inline atomic_guard(const volatile void * address, memory_order order)
		: global(seq_cst_lock, order==memory_order_seq_cst),
		local(get_lock_for_address(address))
	{}
	inline ~atomic_guard(void) {}
protected:
	static boost::mutex &get_lock_for_address(volatile const void * address);
	mutex::scoped_lock global, local;
private:
	static boost::mutex seq_cst_lock;
};

template<typename T>
class __fallback_atomic {
public:
	__fallback_atomic(void) {}
	explicit __fallback_atomic(const T &t) : i(t) {}
	
	const T &store(const T &t, memory_order order=memory_order_seq_cst) volatile
	{
		atomic_guard guard(&i, order);
		i=t;
		return t;
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		atomic_guard guard(&i, order);
		return i;
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		atomic_guard guard(&i, order);
		if (i==expected) {i=desired; return true;}
		else {expected=i; return false;}
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		return compare_exchange_strong(expected, desired, order);
	}
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		atomic_guard guard(&i, order);
		T tmp=i;
		i=replacement;
		return tmp;
	}
	bool is_lock_free(void) const volatile {return false;}
protected:
	T i;
	typedef T integral_type;
};

}
}
}

#endif
