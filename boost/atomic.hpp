#ifndef __BOOST_ATOMIC_HPP
#define __BOOST_ATOMIC_HPP

#include <boost/atomic/types.hpp>
#include <boost/atomic/platform.hpp>

namespace boost {

template<>
class atomic<char> : public detail::atomic::__atomic_char {
public:
	atomic(void) {}
	explicit atomic(char v) : detail::atomic::__atomic_char(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<unsigned char> : public detail::atomic::__atomic_uchar {
public:
	atomic(void) {}
	explicit atomic(unsigned char v) : detail::atomic::__atomic_uchar(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<signed char> : public detail::atomic::__atomic_schar {
public:
	atomic(void) {}
	explicit atomic(signed char v) : detail::atomic::__atomic_schar(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<unsigned short> : public detail::atomic::__atomic_ushort {
public:
	atomic(void) {}
	explicit atomic(unsigned short v) : detail::atomic::__atomic_ushort(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<short> : public detail::atomic::__atomic_short {
public:
	atomic(void) {}
	explicit atomic(short v) : detail::atomic::__atomic_short(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<unsigned int> : public detail::atomic::__atomic_uint {
public:
	atomic(void) {}
	explicit atomic(unsigned int v) : detail::atomic::__atomic_uint(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<int> : public detail::atomic::__atomic_int {
public:
	atomic(void) {}
	explicit atomic(int v) : detail::atomic::__atomic_int(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<unsigned long> : public detail::atomic::__atomic_ulong {
public:
	atomic(void) {}
	explicit atomic(unsigned long v) : detail::atomic::__atomic_ulong(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<long> : public detail::atomic::__atomic_long {
public:
	atomic(void) {}
	explicit atomic(long v) : detail::atomic::__atomic_long(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<unsigned long long> : public detail::atomic::__atomic_ullong {
public:
	atomic(void) {}
	explicit atomic(unsigned long long v) : detail::atomic::__atomic_ullong(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

template<>
class atomic<long long> : public detail::atomic::__atomic_llong {
public:
	atomic(void) {}
	explicit atomic(long long v) : detail::atomic::__atomic_llong(v) {}
private:
	atomic(const atomic &a) {}
	void operator=(const atomic &a) {}
};

class atomic_address : public detail::atomic::__atomic_address {
public:
	atomic_address(void) {}
	explicit atomic_address(void * v) : detail::atomic::__atomic_address(v) {}
private:
	atomic_address(const atomic_address &a) {}
	void operator=(const atomic_address &a) {}
};

template<typename T>
class atomic<T *> : private atomic_address {
public:
	atomic(void) {}
	explicit atomic(T * v) : atomic_address(v) {}
	T *load(memory_order order=memory_order_seq_cst) const volatile
	{
		return static_cast<T *>(atomic_address::load(order));
	}
	void store(T * v, memory_order order=memory_order_seq_cst) volatile
	{
		atomic_address::store(v, order);
	}
	bool compare_exchange_weak(T *&expected, T *desired, memory_order order=memory_order_seq_cst) volatile
	{
		void **expected_void = reinterpret_cast<void **>(&expected);
		return atomic_address::compare_exchange_weak(*expected_void, static_cast<void *>(desired), order);
	}
	bool compare_exchange_strong(T *&expected, T *desired, memory_order order=memory_order_seq_cst) volatile
	{
		void **expected_void = reinterpret_cast<void **>(&expected);
		return atomic_address::compare_exchange_strong(*expected_void, static_cast<void *>(desired), order);
	}
	T *exchange(T *replacement, memory_order order=memory_order_seq_cst) volatile
	{
		return static_cast<T *>(atomic_address::exchange(static_cast<void *>(replacement)));
	}
	
	operator T *(void) const {return load();}
	T *operator=(T *v) {store(v); return v;}
};

}

#endif
