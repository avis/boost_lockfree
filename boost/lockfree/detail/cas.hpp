//  Copyright (C) 2007, 2008, 2009 Tim Blechmann & Thomas Grill
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_CAS_HPP_INCLUDED
#define BOOST_LOCKFREE_CAS_HPP_INCLUDED

#include <boost/lockfree/detail/prefix.hpp>
#include <boost/detail/lightweight_mutex.hpp>
#include <boost/static_assert.hpp>

#include <boost/cstdint.hpp>

#include <boost/mpl/map.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/long.hpp>

#ifdef __SSE2__
#include "emmintrin.h"
#endif

namespace boost
{
namespace lockfree
{

inline void memory_barrier(void)
{
#if defined(__SSE2__)
    _mm_mfence();

#elif defined(__GNUC__) && ( (__GNUC__ > 4) || ((__GNUC__ >= 4) &&      \
                                                (__GNUC_MINOR__ >= 1))) \
    || defined(__INTEL_COMPILER)
    __sync_synchronize();
#elif defined(__GNUC__) && defined (__i386__)
    asm volatile("lock; addl $0,0(%%esp)":::"memory")
#elif defined(_MSC_VER) && (_MSC_VER >= 1300)
    _ReadWriteBarrier();
#elif defined(__APPLE__)
    OSMemoryBarrier();
#elif defined(AO_HAVE_nop_full)
    AO_nop_full();
#else
#   warning "no memory barrier implemented for this platform"
#endif
}

inline void read_memory_barrier(void)
{
#if defined(__SSE2__)
    _mm_lfence();
#else
    memory_barrier();
#endif
}

template <typename C>
struct atomic_cas_emulator
{
    static inline bool cas(volatile C * addr, C old, C nw)
    {
        static boost::detail::lightweight_mutex guard;
        boost::detail::lightweight_mutex::scoped_lock lock(guard);

        if (*addr == old)
        {
            *addr = nw;
            return true;
        }
        else
            return false;
    }

    typedef C cas_type;
};


template <typename C>
inline bool atomic_cas_emulation(volatile C * addr, C old, C nw)
{
    return atomic_cas_emulator<C>::cas(addr, old, nw);
}

using boost::uint32_t;
using boost::uint64_t;

struct atomic_cas32
{
    static inline bool cas(volatile uint32_t * addr,
                           uint64_t const & old,
                           uint64_t const & nw)
    {
#if defined(__GNUC__) && ( (__GNUC__ > 4) || ((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)) ) || defined(__INTEL_COMPILER)
        return __sync_bool_compare_and_swap(addr, old, nw);
#else
        return boost::interprocess::detail::atomic_cas32(addr, old, nw) == old;
#endif
    }
    typedef uint32_t cas_type;
};

struct atomic_cas64
{
    static inline bool cas(volatile uint64_t * addr,
                           uint64_t const & old,
                           uint64_t const & nw)
    {
#if defined(__GNUC__) && ( (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)) ) || defined(__INTEL_COMPILER)
        return __sync_bool_compare_and_swap(addr, old, nw);
#elif defined(_M_IX86)
        return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(addr),
                                          reinterpret_cast<LONG>(nw),
                                          reinterpret_cast<LONG>(old)) == old;
#elif defined(_M_X64)
        return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(addr),
                                          reinterpret_cast<LONG>(nw),
                                          reinterpret_cast<LONG>(old)) == old;
#else
#warning ("blocking CAS emulation")
        return atomic_cas_emulation(addr, old, nw);
#endif
    }

    typedef uint64_t cas_type;
};

struct atomic_cas128
{
#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_16)
    typedef int cas_type __attribute__ ((mode (TI)));
#else
    struct cas_type
    {
        uint64_t data[2];
    };
#endif

    static inline bool cas(volatile cas_type * addr, cas_type const & old, cas_type const & nw)
    {
#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_16)
        return __sync_bool_compare_and_swap_16(addr, old, nw);
#else
#warning ("blocking CAS emulation")
        return atomic_cas_emulation(addr, old, nw);
#endif
    }
};

template <class C>
inline bool atomic_cas(volatile C * addr, C const & old, C const & nw)
{
    using namespace boost::mpl;

    typedef map3<pair<long_<4>, atomic_cas32>,
        pair<long_<8>, atomic_cas64>,
        pair<long_<16>, atomic_cas128>
        > cas_map;

    typedef typename at<cas_map, long_<sizeof(C)> >::type atomic_cas_t;

    typedef typename if_<has_key<cas_map, long_<sizeof(C)> >,
        atomic_cas_t,
        atomic_cas_emulator<C> >::type cas_t;

    typedef typename cas_t::cas_type cas_value_t;

    return cas_t::cas((volatile cas_value_t*)addr, *(cas_value_t*)&old, *(cas_value_t*)&nw);
}

} /* namespace lockfree */
} /* namespace boost */

#endif /* BOOST_LOCKFREE_CAS_HPP_INCLUDED */
