#ifndef __BOOST_DETAIL_GENERIC_ATOMIC_HPP
#define __BOOST_DETAIL_GENERIC_ATOMIC_HPP

namespace boost { namespace detail { namespace atomic {

template<typename T>
class __platform_atomic<T, 1> : private __platform_atomic_integral<uint8_t> {
public:
	typedef __platform_atomic_integral<uint8_t> super;
	typedef union { T e; uint8_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(*reinterpret_cast<uint8_t *>(&t)) {}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=t;
		super::store(c.i, order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		conv c;
		c.i=super::load(order);
		return c.e;
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_strong(_expected.i, _desired.i, order);
		expected=_expected.e;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_weak(_expected.i, _desired.i, order);
		expected=_expected.i;
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=replacement;
		c.i=super::exchange(c.i, order);
		return c.e;
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
};

template<typename T>
class __platform_atomic<T, 2> : private __platform_atomic_integral<uint16_t> {
public:
	typedef __platform_atomic_integral<uint16_t> super;
	typedef union { T e; uint16_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(*reinterpret_cast<uint16_t *>(&t)) {}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=t;
		super::store(c.i, order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		conv c;
		c.i=super::load(order);
		return c.e;
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_strong(_expected.i, _desired.i, order);
		expected=_expected.e;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_weak(_expected.i, _desired.i, order);
		expected=_expected.i;
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=replacement;
		c.i=super::exchange(c.i, order);
		return c.e;
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
};

template<typename T>
class __platform_atomic<T, 4> : private __platform_atomic_integral<uint32_t> {
public:
	typedef __platform_atomic_integral<uint32_t> super;
	typedef union { T e; uint32_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(*reinterpret_cast<uint32_t *>(&t)) {}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=t;
		super::store(c.i, order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		conv c;
		c.i=super::load(order);
		return c.e;
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_strong(_expected.i, _desired.i, order);
		expected=_expected.e;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_weak(_expected.i, _desired.i, order);
		expected=_expected.i;
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=replacement;
		c.i=super::exchange(c.i, order);
		return c.e;
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
};

template<typename T>
class __platform_atomic<T, 8> : private __platform_atomic_integral<uint64_t> {
public:
	typedef __platform_atomic_integral<uint64_t> super;
	typedef union { T e; uint64_t i;} conv;
	
	__platform_atomic() {}
	explicit __platform_atomic(T t) : super(*reinterpret_cast<uint64_t *>(&t)) {}
	
	void store(T t, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=t;
		super::store(c.i, order);
	}
	T load(memory_order order=memory_order_seq_cst) volatile const
	{
		conv c;
		c.i=super::load(order);
		return c.e;
	}
	bool compare_exchange_strong(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_strong(_expected.i, _desired.i, order);
		expected=_expected.e;
		return success;
	}
	bool compare_exchange_weak(T &expected, T desired, memory_order order=memory_order_seq_cst) volatile
	{
		conv _expected, _desired;
		_expected.e=expected;
		_desired.e=desired;
		bool success=super::compare_exchange_weak(_expected.i, _desired.i, order);
		expected=_expected.i;
		return success;
	}
	
	T exchange(T replacement, memory_order order=memory_order_seq_cst) volatile
	{
		conv c;
		c.e=replacement;
		c.i=super::exchange(c.i, order);
		return c.e;
	}
	
	operator T(void) const volatile {return load();}
	T operator=(T v) volatile {store(v); return v;}	
	
	using super::is_lock_free;
};

} } }

#endif
