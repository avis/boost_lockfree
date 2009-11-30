#ifndef BOOST_DETAIL_ATOMIC_VALID_INTEGRAL_TYPES_HPP
#define BOOST_DETAIL_ATOMIC_VALID_INTEGRAL_TYPES_HPP

#include <stdint.h>

namespace boost {
namespace detail {
namespace atomic {

/* list all types for which corresponding atomic types can be defined */
template<class T> class valid_atomic_type;

template<> class valid_atomic_type<bool> {};
template<> class valid_atomic_type<char> {};
template<> class valid_atomic_type<unsigned char> {};
template<> class valid_atomic_type<signed char> {};
//template<> class valid_atomic_type<uint8_t> {};
//template<> class valid_atomic_type<int8_t> {};
template<> class valid_atomic_type<unsigned short> {};
template<> class valid_atomic_type<short> {};
//template<> class valid_atomic_type<uint16_t> {};
//template<> class valid_atomic_type<int16_t> {};
template<> class valid_atomic_type<int> {};
template<> class valid_atomic_type<unsigned int> {};
//template<> class valid_atomic_type<uint32_t> {};
//template<> class valid_atomic_type<int32_t> {};
template<> class valid_atomic_type<long> {};
template<> class valid_atomic_type<unsigned long> {};
//template<> class valid_atomic_type<uint64_t> {};
//template<> class valid_atomic_type<int64_t> {};
template<> class valid_atomic_type<long long> {};
template<> class valid_atomic_type<unsigned long long> {};

}
}
}

#endif
