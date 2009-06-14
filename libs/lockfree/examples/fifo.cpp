//  Copyright (C) 2009 Tim Blechmann
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/thread/thread.hpp>
#include <boost/lockfree/atomic_int.hpp>
#include <boost/lockfree/fifo.hpp>
#include <iostream>

boost::lockfree::atomic_int<int> producer_count(0);
boost::lockfree::atomic_int<int> consumer_count(0);

boost::lockfree::fifo<int> fifo;

const int iterations = 1000000;

void producer(void)
{
    for (int i = 0; i != iterations; ++i) {
        int value = ++producer_count;
        fifo.enqueue(value);
    }
}

void consumer(void)
{
    int value;
    while (producer_count != 2*iterations) {
        while (fifo.dequeue(&value))
            ++consumer_count;
    }

    while (fifo.dequeue(&value))
        ++consumer_count;

}

int main(int argc, char* argv[])
{
    boost::thread thrd_p1(producer);
    boost::thread thrd_p2(producer);
    boost::thread thrd_c1(consumer);
    boost::thread thrd_c2(consumer);

    thrd_p1.join();
    thrd_p2.join();
    thrd_c1.join();
    thrd_c2.join();

    std::cout << "produced " << producer_count << " objects." << std::endl;
    std::cout << "consumed " << consumer_count << " objects." << std::endl;
}
