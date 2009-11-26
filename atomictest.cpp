#include <boost/atomic.hpp>
#include <stdio.h>

#include <assert.h>

using namespace boost;

template<typename T>
void test_atomic(void)
{
	atomic<T> i(41);
	T n;
	assert(sizeof(i)>=sizeof(n));
	
	bool success;
	
	n=i++;
	assert(i==42);
	assert(n==41);
	
	n=i--;
	assert(n==42);
	assert(i==41);
	
	n=++i;
	assert(i==42);
	assert(n==42);
	
	n=--i;
	assert(n==41);
	assert(i==41);
	
	n=i.fetch_and(15);
	assert(n==41);
	assert(i==9);
	
	n=i.fetch_or(17);
	assert(n==9);
	assert(i==25);
	
	n=i.fetch_xor(3);
	assert(n==25);
	assert(i==26);
	
	n=i.exchange(12);
	assert(n==26);
	assert(i==12);
	
	n=12;
	success=i.compare_exchange_strong(n, 17);
	assert(success);
	assert(n==12);
	assert(i==17);
	
	n=12;
	success=i.compare_exchange_strong(n, 19);
	assert(!success);
	assert(n==17);
	assert(i==17);
}

int main()
{
	test_atomic<char>();
	test_atomic<signed char>();
	test_atomic<unsigned char>();
	test_atomic<short>();
	test_atomic<unsigned short>();
	test_atomic<int>();
	test_atomic<unsigned int>();
	test_atomic<long>();
	test_atomic<unsigned long>();
	test_atomic<long long>();
	test_atomic<unsigned long long>();
	
	atomic<int *> i;
	int *p;
	bool success;
	
	i=0;
	assert(i==0);
	
	p=(int *)40;
	success=i.compare_exchange_strong(p, (int *)44);
	assert(!success);
	assert(p==0);
	assert(i==0);
	
	p=(int *)0;
	success=i.compare_exchange_strong(p, (int *)44);
	assert(success);
	assert(p==0);
	assert(i==(int *)44);
	
	p=i.exchange((int *)20);
	assert(p==(int *)44);
	assert(i==(int *)20);
}
