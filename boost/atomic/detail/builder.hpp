#ifndef BOOST_DETAIL_ATOMIC_BUILDER_HPP
#define BOOST_DETAIL_ATOMIC_BUILDER_HPP

namespace boost {
namespace detail {
namespace atomic {

/*
given a Base that implements:

- load(memory_order order)
- compare_exchange_weak(IntegralType &expected, IntegralType desired, memory_order order)

generates exchange and compare_exchange_strong
*/
template<typename Base>
class __build_exchange : public Base {
public:
	typedef typename Base::IntegralType IntegralType;
	
	using Base::load;
	using Base::compare_exchange_weak;
	
	bool compare_exchange_strong(IntegralType &expected, IntegralType desired, memory_order order=memory_order_seq_cst) volatile
	{
		IntegralType expected_save=expected;
		while(true) {
			if (compare_exchange_weak(expected, desired, order)) return true;
			if (expected_save!=expected) return false;
			expected=expected_save;
		}
	}
	
	IntegralType exchange(IntegralType replacement, memory_order order=memory_order_seq_cst) volatile
	{
		IntegralType o=load(memory_order_relaxed);
		do {} while(!compare_exchange_weak(o, replacement, order));
		return o;
	}
	
	__build_exchange() {}
	explicit __build_exchange(IntegralType i) : Base(i) {}
};

/*
given a Base that implements:

- load(memory_order order)
- store(IntegralType v, memory_order order)

creates assignment and type conversion operator
*/
template<typename Base>
class __build_assign : public Base {
public:
	typedef typename Base::IntegralType IntegralType;
	
	using Base::load;
	using Base::store;
	
	operator IntegralType(void) const volatile {return load();}
	IntegralType operator=(IntegralType v) volatile {store(v); return v;}
	
	__build_assign() {}
	explicit __build_assign(IntegralType i) : Base(i) {}
};

/*
given a Base that implements:

- fetch_add_var(IntegralType c, memory_order order)
- fetch_inc(memory_order order)
- fetch_dec(memory_order order)

creates a fetch_add method that delegates to fetch_inc/fetch_dec if operand
is constant +1/-1, and uses fetch_add_var otherwise

the intention is to allow optimizing the incredibly common case of +1/-1
*/
template<typename Base>
class __build_const_fetch_add : public Base {
public:
	typedef typename Base::IntegralType IntegralType;
	
	IntegralType fetch_add(long c, memory_order order=memory_order_seq_cst) volatile
	{
		if (__builtin_constant_p(c)) {
			switch(c) {
				case -1: return fetch_dec(order);
				case 1: return fetch_inc(order);
			}
		}
		return fetch_add_var(c, order);
	}
	
	__build_const_fetch_add() {}
	explicit __build_const_fetch_add(IntegralType i) : Base(i) {}
protected:
	using Base::fetch_add_var;
	using Base::fetch_inc;
	using Base::fetch_dec;
};

/*
given a Base that implements:

- load(memory_order order)
- compare_exchange_weak(IntegralType &expected, IntegralType desired, memory_order order)

generates a -- not very efficient, but correct -- fetch_add operation
*/
template<typename Base>
class __build_fetch_add : public Base {
public:
	typedef typename Base::IntegralType IntegralType;
	
	using Base::compare_exchange_weak;
	
	IntegralType fetch_add(long c, memory_order order=memory_order_seq_cst) volatile
	{
		IntegralType o=Base::load(memory_order_relaxed), n;
		do {n=o+c;} while(!compare_exchange_weak(o, n, order));
		return o;
	}
	
	__build_fetch_add() {}
	explicit __build_fetch_add(IntegralType i) : Base(i) {}
};

/*
given a Base that implements:

- fetch_add(IntegralType c, memory_order order)

generates fetch_sub and post/pre- increment/decrement operators
*/
template<typename Base>
class __build_arithmeticops : public Base {
public:
	typedef typename Base::IntegralType IntegralType;
	
	using Base::fetch_add;
	
	IntegralType fetch_sub(long c, memory_order order=memory_order_seq_cst) volatile
	{
		return fetch_add(-c, order);
	}
	
	IntegralType operator++(void) volatile {return fetch_add(1)+1;}
	IntegralType operator++(int) volatile {return fetch_add(1);}
	IntegralType operator--(void) volatile {return fetch_sub(1)-1;}
	IntegralType operator--(int) volatile {return fetch_sub(1);}
	
	IntegralType operator+=(IntegralType c) volatile {return fetch_add(c)+c;}
	IntegralType operator-=(IntegralType c) volatile {return fetch_sub(c)-c;}
	
