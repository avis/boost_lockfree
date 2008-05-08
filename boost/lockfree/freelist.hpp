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

#include <algorithm>            /* for std::min */

namespace boost
{
namespace lockfree
{

/** dummy freelist  */
template <typename T, typename Alloc = std::allocator<T> >
struct dummy_freelist
{
    T * allocate (void)
    {
        return allocator.allocate(1);
    }

    void deallocate (T * n)
    {
        allocator.deallocate(n, 1);
    }

    Alloc allocator;
};


/** simple freelist implementation  */
template <typename T,
          std::size_t max_size = 64,
          typename Alloc = std::allocator<T> >
class freelist
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

    explicit freelist(std::size_t initial_nodes):
        pool_(NULL)
    {
        for (int i = 0; i != std::min(initial_nodes, max_size); ++i)
        {
            T * node = allocator.allocate(1);
            deallocate(node);
        }
    }

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
                return allocator.allocate(1);

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
            allocator.deallocate(n, 1);
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
            allocator.deallocate(reinterpret_cast<T*>(n), 1);
        }
    }

    tagged_ptr pool_;
    atomic_int<long> free_list_size;
    Alloc allocator;
};

template <typename T, typename Alloc = std::allocator<T> >
class caching_freelist
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

    explicit caching_freelist(std::size_t initial_nodes):
        pool_(NULL)
    {
        for (int i = 0; i != initial_nodes; ++i)
        {
            T * node = allocator.allocate(1);
            deallocate(node);
        }
    }

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
                return allocator.allocate(1);

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
            allocator.deallocate(reinterpret_cast<T*>(n), 1);
        }
    }

    tagged_ptr pool_;
    Alloc allocator;
};

} /* namespace lockfree */
} /* namespace boost */

#endif /* BOOST_LOCKFREE_FREELIST_HPP_INCLUDED */
