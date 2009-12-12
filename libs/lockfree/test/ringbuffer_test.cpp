#include <boost/lockfree/ringbuffer.hpp>

#include <climits>
#define BOOST_TEST_MODULE lockfree_tests
#include <boost/test/included/unit_test.hpp>


#include <boost/thread.hpp>
#include <iostream>
#include <memory>


#include "test_helpers.hpp"

using namespace boost;
using namespace boost::lockfree;
using namespace std;


BOOST_AUTO_TEST_CASE( simple_ringbuffer_test )
{
    ringbuffer<int, 64> f;

    BOOST_REQUIRE(f.empty());
    f.enqueue(1);
    f.enqueue(2);

    int i1(0), i2(0);

    BOOST_REQUIRE(f.dequeue(&i1));
    BOOST_REQUIRE_EQUAL(i1, 1);

    BOOST_REQUIRE(f.dequeue(&i2));
    BOOST_REQUIRE_EQUAL(i2, 2);
    BOOST_REQUIRE(f.empty());
}


struct ringbuffer_tester
{
    ringbuffer<int, 128> sf;

    atomic<long> ringbuffer_cnt, received_nodes;

    static_hashed_set<int, 1<<16 > working_set;

    static const uint nodes_per_thread = 20000000;

    static const int reader_threads = 1;
    static const int writer_threads = 1;

    ringbuffer_tester(void):
        ringbuffer_cnt(0), received_nodes(0)
    {}

    void add(void)
    {
        for (uint i = 0; i != nodes_per_thread; ++i)
        {
            while(ringbuffer_cnt > 10000)
                thread::yield();

            int id = generate_id<int>();

            working_set.insert(id);

            while (sf.enqueue(id) == false)
            {
                thread::yield();
            }

            ++ringbuffer_cnt;
        }
    }

    bool get_element(void)
    {
        int data;

        bool success = sf.dequeue(&data);

        if (success)
        {
            ++received_nodes;
            --ringbuffer_cnt;
            bool erased = working_set.erase(data);
            assert(erased);
            return true;
        }
        else
            return false;
    }

    volatile bool running;

    void get(void)
    {
        for(;;)
        {
            bool success = get_element();
            if (not running and not success)
                return;
            if (not success)
                thread::yield();
        }
    }

    void run(void)
    {
        running = true;

        thread_group writer;
        thread_group reader;

        BOOST_REQUIRE(sf.empty());
        for (int i = 0; i != reader_threads; ++i)
            reader.create_thread(boost::bind(&ringbuffer_tester::get, this));

        for (int i = 0; i != writer_threads; ++i)
            writer.create_thread(boost::bind(&ringbuffer_tester::add, this));
        cout << "reader and writer threads created" << endl;

        writer.join_all();
        cout << "writer threads joined. waiting for readers to finish" << endl;

        running = false;
        reader.join_all();

        BOOST_REQUIRE_EQUAL(received_nodes, writer_threads * nodes_per_thread);
        BOOST_REQUIRE_EQUAL(ringbuffer_cnt, 0);
        BOOST_REQUIRE(sf.empty());
        BOOST_REQUIRE(working_set.count_nodes() == 0);
    }
};



BOOST_AUTO_TEST_CASE( ringbuffer_test_caching )
{
    ringbuffer_tester test1;
    test1.run();
}