#ifndef BOOST_DETAIL_ATOMIC_VALID_INTEGRAL_TYPES_HPP
#define BOOST_DETAIL_ATOMIC_VALID_INTEGRAL_TYPES_HPP

#include <stdint.h>

namespace boost {
namespace detail {
namespace atomic {

template<typename T> struct is_integral_type {typedef void test;};

template<> struct is_integral_type<char> {typedef int test;};

template<> struct is_integral_type<unsigned char> {typedef int test;};
template<> struct is_integral_type<signed char> {typedef int test;};
template<> struct is_integral_type<unsigned short> {typedef int test;};
template<> struct is_integral_type<short> {typedef int test;};
template<> struct is_integral_type<unsigned int> {typedef int test;};
template<> struct is_integral_type<int> {typedef int test;};
template<> struct is_integral_type<unsigned long> {typedef int test;};
template<> struct is_integral_type<long> {typedef int test;};
template<> struct is_integral_type<unsigned long long> {typedef int test;};
template<> struct is_integral_type<long long> {typedef int test;};

}
}
}

#endif
