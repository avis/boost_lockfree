/** \file boost/atomic.hpp */

/**
	\mainpage Atomic operations
	
	<UL>
		<LI>\ref memory_order "Memory order"</LI>
		<LI>\ref boost::atomic template class</LI>
	</UL>
*/

/**
	\page memory_order Memory order
	
	While a programmer may write operations accessing memory
	in a specific order (the <I>program order</I>), both compiler
	and processor may reorder operations rather arbitrarily.
	Consider the following popular example:
	
	\code
		class X;
		
		X * ring_buffer[16];
		unsigned int head, tail;
		
		// thread 1
		void add_element()
		{
			// prevent "overtaking" the head pointer
			unsigned int next=(head+1)&15;
			if (next==tail) return;
			ring_buffer[head]=new X;
			head=next;
		}
		
		// thread 2
		void remove_element()
		{
			unsigned int next=(tail+1)&15;
			if (head==tail) return;
			X * x=ring_buffer[tail];
			delete x;
			tail=next;
		}
	\endcode
	
	The programmer's intention is to create a new element,
	link it into the ring and afterwards increase the tail
	pointer. <B>This code is however incorrect</B> since
	the memory accesses may be reordered. (NB: Adding a
	@c volatile qualifier is <B>not</B> sufficient).
	
	The C++ draft standard for atomic operations allows to
	specify ordering constraints that restrict reordering
	of an atomic operation and other kinds of memory
	accesses (including other atomic operations). The above
	ring buffer implementation could correctly be written
	for example as:
	
	\code
		class X;
		
		X * ring_buffer[16];
		atomic<unsigned int> head, tail;
		
		// thread 1
		void add_element()
		{
			// prevent "overtaking" the head pointer
			unsigned int current=head.load(memory_order_relaxed);
			unsigned int next=(current+1)&15;
			if (next==tail.load(memory_order_acquire)) return;
			ring_buffer[current]=new X;
			head.store(next, memory_order_release);
		}
		
		// thread 2
		void remove_element()
		{
			unsigned int current=tail.load(memory_order_relaxed);
			unsigned int next=(current+1)&15;
			if (head.load(memory_order_acquire)==current) return;
			X * x=ring_buffer[current];
			tail.store(next, memory_order_release);
		}
	\endcode
	
	The possible constraints are:
	
	<UL>
		<LI> @c memory_order_relaxed: Atomic operation and other
		memory operations may be reordered freely. </LI>
		
		<LI> @c memory_order_acquire: Atomic operation must strictly
		precede all memory operations that follow in
		program order. Use this constraint if the thread
		is going to use a resource produced or released by another
		thread.</LI>
		
		<LI> @c memory_order_release: Atomic operation must strictly
		follow all memory operations that preced it in program order.
		Use this constraint if the thread produced or released a resource
		for use by another thread.
		</LI>
		
		<LI> @c memory_order_consume: Atomic operation must strictly
		precede all memory operations that follow in
		program order <I>and</I> are computationally dependent on the
		outcome of the operation. Use this constraint if the thread
		is going to dereference an atomic pointer written by another thread.
		</LI>
		
		<LI> @c memory_order_acq_rel: Combines
		@c memory_order_acquire and @c memory_order_release.
		</LI>
		
		<LI> @c memory_order_seq_cst: Requires the guarantee provided
		by @c memory_order_acq_rel and additionally ensures sequentially
		consistent execution (i.e. all threads in the system see the
		same order of modifications to all variables modified with
		@c memory_order_seq_cst constraint</LI>
	</UL>
	
	\section memory_order_use Use cases
	
	\subsection refcounters Reference counters
	
	When using an atomic variable for implementing a reference counter,
	its only purpose is to keep an accurate count of the number of pointers
	to this object, for example:
	
	\code
	class X {
	public:
		typedef boost::intrusive_ptr<X> pointer;
		X() : refcount(0) {}
		atomic<int> refcount;
	};
	
	void intrusive_ptr_add_ref(X *x)
	{
		x->refcount.atomic_add(1, memory_order_relaxed);
	}
	
	void intrusive_ptr_release(X *x)
	{
		if (x->refcount.atomic_sub(1, memory_order_relaxed)==1)
			delete x;
	}
	\endcode
	
	Since different threads may only access different <TT>X::pointer</TT> instances,
	<TT>memory_order_relaxed</TT> may be used here. (NB: when implementing
	weak_ptr, the 0->1 transition must use <TT>memory_order_acquire</TT>
	while the 1->0 transition must use <TT>memory_order_release</TT>).
	
	\subsection producer_consumer Producer/consumer
	
	When two threads use an atomic variable to transfer ownership 
	of objects, then the threads must use @c memory_order_release
	(in the thread relinquishing the object) and @c memory_order_acquire
	(in the thread taking owenrship of the object).
	
	\code
	class X;
	
	atomic<int> flag(0);
	X * instance(0);
	
	// thread 1
	void produce()
	{
		instance = new X;
		flag.store(1, memory_order_release);
	}
	
	// thread 2
	void consume()
	{
		if (flag.load(memory_order_acquire)==1) {
			X * x = instance;
			...
		}
	}
	\endcode
	
	\subsection pointer_publishing Object publishing
	
	In a few special cases, the "consumer" portion of
	the producer/consumer model described in the previous section
	can be implemented in a more light-weight fashion. One
	particularly important case is when
	
	- the producer generates an object and publishes through
	an atomic pointer update
	- the consumer loads the atomic pointer and wishes to
	dereference the pointer
	
	\code
	class X;
	
	atomic<X *> instance(0);
	
	// thread 1
	void produce()
	{
		X * x = new X;
		instance.store(x, memory_order_release);
	}
	
	// thread 2
	void consume()
	{
		X * x=instance.exchange(0, memory_order_consume);
		if (x) {
			// dereference x
		}
	}
	\endcode
	
	@c memory_order_consume is required on the consumer side
	(otherwise the memory accesses dereferencing the object
	might be scheduled before the pointer load).
	
	One special case is the venerable "double-check pattern":
	
	\code
	class X;
	
	X * instance()
	{
		static atomic<X *> singleton(0);
		static boost::mutex instantion_mutex;
		
		X * x = singleton.load(memory_order_consume);
		if (x) return x;
		
		boost::mutex::scoped_lock guard(instantiation_mutex);
		x=singleton.load(memory_order_consume);
		if (x) return x;
		x = new X;
		return x;
	}
	\endcode
	
*/

