//  lock-free single-producer/single-consumer ringbuffer
//  this algorithm is implemented in various projects (jack, portaudio, supercollider)
//
//  implementation for c++
//
//  Copyright (C) 2009 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

#ifndef BOOST_LOCKFREE_RINGBUFFER_HPP_INCLUDED
#define BOOST_LOCKFREE_RINGBUFFER_HPP_INCLUDED

#include <boost/atomic.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/scoped_array.hpp>

#include "detail/branch_hints.hpp"
#include "detail/prefix.hpp"

namespace boost
{
namespace lockfree
{

namespace detail
{

template <typename T>
class ringbuffer_internal:
    boost::noncopyable
{
    static const int padding_size = BOOST_LOCKFREE_CACHELINE_BYTES - sizeof(size_t);
    atomic<size_t> write_index_;
    char padding1[padding_size]; /* force read_index and write_index to different cache lines */
    atomic<size_t> read_index_;

protected:
    ringbuffer_internal(void):
        write_index_(0), read_index_(0)
    {}

    static size_t next_index(size_t arg, size_t max_size)
    {
        size_t ret = arg + 1;
        while (unlikely(ret >= max_size))
            ret -= max_size;
        return ret;
    }

    bool enqueue(T const & t, T * buffer, size_t max_size)
    {
        size_t next = next_index(write_index_.load(memory_order_acquire), max_size);

        if (next == read_index_.load(memory_order_acquire))
            return false; /* ringbuffer is full */

        buffer[next] = t;

        write_index_.store(next, memory_order_release);

        return true;
    }

    bool dequeue (T * ret, T * buffer, size_t max_size)
    {
        if (empty())
            return false;

        size_t next = next_index(read_index_.load(memory_order_acquire), max_size);
        *ret = buffer[next];
        read_index_.store(next, memory_order_release);
        return true;
    }

public:
    void reset(void)
    {
        write_index_.store(0, memory_order_relaxed);
        read_index_.store(0, memory_order_release);
    }

    bool empty(void)
    {
        return write_index_.load(memory_order_relaxed) == read_index_.load(memory_order_relaxed);
    }

};

} /* namespace detail */

template <typename T, size_t max_size>
class ringbuffer:
    public detail::ringbuffer_internal<T>
{
    boost::array<T, max_size> array_;

public:
    bool enqueue(T const & t)
    {
        return detail::ringbuffer_internal<T>::enqueue(t, array_.c_array(), max_size);
    }

    bool dequeue(T * ret)
    {
        return detail::ringbuffer_internal<T>::dequeue(ret, array_.c_array(), max_size);
    }
};

template <typename T>
class ringbuffer<T, 0>:
    public detail::ringbuffer_internal<T>
{
    size_t max_size_;
    scoped_array<T> array_;

public:
    ringbuffer(size_t max_size):
        max_size_(max_size), array_(new T[max_size])
    {}

    bool enqueue(T const & t)
    {
        return detail::ringbuffer_internal<T>::enqueue(t, array_.get(), max_size_);
    }

    bool dequeue(T * ret)
    {
        return detail::ringbuffer_internal<T>::dequeue(ret, array_.get(), max_size_);
    }
};


} /* namespace lockfree */
} /* namespace boost */


#endif /* BOOST_LOCKFREE_RINGBUFFER_HPP_INCLUDED */
