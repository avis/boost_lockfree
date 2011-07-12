#include <boost/lockfree/detail/freelist.hpp>
#include <boost/thread.hpp>

#include <climits>
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

#include <boost/type_traits/is_same.hpp>

#include <vector>


class dummy
{
    int foo[64];
};

template <typename freelist_type,
          bool threadsafe>
void run_test(void)
{
    freelist_type fl(8);

    std::vector<dummy*> nodes;

    for (int i = 0; i != 4; ++i)
        nodes.push_back(threadsafe ? fl.allocate()
                                   : fl.allocate_unsafe());

    BOOST_FOREACH(dummy * d, nodes)
        if (threadsafe)
            fl.deallocate(d);
        else
            fl.deallocate_unsafe(d);

    nodes.clear();
    for (int i = 0; i != 4; ++i)
        nodes.push_back(threadsafe ? fl.allocate()
                                   : fl.allocate_unsafe());

    BOOST_FOREACH(dummy * d, nodes)
        if (threadsafe)
            fl.deallocate(d);
        else
            fl.deallocate_unsafe(d);

    for (int i = 0; i != 4; ++i)
        nodes.push_back(threadsafe ? fl.allocate()
                                   : fl.allocate_unsafe());
}

BOOST_AUTO_TEST_CASE( freelist_tests )
{
    run_test<boost::lockfree::detail::freelist_stack<dummy, true>, true >();
    run_test<boost::lockfree::detail::freelist_stack<dummy, false>, true >();
    run_test<boost::lockfree::detail::freelist_stack<dummy, true>, false >();
    run_test<boost::lockfree::detail::freelist_stack<dummy, false>, false >();
}

template <typename freelist_type>
struct freelist_tester
{
    static const int max_nodes = 1024;
    static const int thread_count = 4;
    static const int loops_per_thread = 1024;

    boost::lockfree::detail::atomic<int> free_nodes;
    boost::thread_group threads;

    freelist_type fl;

    freelist_tester(void):
        free_nodes(0), fl(max_nodes * thread_count)
    {
        for (int i = 0; i != thread_count; ++i)
            threads.create_thread(boost::bind(&freelist_tester::run, this));
        threads.join_all();
    }

    void run(void)
    {
        std::vector<dummy*> nodes;
        nodes.reserve(max_nodes);

        for (int i = 0; i != loops_per_thread; ++i) {
            while (nodes.size() < max_nodes) {
                dummy * node = fl.allocate();
                if (node == NULL)
                    break;
                nodes.push_back(node);
            }

            while (!nodes.empty()) {
                dummy * node = nodes.back();
                assert(node);
                nodes.pop_back();
                fl.deallocate(node);
            }
        }
        BOOST_REQUIRE(nodes.empty());
    }
};

BOOST_AUTO_TEST_CASE( caching_freelist_test )
{
    freelist_tester<boost::lockfree::detail::freelist_stack<dummy, true> > tester();
}

BOOST_AUTO_TEST_CASE( static_freelist_test )
{
    freelist_tester<boost::lockfree::detail::freelist_stack<dummy, true> > tester();
}