	__build_arithmeticops() {}
	explicit __build_arithmeticops(IntegralType i) : Base(i) {}
};

/*
given a Base that implements:

- load(memory_order order)
- compare_exchange_weak(IntegralType &expected, IntegralType desired, memory_order order)

generates -- not very efficient, but correct -- fetch_and, fetch_or and fetch_xor operators
*/
template<typename Base>
class __build_logicops : public Base {
public:
	typedef typename Base::IntegralType IntegralType;
	
	using Base::compare_exchange_weak;
	using Base::load;
	
	IntegralType fetch_and(IntegralType c, memory_order order=memory_order_seq_cst) volatile
	{
		IntegralType o=load(memory_order_relaxed), n;
		do {n=o&c;} while(!compare_exchange_weak(o, n, order));
		return o;
	}
	IntegralType fetch_or(IntegralType c, memory_order order=memory_order_seq_cst) volatile
	{
		IntegralType o=load(memory_order_relaxed), n;
		do {n=o|c;} while(!compare_exchange_weak(o, n, order));
		return o;
	}
	IntegralType fetch_xor(IntegralType c, memory_order order=memory_order_seq_cst) volatile
	{
		IntegralType o=load(memory_order_relaxed), n;
		do {n=o^c;} while(!compare_exchange_weak(o, n, order));
		return o;
	}
	
	IntegralType operator&=(IntegralType c) volatile {return fetch_and(c)&c;}
	IntegralType operator-=(IntegralType c) volatile {return fetch_or(c)&c;}
	IntegralType operator^=(IntegralType c) volatile {return fetch_xor(c)&c;}
	
	__build_logicops() {}
	__build_logicops(IntegralType i) : Base(i) {}
};

/*
given a Base that implements:

- load(memory_order order)
- store(IntegralType i, memory_order order)
- compare_exchange_weak(IntegralType &expected, IntegralType desired, memory_order order)

generates the full set of atomic operations for integral types
*/
template<typename Base>
class __build_atomic_from_minimal : public __build_logicops< __build_arithmeticops< __build_fetch_add< __build_exchange<__build_assign<Base> > > > > {
public:
	typedef __build_logicops< __build_arithmeticops< __build_fetch_add< __build_exchange<__build_assign<Base> > > > > __super;
	
	__build_atomic_from_minimal(void) {}
	__build_atomic_from_minimal(typename __super::IntegralType i) : __super(i) {}
};

/*
given a Base that implements:

- load(memory_order order)
- store(IntegralType i, memory_order order)
- compare_exchange_weak(IntegralType &expected, IntegralType desired, memory_order order)
- compare_exchange_strong(IntegralType &expected, IntegralType desired, memory_order order)
- exchange(IntegralType replacement, memory_order order)
- fetch_add_var(IntegralType c, memory_order order)
- fetch_inc(memory_order order)
- fetch_dec(memory_order order)

generates the full set of atomic operations for integral types
*/
template<typename Base>
class __build_atomic_from_typical : public __build_logicops< __build_arithmeticops< __build_const_fetch_add<__build_assign<Base> > > > {
public:
	typedef __build_logicops< __build_arithmeticops< __build_const_fetch_add<__build_assign<Base> > > > __super;
	
	__build_atomic_from_typical(void) {}
	__build_atomic_from_typical(typename __super::IntegralType i) : __super(i) {}
};

/*
given a Base that implements:

- load(memory_order order)
- store(IntegralType i, memory_order order)
- compare_exchange_weak(IntegralType &expected, IntegralType desired, memory_order order)
- compare_exchange_strong(IntegralType &expected, IntegralType desired, memory_order order)
- exchange(IntegralType replacement, memory_order order)
- fetch_add(IntegralType c, memory_order order)

generates the full set of atomic operations for integral types
*/
template<typename Base>
class __build_atomic_from_add : public __build_logicops< __build_arithmeticops< __build_assign<Base> > > {
public:
	typedef __build_logicops< __build_arithmeticops< __build_assign<Base> > > __super;
	
	__build_atomic_from_add(void) {}
	__build_atomic_from_add(typename __super::IntegralType i) : __super(i) {}
};

/*
given a Base that implements:

- load(memory_order order)
- store(IntegralType i, memory_order order)
- compare_exchange_weak(IntegralType &expected, IntegralType desired, memory_order order)

generates the full set of atomic operations for pointers
*/
template<typename Base>
class __build_atomic_ptr_from_minimal : public __build_exchange<__build_assign<Base> > {
public:
	typedef __build_exchange<__build_assign<Base> > __super;
	
	__build_atomic_ptr_from_minimal(void) {}
	__build_atomic_ptr_from_minimal(typename __super::IntegralType i) : __super(i) {}
};

template<typename Base>
class __build_atomic_ptr_from_typical : public __build_assign<Base> {
public:
	typedef __build_assign<Base> __super;
	
	__build_atomic_ptr_from_typical(void) {}
	__build_atomic_ptr_from_typical(typename __super::IntegralType i) : __super(i) {}
};

}
}
}

#endif