namespace boost {

/**
	\brief Memory ordering constraints
	
	This defines the relative order of one atomic operation
	and other memory operations (loads, stores, other atomic operations)
	executed by the same thread.
	
	The order of operations specified by the programmer in the
	source code ("program order") does not necessarily match
	the order in which they are actually executed on target system:
	Both compiler as well as processor may reorder operations
	quite arbitrarily. <B>Neglecting to specify any ordering
	constraint will therefore generally result in an incorrect program.</B>
*/
enum memory_order {
	/**
		\brief No constraint
		Atomic operation and other memory operations may be reordered freely.
	*/
	memory_order_relaxed,
	/**
		\brief Data dependence constraint
		Atomic operation must strictly precede any memory operation that
		computationally depends on the outcome of the atomic operation.
	*/
	memory_order_consume,
	/**
		\brief Acquire memory
		Atomic operation must strictly precede all memory operations that
		follow in program order.
	*/
	memory_order_acquire,
	/**
		\brief Release memory
		Atomic operation must strictly follow all memory operations that precede
		in program order.
	*/
	memory_order_release,
	/**
		\brief Acquire and release memory
		Combines the effects of \ref memory_order_acquire and \ref memory_order_release
	*/
	memory_order_acq_rel,
	/**
		\brief Sequentially consistent
		Produces the same result \ref memory_order_acq_rel, but additionally
		enforces globally sequential consistent execution
	*/
	memory_order_seq_cst
};

/**
	\brief Atomic datatype
	
	An atomic variable. Provides methods to modify this variable atomically.
	Valid template parameters are:
	
	- integral data types (char, short, int, ...)
	- pointer data types
	
	Unless specified otherwise, any memory ordering constraint can be used
	with any of the atomic operations.
*/
template<typename Type>
class atomic {
public:
	/**
		\brief Create uninitialized atomic variable
		Creates an atomic variable. Its initial value is undefined.
	*/
	atomic();
	/**
		\brief Create an initialize atomic variable
		\param value Initial value
		Creates and initializes an atomic variable.
	*/
	atomic(Type value);
	
