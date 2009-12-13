/** \file boost/atomic.hpp */

/**
	\mainpage Atomic operations
	
	<UL>
		<LI>\ref memory_order "Memory order"</LI>
		<LI>\ref boost::atomic template class</LI>
		<LI>\ref architecture_support </LI>
		<LI>\ref faq "Frequently asked questions"</LI>
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
			unsigned int current=head.load(memory_order_relaxed);
			unsigned int next=(current+1)&15;
			// prevent "overtaking" the head pointer
			if (next==tail.load(memory_order_acquire)) return;
			ring_buffer[current]=new X;
			head.store(next, memory_order_release);
		}
		
		// thread 2
		void remove_element()
		{
			unsigned int current=tail.load(memory_order_relaxed);
			unsigned int next=(current+1)&15;
			if (current==head.load(memory_order_acquire)) return;
			X * x=ring_buffer[current];
			tail.store(next, memory_order_release);
		}
	\endcode
	
	The possible constraints are:
	
	<UL>
		<LI> @c memory_order_relaxed: Atomic operation and other
		memory operations may be reordered freely. </LI>
		
		<LI> @c memory_order_release: Atomic operation must strictly
		follow all memory operations that precede it in program order.
		Use this constraint if the thread wants to hand over
		a resource to another thread.
		</LI>
		
		<LI> @c memory_order_acquire: Atomic operation must strictly
		precede all memory operations that follow in
		program order. Use this constraint if the thread
		wants to receive an object handed over from another
		thread (via @c memory_order_release).
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

/**
	\page faq Frequently asked questions
	
	<h3>Why do you use inline assembler instead of compiler intrinsics such as <tt>__sync_val_compare_and_swap</tt>?</h3>
	
	The <tt>__sync_*</tt> family of gcc intrinsics implicitly act as memory
	fences (sometimes before the operations, sometimes after the operations,
	sometimes both). The memory fences are the expensive part of the atomic
	operation, and there are legitimate use cases for atomic operations
	using weaker forms of memory ordering than <tt>__sync_*</tt> impose.
	
*/

