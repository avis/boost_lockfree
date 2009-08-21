#include <boost/lockfree/detail/cas.hpp>

#include <climits>
#define BOOST_TEST_MODULE lockfree_tests
#include <boost/test/included/unit_test.hpp>

#include <cstddef>

struct cas2_tst
{
    long i;
    long j;
};

struct cas2_tst2
{
    void* i;
    long j;
};

using namespace boost::lockfree;


BOOST_AUTO_TEST_CASE( cas_test )
{
    {
        int i = 1;

        BOOST_REQUIRE_EQUAL (cas(&i, 1, 3), true);
        BOOST_REQUIRE_EQUAL (i, 3);

        BOOST_REQUIRE_EQUAL (cas(&i, 1, 3), false);
        BOOST_REQUIRE_EQUAL (i, 3);
    }

}
