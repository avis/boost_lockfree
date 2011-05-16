#include "../../../boost/lockfree/fifo.hpp"
#include "../../../boost/lockfree/stack.hpp"

int elements = 16384 * 4;
int iterations = 50;

#define FIFO \
    boost::lockfree::fifo<long> f(elements);

#define FIFO_FULL \
    boost::lockfree::fifo<long> f(elements); \
    for (int i = 0; i != elements; ++i) \
        f.enqueue_unsafe(i); \

#define STACK \
    boost::lockfree::stack<long> f(elements);

#define STACK_FULL \
    boost::lockfree::stack<long> f(elements); \
    for (int i = 0; i != elements; ++i) \
        f.push_unsafe(i); \

__attribute__ ((noinline))
__attribute__ ((flatten))
void test_fifo_push(void)
{
    FIFO
    for (int i = 0; i != elements; ++i)
		f.enqueue(i);
}

__attribute__ ((noinline))
__attribute__ ((flatten))
void test_fifo_push_unsafe(void)
{
    FIFO
    for (int i = 0; i != elements; ++i)
		f.enqueue_unsafe(i);
}

__attribute__ ((noinline))
__attribute__ ((flatten))
void test_stack_push(void)
{
    STACK
	for (int i = 0; i != elements; ++i)
		f.push(i);
}

__attribute__ ((noinline))
__attribute__ ((flatten))
void test_stack_push_unsafe(void)
{
    STACK
	for (int i = 0; i != elements; ++i)
		f.push_unsafe(i);
}


__attribute__ ((noinline))
__attribute__ ((flatten))
void test_fifo_pop(void)
{
    FIFO_FULL
    long out;
    for (int i = 0; i != elements; ++i)
        f.dequeue(&out);
}

__attribute__ ((noinline))
__attribute__ ((flatten))
void test_fifo_pop_unsafe(void)
{
    FIFO_FULL
    long out;
    for (int i = 0; i != elements; ++i)
        f.dequeue_unsafe(&out);
}

__attribute__ ((noinline))
__attribute__ ((flatten))
void test_stack_pop(void)
{
    STACK_FULL
    long out;
    for (int i = 0; i != elements; ++i)
        f.pop(&out);
}

__attribute__ ((noinline))
__attribute__ ((flatten))
void test_stack_pop_unsafe(void)
{
    STACK_FULL
    long out;
    for (int i = 0; i != elements; ++i)
        f.pop_unsafe(&out);
}

int main()
{
    for (int i = 0; i != iterations; ++i)
        test_fifo_push();

    for (int i = 0; i != iterations; ++i)
        test_fifo_push_unsafe();

    for (int i = 0; i != iterations; ++i)
        test_fifo_pop();

    for (int i = 0; i != iterations; ++i)
        test_fifo_pop_unsafe();

    for (int i = 0; i != iterations; ++i)
        test_stack_push();

    for (int i = 0; i != iterations; ++i)
        test_stack_push_unsafe();

    for (int i = 0; i != iterations; ++i)
        test_stack_pop();

    for (int i = 0; i != iterations; ++i)
        test_stack_pop_unsafe();
}
