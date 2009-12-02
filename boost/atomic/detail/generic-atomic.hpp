#ifndef __BOOST_DETAIL_GENERIC_ATOMIC_HPP
#define __BOOST_DETAIL_GENERIC_ATOMIC_HPP

#include <string.h>

namespace boost { namespace detail { namespace atomic {

template<typename T>
class __platform_atomic<T, 1> : private __platform_atomic_integral<uint8_t> {
public:
	typedef __platform_atomic_integral<uint8_t> super;
	typedef union { T e; uint8_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(to_integral(t))
	{
	}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		super::store(to_integral(t), order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		return from_integral(super::load(order));
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint8_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_strong(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint8_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_weak(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		return from_integral(super::exchange(to_integral(replacement), order));
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
protected:
	static inline uint8_t to_integral(T &t)
	{
		uint8_t tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
	static inline T from_integral(uint8_t t)
	{
		T tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
};

template<typename T>
class __platform_atomic<T, 2> : private __platform_atomic_integral<uint16_t> {
public:
	typedef __platform_atomic_integral<uint16_t> super;
	typedef union { T e; uint16_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(to_integral(t))
	{
	}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		super::store(to_integral(t), order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		return from_integral(super::load(order));
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint16_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_strong(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint16_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_weak(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		return from_integral(super::exchange(to_integral(replacement), order));
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
protected:
	static inline uint16_t to_integral(T &t)
	{
		uint16_t tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
	static inline T from_integral(uint16_t t)
	{
		T tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
};

template<typename T>
class __platform_atomic<T, 4> : private __platform_atomic_integral<uint32_t> {
public:
	typedef __platform_atomic_integral<uint32_t> super;
	typedef union { T e; uint32_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(to_integral(t))
	{
	}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		super::store(to_integral(t), order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		return from_integral(super::load(order));
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint32_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_strong(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint32_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_weak(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		return from_integral(super::exchange(to_integral(replacement), order));
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
protected:
	static inline uint32_t to_integral(T &t)
	{
		uint32_t tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
	static inline T from_integral(uint32_t t)
	{
		T tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
};

template<typename T>
class __platform_atomic<T, 8> : private __platform_atomic_integral<uint64_t> {
public:
	typedef __platform_atomic_integral<uint64_t> super;
	typedef union { T e; uint64_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(to_integral(t))
	{
	}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		super::store(to_integral(t), order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		return from_integral(super::load(order));
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint64_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_strong(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		uint64_t _expected, _desired;
		_expected=to_integral(expected);
		_desired=to_integral(desired);
		bool success=super::compare_exchange_weak(_expected, _desired, order);
		expected=from_integral(_expected);
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		return from_integral(super::exchange(to_integral(replacement), order));
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
protected:
	static inline uint64_t to_integral(T &t)
	{
		uint64_t tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
	static inline T from_integral(uint64_t t)
	{
		T tmp;
		memcpy(&tmp, &t, sizeof(t));
		return tmp;
	}
};

} } }

#endif
