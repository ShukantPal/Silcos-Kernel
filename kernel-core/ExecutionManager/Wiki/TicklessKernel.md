# Introduction
A tickless kernel is one which does not use **regular timer interrupts** for scheduling and other purposes. The linux kernel does so only in special cases - like when the processor is idle. But here we want to make the kernel truly tickless - during all occasions. The silcos kernel does so by maintaining records of future timer interrupt requirements. This records may refer to -

1. A task that will wakeup in the future and cause the scheduler to dispatch.

2. The kernel will preempt the running task after its quanta finishes.

3. A driver will perform I/O related operation after some time.

## TimerManager
The execution manager module uses the _C++ class TimerManager_ to hold the timer & manually set its next interrupt. It is used for _registering interrupt-requests_ and after dispatching them to _delete records of past interrupts_. It dynamically manages these records on a _per-CPU_ basis. Unlike the linux-kernel, the TimerManager doesn't require the need to disable the _tick-less_ kernel at any time.
    
    // sample of code from ExecutionManager (kernel.silcos.exemgr)
    
    #include <Util/LinkedList.h> // we don't actually use the functions in LinkedList.h (due to sorting)
    
    namespace ExecutionManager
    { 
    
    enum EventMask
    {
    	HANDLER_EXECUTED = (1 << 0),
    	LISTENER_POOL_HELD = (1 << 1),
    	LISTENER_POOL_OPERATIVE = (1 << 2)
    };
    
    class TimerManager
    {
    public:
    	Event *createEvent(void (*kernHandler)(Time kernelCounter), unsigned long relativePriority);
    	void registerEvent(Time absoluteDispatchPoint, Event *handlerBlock);
    	void addListener(void (*kernListener)(Event*), Event *futureHolder);
    	void removeListener(void (*kernListener)(Event*), Event *listener);
    	
    	TimerManager();   	
    	struct Event;
    private:
    	struct Event
    	{
    		LinkedListNode pointNode;
    		void (*handler)(Time);
    		CircularList listeners;
    		unsigned long holderMask;
    	}
    };
    
    }
    
## Timer Events

The TimerManager uses a internal object **TimeManager::Event** which is abstract for external code. It can be referenced by the use of a pointer. These objects refer to events at various points in kernel time which have associated handlers. Other than one handler, they can have multiple listeners which are scheduled on a separate kernel-routine.

### Handler

The handler function executes locally in the **interrupt context**, and _may be null if not handler is specified_ and in that case _atleast one listener must be present_ otherwise the Event is obselete. While writing handler code for timer-events, care must be taken to ensure locking safety with other subsystems.

### Listener

A listener executes in the context of a non-preemptible **high-priority deferred interrupt kernel-routine**. This allows it to ensure no other task interrupts and **it must not lock any subsystem without checking**. That would result in total deadlock of the kernel including the scheduler. Note that memory can be allocated as interrupts are turned off while memory operations take place and therefore locking is for multi-processor access.