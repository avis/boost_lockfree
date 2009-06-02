//  lock-free fifo queue from
//  Michael, M. M. and Scott, M. L.,
//  "simple, fast and practical non-blocking and blocking concurrent queue algorithms"
//
//  implementation for c++
//
//  Copyright (C) 2008 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_FIFO_HPP_INCLUDED
#define BOOST_LOCKFREE_FIFO_HPP_INCLUDED

#include <boost/lockfree/tagged_ptr.hpp>
#include <boost/lockfree/atomic_int.hpp>
#include <boost/lockfree/freelist.hpp>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pod.hpp>

#include <memory>               /* std::auto_ptr */
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace boost
{
namespace lockfree
{
namespace detail
{

template <typename T, typename Alloc>
class fifo:
    boost::noncopyable
{
    BOOST_STATIC_ASSERT(boost::is_pod<T>::value);

    struct BOOST_LOCKFREE_CACHELINE_ALIGNMENT node
    {
        node(T const & v):
            data(v)
        {
            next.set(NULL, next.get_tag()+1); /* increment tag to avoid ABA problem */
        }

        node (void):
            next(NULL)
        {}

        tagged_ptr<node> next;
        T data;
    };

    typedef tagged_ptr<node> atomic_node_ptr;


public:
    fifo(void)
    {
        node * n = alloc_node();
        head_.set_ptr(n);
        tail_.set_ptr(n);
    }

    explicit fifo(std::size_t initial_nodes):
        pool(initial_nodes)
    {
        node * n = alloc_node();
        head_.set_ptr(n);
        tail_.set_ptr(n);
    }

    ~fifo(void)
    {
        assert(empty());
        dealloc_node(head_.get_ptr());
    }

    bool empty(void) const
    {
        return head_.get_ptr() == tail_.get_ptr();
    }

    void enqueue(T const & t)
    {
        node * n = alloc_node(t);

        for (;;)
        {
            atomic_node_ptr tail (tail_);
            memory_barrier();
            atomic_node_ptr next (tail->next);
            memory_barrier();

            if (likely(tail == tail_))
            {
                if (next.get_ptr() == 0)
                {
                    if ( tail->next.CAS(next, n) )
                    {
                        tail_.CAS(tail, n);
                        return;
                    }
                }
                else
                    tail_.CAS(tail, next.get_ptr());
            }
        }
    }

    bool dequeue (T * ret)
    {
        for (;;)
        {
            atomic_node_ptr head (head_);
            memory_barrier();

            atomic_node_ptr tail(tail_);
            node * next = head->next.get_ptr();
            memory_barrier();

            if (likely(head == head_))
            {
                if (head.get_ptr() == tail.get_ptr())
                {
                    if (next == 0)
                        return false;

                    tail_.CAS(tail, next);
                }
                else
                {
                    *ret = next->data;
                    if (head_.CAS(head, next))
                    {
                        dealloc_node(head.get_ptr());

                        return true;
                    }
                }
            }
        }
    }

private:
    node * alloc_node(void)
    {
        node * chunk = pool.allocate();
        new(chunk) node();
        return chunk;
    }

    node * alloc_node(T const & t)
    {
        node * chunk = pool.allocate();
        new(chunk) node(t);
        return chunk;
    }

    void dealloc_node(node * n)
    {
        n->~node();
        pool.deallocate(n);
    }

    typedef typename Alloc::template rebind<node>::other node_allocator;

    boost::lockfree::caching_freelist<node, node_allocator> pool;

    atomic_node_ptr head_;
    atomic_node_ptr BOOST_LOCKFREE_CACHELINE_ALIGNMENT tail_; /* force head_ and tail_ to different cache lines! */
};

} /* namespace detail */

/** lockfree fifo
 *
 *  - wrapper for detail::fifo
 * */
template <typename T,
          typename Alloc = std::allocator<T> >
class fifo:
    public detail::fifo<T, Alloc>
{
public:
    fifo(void)
    {}

    explicit fifo(std::size_t initial_nodes):
        detail::fifo<T, Alloc>(initial_nodes)
    {}
};


/** lockfree fifo, template specialization for pointer-types
 *
 *  - wrapper for detail::fifo
 *  - overload dequeue to support smart pointers
 * */
template <typename T, typename Alloc>
class fifo<T*, Alloc>:
    public detail::fifo<T*, Alloc>
{
    typedef detail::fifo<T*, Alloc> fifo_t;

    template <typename smart_ptr>
    bool dequeue_smart_ptr(smart_ptr & ptr)
    {
        T * result = 0;
        bool success = fifo_t::dequeue(&result);

        if (success)
            ptr.reset(result);
        return success;
    }

public:
    fifo(void)
    {}

    explicit fifo(std::size_t initial_nodes):
        fifo_t(initial_nodes)
    {}

    void enqueue(T * t)
    {
        fifo_t::enqueue(t);
    }

    bool dequeue (T ** ret)
    {
        return fifo_t::dequeue(ret);
    }

    bool dequeue (std::auto_ptr<T> & ret)
    {
        return dequeue_smart_ptr(ret);
    }

    bool dequeue (boost::scoped_ptr<T> & ret)
    {
        BOOST_STATIC_ASSERT(sizeof(boost::scoped_ptr<T>) == sizeof(T*));
        return dequeue(reinterpret_cast<T**>(&ret));
    }

    bool dequeue (boost::shared_ptr<T> & ret)
    {
        return dequeue_smart_ptr(ret);
    }
};

} /* namespace lockfree */
} /* namespace boost */


#endif /* BOOST_LOCKFREE_FIFO_HPP_INCLUDED */