	/**
		\brief Read the current value of the atomic variable
		\param order Memory ordering constraint, see \ref memory_order
		\return Current value of the variable
		
		Valid memory ordering constraints are:
		- memory_order_relaxed
		- memory_order_consume
		- memory_order_acquire
		- memory_order_seq_cst
	*/
	Type load(memory_order order=memory_order_seq_cst) const;
	
	/**
		\brief Write new value for atomic variable
		\param value New value
		\param order Memory ordering constraint, see \ref memory_order
		
		Valid memory ordering constraints are:
		- memory_order_relaxed
		- memory_order_release
		- memory_order_seq_cst
	*/
	void store(Type value, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically compare and exchange variable
		\param expected Expected old value
		\param desired Desired new value
		\param order Memory ordering constraint, see \ref memory_order
		\return @c true if value was changed
		
		Atomically performs the following operation
		
		\code
		if (variable==expected) {
			variable=desired;
			return true;
		} else {
			expected=variable;
			return false;
		}
		\endcode
		
		This operation may fail "spuriously", i.e. the state of the variable
		is unchanged even though the expected value was found (this is the
		case on architectures using "load-linked"/"store conditional" to
		implement the operation).
	*/
	bool compare_exchange_weak(Type &expected, desired, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically compare and exchange variable
		\param expected Expected old value
		\param desired Desired new value
		\param order Memory ordering constraint, see \ref memory_order
		\return @c true if value was changed
		
		Atomically performs the following operation
		
		\code
		if (variable==expected) {
			variable=desired;
			return true;
		} else {
			expected=variable;
			return false;
		}
		\endcode
		
		In contrast to \ref compare_exchange_weak, this operation will never
		fail spuriously. Since compare-and-swap must generally be retried
		in a loop, implementors are advised to prefer \ref compare_exchange_weak
		where feasible.
	*/
	bool compare_exchange_strong(Type &expected, desired, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically exchange variable
		\param value New value
		\param order Memory ordering constraint, see \ref memory_order
		\return Old value of the variable
		
		Atomically exchanges the value of the variable with the new
		value and returns its old value.
	*/
	Type exchange(Type value, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically add and return old value
		\param operand Operand
		\param order Memory ordering constraint, see \ref memory_order
		\return Old value of the variable
		
		Atomically adds operand to the variable and returns its
		old value.
	*/
	Type fetch_add(Type operand, memory_order order=memory_order_seq_cst);
	/**
		\brief Atomically subtract and return old value
		\param operand Operand
		\param order Memory ordering constraint, see \ref memory_order
		\return Old value of the variable
		
		Atomically subtracts operand from the variable and returns its
		old value.
	*/
	Type fetch_sub(Type operand, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically perform bitwise "AND" and return old value
		\param operand Operand
		\param order Memory ordering constraint, see \ref memory_order
		\return Old value of the variable
		
		Atomically performs bitwise "AND" with the variable and returns its
		old value.
		
		This method is available only if \c Type is an integral type.
	*/
	Type fetch_and(Type operand, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically perform bitwise "OR" and return old value
		\param operand Operand
		\param order Memory ordering constraint, see \ref memory_order
		\return Old value of the variable
		
		Atomically performs bitwise "OR" with the variable and returns its
		old value.
		
		This method is available only if \c Type is an integral type.
	*/
	Type fetch_or(Type operand, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically perform bitwise "XOR" and return old value
		\param operand Operand
		\param order Memory ordering constraint, see \ref memory_order
		\return Old value of the variable
		
		Atomically performs bitwise "XOR" with the variable and returns its
		old value.
		
		This method is available only if \c Type is an integral type.
	*/
	Type fetch_xor(Type operand, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Implicit load
		\return Current value of the variable
		
		The same as <tt>load(memory_order_seq_cst)</tt>. Avoid using
		the implicit conversion operator, use \ref load with
		an explicit memory ordering constraint.
	*/
	operator Type(void) const;
	/**
		\brief Implicit store
		\param value New value
		\return Copy of @c value
		
		The same as <tt>store(value, memory_order_seq_cst)</tt>. Avoid using
		the implicit conversion operator, use \ref store with
		an explicit memory ordering constraint.
	*/
	Type operator=(Type v);
	
	/**
		\brief Atomically perform bitwise "AND" and return new value
		\param operand Operand
		\return New value of the variable
		
		The same as <tt>fetch_and(operand, memory_order_seq_cst)&operand</tt>.
		Avoid using the implicit bitwise "AND" operator, use \ref fetch_and
		with an explicit memory ordering constraint.
	*/
	Type operator&=(Type operand);
	
	/**
		\brief Atomically perform bitwise "OR" and return new value
		\param operand Operand
		\return New value of the variable
		
		The same as <tt>fetch_or(operand, memory_order_seq_cst)|operand</tt>.
		Avoid using the implicit bitwise "OR" operator, use \ref fetch_or
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type.
	*/
	Type operator|=(Type operand);
	
	/**
		\brief Atomically perform bitwise "XOR" and return new value
		\param operand Operand
		\return New value of the variable
		
		The same as <tt>fetch_xor(operand, memory_order_seq_cst)^operand</tt>.
		Avoid using the implicit bitwise "XOR" operator, use \ref fetch_xor
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type.
	*/
	Type operator^=(Type operand);
	
	/**
		\brief Atomically add and return new value
		\param operand Operand
		\return New value of the variable
		
		The same as <tt>fetch_add(operand, memory_order_seq_cst)+operand</tt>.
		Avoid using the implicit add operator, use \ref fetch_add
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type.
	*/
	Type operator+=(Type operand);
	
	/**
		\brief Atomically subtract and return new value
		\param operand Operand
		\return New value of the variable
		
		The same as <tt>fetch_sub(operand, memory_order_seq_cst)-operand</tt>.
		Avoid using the implicit subtract operator, use \ref fetch_sub
		with an explicit memory ordering constraint.
	*/
	Type operator-=(Type operand);
	
	/**
		\brief Atomically increment and return new value
		\return New value of the variable
		
		The same as <tt>fetch_add(1, memory_order_seq_cst)+1</tt>.
		Avoid using the implicit increment operator, use \ref fetch_add
		with an explicit memory ordering constraint.
	*/
	Type operator++(void);
	/**
		\brief Atomically increment and return old value
		\return Old value of the variable
		
		The same as <tt>fetch_add(1, memory_order_seq_cst)</tt>.
		Avoid using the implicit increment operator, use \ref fetch_add
		with an explicit memory ordering constraint.
	*/
	Type operator++(int);
	/**
		\brief Atomically subtract and return new value
		\return New value of the variable
		
		The same as <tt>fetch_sub(1, memory_order_seq_cst)-1</tt>.
		Avoid using the implicit increment operator, use \ref fetch_sub
		with an explicit memory ordering constraint.
	*/
	Type operator--(void);
	/**
		\brief Atomically subtract and return old value
		\return Old value of the variable
		
		The same as <tt>fetch_sub(1, memory_order_seq_cst)</tt>.
		Avoid using the implicit increment operator, use \ref fetch_sub
		with an explicit memory ordering constraint.
	*/
	Type operator--(int);
	
private:
	/** \brief Deleted copy constructor */
	atomic(const atomic &);
	/** \brief Deleted copy assignment */
	void operator=(const atomic &);
};

}