/**
	\page architecture_support Implementing support for new architectures
	
	\section template_organization Organization of class template layers
	
	The implementation uses multiple layers of template classes that
	inherit from the next lower level each and refine or adapt the respective
	underlying class:
	
	<UL>
		<LI>
			<code>boost::atomic&lt;T&gt;</code> is the topmost-level, providing 
			the external interface. Implementation-wise,
			it does not add anything (except for hiding copy constructor
			and assignment operator).
		</LI>
		<LI>
			<code>boost::detail::atomic::internal_atomic&lt;T,S=sizeof(T),I=is_integral_type&lt;T&gt; &gt;</code>.
			This layer is mainly responsible for providing the overloaded operators
			mapping to API member functions (e.g. <code>+=</code> to <code>fetch_add</code>).
			The defaulted template parameter <code>I</code> allows
			to expose the correct API functions (via partial template
			specialization): For non-integral types, it only
			publishes the various <code>exchange</code> functions
			as well as load and store, for integral types it
			additionally exports arithmetic and logic operations.
			
			Depending on whether the given type is integral, it
			inherits from either <code>boost::detail::atomic::platform_atomic&lt;T,S=sizeof(T)&gt;</code>
			or <code>boost::detail::atomic::platform_atomic_integral&lt;T,S=sizeof(T)&gt;</code>.
			There is however some special-casing: for non-integral types
			of size 1, 2, 4 or 8, it will coerce the datatype into an integer representation
			and delegate to <code>boost::detail::atomic::platform_atomic_integral&lt;T,S=sizeof(T)&gt;</code>
			-- the rationale is that platform implementors only need to provide
			integer-type operations.
		</LI>
		<LI>
			<code>boost::detail::atomic::platform_atomic_integral&lt;T,S=sizeof(T)&gt;</code>
			must provide the full set of operations for an integral type T
			(i.e. <code>load</code>, <code>store</code>, <code>exchange</code>,
			<code>compare_exchange_weak</code>, <code>compare_exchange_strong</code>,
			<code>fetch_add</code>, <code>fetch_sub</code>, <code>fetch_and</code>,
			<code>fetch_or</code>, <code>fetch_xor</code>, <code>is_lock_free</code>).
			The default implementation uses locking to emulate atomic operations, so
			this is the level at which implementors should provide template specializations
			to add support for platform-specific atomic operations.
			
			The two separate template parameters allow separate specialization
			on size and type (which, with fixed size, cannot
			specify more than signedness/unsignedness). The rationale is that
			most platform-specific atomic operations usually depend only on the
			operand size, so that common implementations for signed/unsigned
			types are possible. Signedness allows to properly to choose sign-extending
			instructions for the <code>load</code> operation, avoiding later
			conversion. The expectation is that in most implementations this will
			be a normal assignment in C, possibly accompanied by memory
			fences, so that the compiler can automatically choose the correct
			instruction.
		</LI>
		<LI>
			At the lowest level,
			<code>boost::detail::atomic::platform_atomic&lt;T,S=sizeof(T)&gt;</code>
			provides the most basic atomic operations (<code>load</code>, <code>store</code>,
			<code>exchange</code>, <code>compare_exchange_weak</code>,
			<code>compare_exchange_strong</code>) for arbitrarily generic data types.
			The default implementation uses locking as a fallback mechanism.
			Implementors generally do not have to specialize at this level
			(since these will not be used for the common integral type sizes
			of 1, 2, 4 and 8 bytes), but if s/he can if s/he so wishes to
			provide truly atomic operations for "odd" data type sizes.
			Some amount of care must be taken as the "raw" data type
			passed in from the user through <code>boost::atomic&lt;T&gt;</code>
			is visible here -- it thus needs to be type-punned or otherwise
			manipulated byte-by-byte to avoid using overloaded assigment,
			comparison operators and copy constructors.
		</LI>
	</UL>
	
	\section platform_atomic_implementation Implementing platform-specific atomic operations
	
	In principle implementors are responsible for providing the
	full range of named member functions of an atomic object
	(i.e. <code>load</code>, <code>store</code>, <code>exchange</code>,
	<code>compare_exchange_weak</code>, <code>compare_exchange_strong</code>,
	<code>fetch_add</code>, <code>fetch_sub</code>, <code>fetch_and</code>,
	<code>fetch_or</code>, <code>fetch_xor</code>, <code>is_lock_free</code>).
	These must be implemented as partial template specializations for
	<code>boost::detail::atomic::platform_atomic_integral&lt;T,S=sizeof(T)&gt;</code>:
	
	\code
		template<typename T>
		class platform_atomic_integral<T, 4>
		{
		public:
			explicit platform_atomic_integral(T v) : i(v) {}
			platform_atomic_integral(void) {}
			
			T load(memory_order order=memory_order_seq_cst) const volatile
			{
				// platform-specific code
			}
			void store(T v, memory_order order=memory_order_seq_cst) volatile
			{
				// platform-specific code
			}
			...
		private:
			volatile T i;
		};
	\endcode
	
	As noted above, it will usually suffice to specialize on the second
	template argument, indicating the size of the data type in bytes.
	
	Often only a portion of the required operations can be
	usefully mapped to machine instructions. Several helper template
	classes are provided that can automatically synthesize missing methods to
	complete an implementation.
	
	At the minimum, an implementor must provide the
	<code>load</code>, <code>store</code>,
	<code>compare_exchange_weak</code> and
	<code>is_lock_free</code> methods:
	
	\code
		template<typename T>
		class my_atomic_32 {
		public:
			my_atomic_32() {}
			my_atomic_32(T initial_value) : value(initial_value) {}
			
			T load(memory_order order=memory_order_seq_cst) volatile const
			{
				// platform-specific code
			}
			void store(T new_value, memory_order order=memory_order_seq_cst) volatile
			{
				// platform-specific code
			}
			bool compare_exchange_weak(T &expected, T desired,
				memory_order order=memory_order_seq_cst) volatile
			{
				// platform-specific code
			}
			bool is_lock_free() const volatile {return true;}
		protected:
			// typedef is required for classes inheriting from this
			typedef T integral_type;
		private:
			T value;
		};
	\endcode
	
	The template <code>boost::detail::atomic::build_atomic_from_minimal</code>
	can then take care of the rest:
	
	\code
		template<typename T>
		class platform_atomic_integral<T, 4>
			: public boost::detail::atomic::build_atomic_from_minimal<my_atomic_32<T> >
		{
		public:
			typedef build_atomic_from_minimal<my_atomic_32<T> > super;
			
			explicit platform_atomic_integral(T v) : super(v) {}
			platform_atomic_integral(void) {}
		};
	\endcode
	
	There are several helper classes to assist in building "complete"
	atomic implementations from different starting points:
	
	<UL>
		<LI>
			<code>build_atomic_from_minimal</code> requires
			<UL>
				<LI><code>load</code></LI>
				<LI><code>store</code></LI>
				<LI><code>compare_exchange_weak</code></LI>
			</UL>
		</LI>
		<LI>
			<code>build_atomic_from_exchange</code> requires
			<UL>
				<LI><code>load</code></LI>
				<LI><code>store</code></LI>
				<LI><code>compare_exchange_weak</code></LI>
				<LI><code>compare_exchange_strong</code></LI>
				<LI><code>exchange</code></LI>
			</UL>
		</LI>
		<LI>
			<code>build_atomic_from_add</code> requires
			<UL>
				<LI><code>load</code></LI>
				<LI><code>store</code></LI>
				<LI><code>compare_exchange_weak</code></LI>
				<LI><code>compare_exchange_strong</code></LI>
				<LI><code>exchange</code></LI>
				<LI><code>fetch_add</code></LI>
			</UL>
		</LI>
		<LI>
			<code>build_atomic_from_typical</code> (<I>supported on gcc only</I>) requires
			<UL>
				<LI><code>load</code></LI>
				<LI><code>store</code></LI>
				<LI><code>compare_exchange_weak</code></LI>
				<LI><code>compare_exchange_strong</code></LI>
				<LI><code>exchange</code></LI>
				<LI><code>fetch_add_var</code> (protected method)</LI>
				<LI><code>fetch_inc</code> (protected method)</LI>
				<LI><code>fetch_dec</code> (protected method)</LI>
			</UL>
			This will generate a <code>fetch_add</code> method
			that calls <code>fetch_inc</code>/<code>fetch_dec</code>
			when the given parameter is a compile-time constant
			equal to +1 or -1 respectively, and <code>fetch_add_var</code>
			in all other cases. This provides a mechanism for
			optimizing the extremely common case of an atomic
			variable being used as a counter.
			
			The prototypes for these methods to be implemented is:
			\code
				template<typename T>
				class my_atomic {
				public:
					T fetch_inc(memory_order order) volatile;
					T fetch_dec(memory_order order) volatile;
					T fetch_add_var(T counter, memory_order order) volatile;
				};
			\endcode
		</LI>
	</UL>
	
	There is one other helper template that can build sub-word-sized
	atomic data types even though the underlying architecture allows
	only word-sized atomic operations:
	
	\code
		template<typename T>
		class platform_atomic_integral<T, 1> :
			public build_atomic_from_larger_type<my_atomic_32<uint32_t>, T>
		{
		public:
			typedef build_atomic_from_larger_type<my_atomic_32<uint32_t>, T> super;
			
			explicit platform_atomic_integral(T v) : super(v) {}
			platform_atomic_integral(void) {}
		};
	\endcode
	
	The above would create an atomic data type of 1 byte size, and
	use masking and shifts to map it to 32-bit atomic operations.
	The base type must implement <code>load</code>, <code>store</code>
	and <code>compare_exchange_weak</code> for this to work.
	
	These helper templates are defined in <code>boost/atomic/detail/builder.hpp</code>.
	In unusual circumstances, an implementor may also opt to specialize
	<code>public boost::detail::atomic::platform_atomic&lt;T,S=sizeof(T)&gt;</code>
	to provide support for atomic objects not fitting an integral size.
	If you do that, keep the following things in mind:
	
	<UL>
		<LI>
			There is no reason to ever do this for object sizes
			of 1, 2, 4 and 8
		</LI>
		<LI>
			Only the methods
			<LI><code>load</code></LI>
			<LI><code>store</code></LI>
			<LI><code>compare_exchange_weak</code></LI>
			<LI><code>compare_exchange_strong</code></LI>
			<LI><code>exchange</code></LI>
			need to be implemented.
		</LI>
		<LI>
			The type of the data to be stored in the atomic
			variable (template parameter <code>T</code>)
			is exposed to this class, and the type may have
			overloaded assignment and comparison operators --
			using these overloaded operators however will result
			in an error. The implementor is responsible for
			accessing the objects in a way that does not
			invoke either of these operators (using e.g.
			<code>memcpy</code> or type-casts).
		</LI>
	</UL>
	
	\section platform_atomic_implementation Putting it altogether
	
	The template specializations should be put into a header file
	in the <code>boost/atomic/detail</code> directory, preferrably
	specifying supported compiler and architecture in its name.
	
	The file <code>boost/atomic/platform.hpp</code> must
	subsequently be modified to conditionally include the new
	header.
	
*/

