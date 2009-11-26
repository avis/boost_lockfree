#include <stdlib.h>

#include <boost/atomic.hpp>
#include <boost/atomic/detail/fallback.hpp>

namespace boost {
namespace detail {
namespace atomic {

static const size_t hash_size=16;
static boost::mutex fallback_locks[hash_size];
boost::mutex atomic_guard::seq_cst_lock;

boost::mutex &atomic_guard::get_lock_for_address(volatile const void * address)
{
	uintptr_t v=reinterpret_cast<uintptr_t>(const_cast<void *>(address));
	v^=v>>16;
	v^=v>>8;
	v^=v>>4;
	v=v%hash_size;
	return fallback_locks[v];
}

}
}
}
