#ifndef BOOST_DETAIL_ATOMIC_FALLBACK_HPP
#define BOOST_DETAIL_ATOMIC_FALLBACK_HPP

#include <boost/atomic/detail/builder.hpp>
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
class __atomic_fallback {
public:
	__atomic_fallback(void) {}
	explicit __atomic_fallback(const T &t) : i(t) {}
	
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
};

template<typename T>
class __atomic_fallback_integral : public __atomic_fallback<T> {
protected:
	using __atomic_fallback<T>::i;
public:
	__atomic_fallback_integral(void) {}
	explicit __atomic_fallback_integral(const T &t) : __atomic_fallback<T>(t) {}
	
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		/* assume that things up to machine word size can be
		read atomically */
		if (sizeof(T)<=sizeof(void *) && order==memory_order_relaxed)
			return i;
		else {
			atomic_guard guard(&i, order);
			return i;
		}
	}
	T fetch_add(long long c, memory_order order=memory_order_seq_cst) volatile
	{
		atomic_guard guard(&i, order);
		T tmp=i;
		i+=c;
		return tmp;
	}
protected:
	typedef T IntegralType;
};

}
}
}

#endif