namespace boost {

/**
	\brief Memory ordering constraints
	
	This defines the relative order of one atomic operation
	and other memory operations (loads, stores, other atomic operations)
	executed by the same thread.
	
	The order of operations specified by the programmer in the
	source code ("program order") does not necessarily match
	the order in which they are actually executed on the target system:
	Both compiler as well as processor may reorder operations
	quite arbitrarily. <B>Specifying the wrong ordering
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
	- any other data type that has a non-throwing default
	  constructor and that can be copied via <TT>memcpy</TT>
	
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
		- @c memory_order_relaxed
		- @c memory_order_consume
		- @c memory_order_acquire
		- @c memory_order_seq_cst
	*/
	Type load(memory_order order=memory_order_seq_cst) const;
	
	/**
		\brief Write new value to atomic variable
		\param value New value
		\param order Memory ordering constraint, see \ref memory_order
		
		Valid memory ordering constraints are:
		- @c memory_order_relaxed
		- @c memory_order_release
		- @c memory_order_seq_cst
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
		
		The established memory order will be @c order if the operation
		is successful. If the operation is unsuccesful, the
		memory order will be
		
		- @c memory_order_relaxed if @c order is @c memory_order_acquire ,
		  @c memory_order_relaxed or @c memory_order_consume
		- @c memory_order_release if @c order is @c memory_order_acq_release
		  or @c memory_order_release
		- @c memory_order_seq_cst if @c order is @c memory_order_seq_cst
	*/
	bool compare_exchange_weak(
		Type &expected,
		Type desired,
		memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically compare and exchange variable
		\param expected Expected old value
		\param desired Desired new value
		\param success_order Memory ordering constraint if operation
		is successful
		\param failure_order Memory ordering constraint if operation is unsuccesful
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
		
		The constraint imposed by @c success_order may not be
		weaker than the constraint imposed by @c failure_order.
	*/
	bool compare_exchange_weak(
		Type &expected,
		Type desired,
		memory_order success_order,
		memory_order failure_order);
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
		
		The established memory order will be @c order if the operation
		is successful. If the operation is unsuccesful, the
		memory order will be
		
		- @c memory_order_relaxed if @c order is @c memory_order_acquire ,
		  @c memory_order_relaxed or @c memory_order_consume
		- @c memory_order_release if @c order is @c memory_order_acq_release
		  or @c memory_order_release
		- @c memory_order_seq_cst if @c order is @c memory_order_seq_cst
	*/
	bool compare_exchange_strong(
		Type &expected,
		Type desired,
		memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically compare and exchange variable
		\param expected Expected old value
		\param desired Desired new value
		\param success_order Memory ordering constraint if operation
		is successful
		\param failure_order Memory ordering constraint if operation is unsuccesful
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
		
		The constraint imposed by @c success_order may not be
		weaker than the constraint imposed by @c failure_order.
	*/
	bool compare_exchange_strong(
		Type &expected,
		Type desired,
		memory_order success_order,
		memory_order failure_order);
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
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		@c operand is of type @c ptrdiff_t and the operation
		is performed following the rules for pointer arithmetic
		in C++.
	*/
	Type fetch_sub(Type operand, memory_order order=memory_order_seq_cst);
	
	/**
		\brief Atomically perform bitwise "AND" and return old value
		\param operand Operand
		\param order Memory ordering constraint, see \ref memory_order
		\return Old value of the variable
		
		Atomically performs bitwise "AND" with the variable and returns its
		old value.
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		@c operand is of type @c ptrdiff_t and the operation
		is performed following the rules for pointer arithmetic
		in C++.
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
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		@c operand is of type @c ptrdiff_t and the operation
		is performed following the rules for pointer arithmetic
		in C++.
	*/
	Type operator+=(Type operand);
	
	/**
		\brief Atomically subtract and return new value
		\param operand Operand
		\return New value of the variable
		
		The same as <tt>fetch_sub(operand, memory_order_seq_cst)-operand</tt>.
		Avoid using the implicit subtract operator, use \ref fetch_sub
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		@c operand is of type @c ptrdiff_t and the operation
		is performed following the rules for pointer arithmetic
		in C++.
	*/
	Type operator-=(Type operand);
	
	/**
		\brief Atomically increment and return new value
		\return New value of the variable
		
		The same as <tt>fetch_add(1, memory_order_seq_cst)+1</tt>.
		Avoid using the implicit increment operator, use \ref fetch_add
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		the operation
		is performed following the rules for pointer arithmetic
		in C++.
	*/
	Type operator++(void);
	/**
		\brief Atomically increment and return old value
		\return Old value of the variable
		
		The same as <tt>fetch_add(1, memory_order_seq_cst)</tt>.
		Avoid using the implicit increment operator, use \ref fetch_add
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		the operation
		is performed following the rules for pointer arithmetic
		in C++.
	*/
	Type operator++(int);
	/**
		\brief Atomically subtract and return new value
		\return New value of the variable
		
		The same as <tt>fetch_sub(1, memory_order_seq_cst)-1</tt>.
		Avoid using the implicit increment operator, use \ref fetch_sub
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		the operation
		is performed following the rules for pointer arithmetic
		in C++.
	*/
	Type operator--(void);
	/**
		\brief Atomically subtract and return old value
		\return Old value of the variable
		
		The same as <tt>fetch_sub(1, memory_order_seq_cst)</tt>.
		Avoid using the implicit increment operator, use \ref fetch_sub
		with an explicit memory ordering constraint.
		
		This method is available only if \c Type is an integral type
		or a non-void pointer type. If it is a pointer type,
		the operation
		is performed following the rules for pointer arithmetic
		in C++.
	*/
	Type operator--(int);
	
private:
	/** \brief Deleted copy constructor */
	atomic(const atomic &);
	/** \brief Deleted copy assignment */
	void operator=(const atomic &);
};

}
