//  lock-free freelist
//
//  Copyright (C) 2008 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_FREELIST_HPP_INCLUDED
#define BOOST_LOCKFREE_FREELIST_HPP_INCLUDED

#include <boost/lockfree/tagged_ptr.hpp>
#include <boost/lockfree/atomic_int.hpp>

namespace boost
{
namespace lockfree
{

/** dummy freelist  */
template <typename T>
struct dummy_freelist
{
    T * allocate (void)
    {
        return static_cast<T*>(operator new(sizeof(T)));
    }

    void deallocate (T * n)
    {
        operator delete(n);
    }
};


/** simple freelist implementation  */
template <typename T, unsigned int max_size = 64>
class freelist:
    public dummy_freelist<T>
{
    struct freelist_node
    {
        tagged_ptr<struct freelist_node> next;
    };

    typedef lockfree::tagged_ptr<struct freelist_node> tagged_ptr;

public:
    freelist(void):
        pool_(NULL)
    {}

    ~freelist(void)
    {
        free_memory_pool();
    }

    T * allocate (void)
    {
        for(;;)
        {
            tagged_ptr old_pool(pool_);

            if (not old_pool)
                return dummy_freelist<T>::allocate();

            tagged_ptr new_pool (old_pool->next);

            new_pool.set_tag(old_pool.get_tag() + 1);

            if (pool_.CAS(old_pool, new_pool))
            {
                --free_list_size;
                return reinterpret_cast<T*>(old_pool.get_ptr());
            }
        }
    }

    void deallocate (T * n)
    {
        if (free_list_size > max_size)
        {
            dummy_freelist<T>::deallocate(n);
            return;
        }

        for(;;)
        {
            tagged_ptr old_pool (pool_);

            freelist_node * fl_node = reinterpret_cast<freelist_node*>(n);

            fl_node->next.set_ptr(old_pool.get_ptr());

            tagged_ptr new_pool (fl_node, old_pool.get_tag() + 1);

            if (pool_.CAS(old_pool, new_pool))
            {
                --free_list_size;
                return;
            }
        }
    }

private:
    void free_memory_pool(void)
    {
        tagged_ptr current (pool_);

        while (current)
        {
            freelist_node * n = current.get_ptr();
            current.set(current->next);
            operator delete(n);
        }
    }

    tagged_ptr pool_;
    atomic_int<long> free_list_size;
};

template <typename T>
class caching_freelist:
    public dummy_freelist<T>
{
    struct freelist_node
    {
        tagged_ptr<struct freelist_node> next;
    };

    typedef lockfree::tagged_ptr<struct freelist_node> tagged_ptr;

public:
    caching_freelist(void):
        pool_(NULL)
    {}

    ~caching_freelist(void)
    {
        free_memory_pool();
    }

    T * allocate (void)
    {
        for(;;)
        {
            tagged_ptr old_pool(pool_);

            if (not old_pool)
                return dummy_freelist<T>::allocate();

            tagged_ptr new_pool (old_pool->next);

            new_pool.set_tag(old_pool.get_tag() + 1);

            if (pool_.CAS(old_pool, new_pool))
                return reinterpret_cast<T*>(old_pool.get_ptr());
        }
    }

    void deallocate (T * n)
    {
        for(;;)
        {
            tagged_ptr old_pool (pool_);

            freelist_node * fl_node = reinterpret_cast<freelist_node*>(n);

            fl_node->next.set_ptr(old_pool.get_ptr());

            tagged_ptr new_pool (fl_node, old_pool.get_tag() + 1);

            if (pool_.CAS(old_pool, new_pool))
                return;
        }
    }

private:
    void free_memory_pool(void)
    {
        tagged_ptr current (pool_);

        while (current)
        {
            freelist_node * n = current.get_ptr();
            current.set(current->next);
            operator delete(n);
        }
    }

    tagged_ptr pool_;
};


} /* namespace lockfree */
} /* namespace boost */

#endif /* BOOST_LOCKFREE_FREELIST_HPP_INCLUDED */
