//  tagged pointer, for aba prevention
//
//  Copyright (C) 2008 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_TAGGED_PTR_HPP_INCLUDED
#define BOOST_LOCKFREE_TAGGED_PTR_HPP_INCLUDED

#include "cas.hpp"
#include "branch_hints.hpp"

#include <cstddef>

namespace boost
{
namespace lockfree
{

template <class T>
class tagged_ptr
{
    typedef std::size_t tag_t;

public:
    /** uninitialized constructor  */
    tagged_ptr(void)//: ptr(0), tag(0)
    {}

    /** copy constructor */
    tagged_ptr(tagged_ptr const & p)//: ptr(0), tag(0)
    {
        set(p);
    }

    tagged_ptr(T * p, tag_t t = 0):
        ptr(p), tag(t)
    {}

    /** atomic set operations */
    /* @{ */
    void operator= (tagged_ptr const & p)
    {
        atomic_set(p);
    }

    void atomic_set(tagged_ptr const & p)
    {
        for (;;)
        {
            tagged_ptr old;
            old.set(*this);
            if(likely(CAS(old, p.ptr, p.tag)))
                return;
        }
    }

    void atomic_set(T * p, tag_t t)
    {
        for (;;)
        {
            tagged_ptr old;
            old.set(*this);

            if(likely(CAS(old, p, t)))
                return;
        }
    }
    /* @} */

    /** unsafe set operation */
    /* @{ */
    void set(tagged_ptr const & p)
    {
        ptr = p.ptr;
        tag = p.tag;
    }

    void set(T * p, tag_t t)
    {
        ptr = p;
        tag = t;
    }
    /* @} */

    /** comparing semantics */
    /* @{ */
    bool operator== (tagged_ptr const & p) const
    {
        return (ptr == p.ptr) && (tag == p.tag);
    }

    bool operator!= (tagged_ptr const & p) const
    {
        return !operator==(p);
    }
    /* @} */

    /** pointer access */
    /* @{ */
    T * get_ptr() const
    {
        return ptr;
    }

    void set_ptr(T * p)
    {
        ptr = p;
    }
    /* @} */

    /** tag access */
    /* @{ */
    tag_t get_tag() const
    {
        return tag;
    }

    void set_tag(tag_t t)
    {
        tag = t;
    }
    /* @} */

    /** compare and swap  */
    /* @{ */
    bool CAS(tagged_ptr const & oldval, tagged_ptr const & newval)
    {
        return boost::lockfree::CAS2(this, oldval.ptr, oldval.tag, newval.ptr, newval.tag);
    }

    bool CAS(tagged_ptr const & oldval, T * newptr)
    {
        return boost::lockfree::CAS2(this, oldval.ptr, oldval.tag, newptr, oldval.tag + 1);
    }

    bool CAS(tagged_ptr const & oldval, T * newptr, tag_t t)
    {
        return boost::lockfree::CAS2(this, oldval.ptr, oldval.tag, newptr, t);
    }

    bool CAS(const T * oldptr, tag_t oldtag, T * newptr)
    {
        return boost::lockfree::CAS2(oldptr, oldtag, newptr, oldtag + 1);
    }

/*     bool CAS(const T * oldptr, tag_t oldtag, T * newptr, tag_t newtag) */
/*     { */
/*         return lockfree::CAS2(const_cast<tagged_ptr*>(this), oldptr, oldtag, newptr, newtag); */
/*     } */
    /* @} */

    /** smart pointer support  */
    /* @{ */
    T & operator*() const
    {
        return *ptr;
    }

    T * operator->() const
    {
        return ptr;
    }

    operator bool(void) const
    {
        return bool (ptr);
    }
    /* @} */

protected:
    T * ptr DCAS_ALIGNMENT_ATTRIBUTE;
    tag_t tag;
};


} /* namespace lockfree */
} /* namespace boost */

#endif /* BOOST_LOCKFREE_TAGGED_PTR_HPP_INCLUDED */
