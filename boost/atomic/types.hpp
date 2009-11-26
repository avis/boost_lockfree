#ifndef __BOOST_ATOMIC_TYPES_HPP
#define __BOOST_ATOMIC_TYPES_HPP

namespace boost {

enum memory_order {
	memory_order_relaxed,
	memory_order_consume,
	memory_order_acquire,
	memory_order_release,
	memory_order_acq_rel,
	memory_order_seq_cst
};

template<typename T>
class atomic {
private:
	atomic(void) {}
	atomic(T v) : i(v) {}
	atomic(const atomic<T> &t) {}
	void operator=(const atomic<T> &t) {}
	
	T i;
};

typedef atomic<unsigned char> atomic_uchar;
typedef atomic<signed char> atomic_schar;
typedef atomic<char> atomic_char;
typedef atomic<unsigned short> atomic_ushort;
typedef atomic<short> atomic_short;
typedef atomic<unsigned int> atomic_uint;
typedef atomic<int> atomic_int;
typedef atomic<unsigned long> atomic_ulong;
typedef atomic<long> atomic_long;
typedef atomic<unsigned long long> atomic_ullong;
typedef atomic<long long> atomic_llong;

}

#endif
