#include <boost/lockfree/detail/freelist.hpp>

#include <climits>
#define BOOST_TEST_MODULE lockfree_tests
#include <boost/test/included/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

#include <boost/type_traits/is_same.hpp>

#include <vector>


class dummy
{
    int foo[64];
};

template <typename freelist_type>
void run_test(void)
{
    freelist_type fl(1024);

    std::vector<dummy*> nodes;

    for (int i = 0; i != 128; ++i)
        nodes.push_back(fl.allocate());

    BOOST_FOREACH(dummy * d, nodes)
        fl.deallocate(d);

    nodes.clear();
    for (int i = 0; i != 128; ++i)
        nodes.push_back(fl.allocate());

    BOOST_FOREACH(dummy * d, nodes)
        fl.deallocate(d);

    for (int i = 0; i != 128; ++i)
        nodes.push_back(fl.allocate());
}

BOOST_AUTO_TEST_CASE( freelist_tests )
{
    run_test<boost::lockfree::caching_freelist<dummy> >();
    run_test<boost::lockfree::static_freelist<dummy> >();
}
/* using namespace boost; */
/* using namespace boost::mpl; */

/* BOOST_STATIC_ASSERT((is_same<lockfree::detail::select_freelist<int, std::allocator<int>, lockfree::caching_freelist_t>, */
/*                      lockfree::caching_freelist<int, std::allocator<int> > */
/*                      >::value)); */

/* BOOST_STATIC_ASSERT((is_same<lockfree::detail::select_freelist<int, std::allocator<int>, lockfree::static_freelist_t>, */
/*                      lockfree::static_freelist<int, std::allocator<int> > */
/*                      >::value)); */
