![License MIT](https://img.shields.io/github/license/BehaviorTree/BehaviorTree.CPP?color=blue)
[![conan Ubuntu](https://github.com/endurodave/AsyncStateMachine/actions/workflows/cmake_ubuntu.yml/badge.svg)](https://github.com/endurodave/AsyncStateMachine/actions/workflows/cmake_ubuntu.yml)
[![conan Ubuntu](https://github.com/endurodave/AsyncStateMachine/actions/workflows/cmake_clang.yml/badge.svg)](https://github.com/endurodave/AsyncStateMachine/actions/workflows/cmake_clang.yml)
[![conan Windows](https://github.com/endurodave/AsyncStateMachine/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/endurodave/AsyncStateMachine/actions/workflows/cmake_windows.yml)

# Asynchronous State Machine Design in C++

An asynchronous C++ state machine implemented using an asynchronous delegate library.

# Table of Contents
- [Asynchronous State Machine Design in C++](#asynchronous-state-machine-design-in-c)
- [Table of Contents](#table-of-contents)
- [Introduction](#introduction)
  - [References](#references)
- [CMake Build](#cmake-build)
  - [Windows Visual Studio](#windows-visual-studio)
  - [Linux Make](#linux-make)
- [Asynchronous Delegates](#asynchronous-delegates)
- [AsyncStateMachine](#asyncstatemachine)
- [Motor Example](#motor-example)
- [Self-Test Subsystem Example](#self-test-subsystem-example)
  - [SelfTestEngine](#selftestengine)
  - [CentrifugeTest](#centrifugetest)
  - [Timer](#timer)
  - [Run-Time](#run-time)
- [Conclusion](#conclusion)

# Introduction

A software-based Finite State Machines (FSM) is an implementation method used to decompose a design into states and events. Simple embedded devices with no operating system employ single threading such that the state machines run on a single “thread”. More complex systems use multithreading to divvy up the processing.

This repository covers how to use a C++ state machines within the context of a multithreaded environment. A new `AsyncStateMachine` class extends the functionality of `StateMachine` documented in [State Machine Design in C++](https://github.com/endurodave/StateMachine). The asynchronous state machine utilizes a C++ `std::thread` for thread support as covered within [C++ std::thread Event Loop](https://github.com/endurodave/StdWorkerThread). Any OS thread is easy adapted. The `std::thread` was convenient for cross-platform Windows and Linux usage.

The `AsyncStateMachine` leverages the asynchronous delegate library documented in [Asynchronous Multicast Delegates in C++](https://github.com/endurodave/DelegateMQ). In short, the library is capable of targeting any callable function synchronously or asynchronously.

The goal for the article is to provide a complete working project with threads, timers, events, and state machines all working together. To illustrate the concept, the example project implements a state-based self-test engine utilizing asynchronous communication between state machines.

The state machine and delegate implementations will not be reexplained here. This article focuses on the `AsyncStateMachine` enhancement and integration with the `DelegateMQ` library.

CMake is used to create the build files. CMake is free and open-source software. Windows, Linux and other toolchains are supported. See the `CMakeLists.txt` file for more information.

## References

* [State Machine Design in C++](https://github.com/endurodave/StateMachine) - A compact C++ finite state machine (FSM) implementation.

* [Asynchronous Multicast Delegates in C++](https://github.com/endurodave/AsyncMulticastDelegateModern) - A C++ delegate library capable of targeting any callable function synchronously or asynchronously.

* [C++ std::thread Event Loop](https://github.com/endurodave/StdWorkerThread) - A worker thread using the C++ thread support library.

# CMake Build
[CMake](https://cmake.org/) is used to create the project build files. See `CMakeLists.txt` for more information.

## Windows Visual Studio

`cmake -G "Visual Studio 17 2022" -A Win32 -B Build -S .`

## Linux Make 

`cmake -G "Unix Makefiles" -B Build -S .`

# Asynchronous Delegates

If you’re not familiar with a delegate, the concept is quite simple. A delegate can be thought of as a super function pointer. In C++, there's no pointer type capable of pointing to all the possible function variations: instance member, virtual, const, static, lambda, and free (global). A function pointer can’t point to instance member functions, and pointers to member functions have all sorts of limitations. However, delegate classes can, in a type-safe way, point to any function provided the function signature matches. In short, a delegate points to any function with a matching signature to support anonymous function invocation.

Asynchronous delegates take the concept further and permits anonymous invocation of any function on a client specified thread of control. The function and all arguments are safely called from a destination thread simplifying inter-thread communication and eliminating cross-threading errors. The repository [Asynchronous Multicast Delegates in C++](https://github.com/endurodave/AsyncMulticastDelegateModern) covers usage patterns in detail.

The `AsyncStateMachine` uses asynchronous delegates to inject external events into a state machine instance.

# AsyncStateMachine

The `AsyncStateMachine` inherits from `StateMachine`. Create an state machine thread using `CreateThread()` or alternatively attach an existing thread using `SetThread()`. 

```cpp
class AsyncStateMachine : public StateMachine
{
public:
    ///	Constructor.
    ///	@param[in] maxStates - the maximum number of state machine states.
    AsyncStateMachine(BYTE maxStates, BYTE initialState = 0);

    /// Destructor
    virtual ~AsyncStateMachine();

    /// Create a new thread for this state machine
    /// @param[in] threadName - the thread name
    void CreateThread(const std::string& threadName);

    /// Set a thread for this state machine
    /// @param[in] thread - a Thread instance
    void SetThread(std::shared_ptr<Thread> thread) { m_thread = thread;  }

    /// Get the thread attached to this state machine
    /// @return A Thread instance
    std::shared_ptr<Thread> GetThread() { return m_thread; }

protected:
    /// @see StateMachine::ExternalEvent()
    void ExternalEvent(BYTE newState, const EventData* pData = NULL);

private:
    // The worker thread instance the state machine executes on
    std::shared_ptr<Thread> m_thread = nullptr;
};
```

Converting a state machine from `StateMachine` to `AsyncStateMachine` requires these changes:

1. Inherit from `AsyncStateMachine`.
2. Add `ASYNC_INVOKE()` to all external event functions.
3. Call `CreateThread()` or `SetThread()` to attach a thread.

# Motor Example

The `Motor` state machine diagram is shown below.

![Motor State Machine](Motor.png)

`Motor` inherits from `AsyncStateMachine` and implements two external events and four states.

```cpp
#ifndef _MOTOR_H
#define _MOTOR_H

#include "AsyncStateMachine.h"

class MotorData : public EventData
{
public:
	INT speed;
};

// Motor is an asynchronous state machine. All external events are executed
// on the Motor thread of control. The DelegateMQ library handles invoking 
// functions asynchronously.
class Motor : public AsyncStateMachine
{
public:
	Motor();

	// External events taken by this state machine
	void SetSpeed(MotorData* data);
	void Halt();

private:
	INT m_currentSpeed; 

	// State enumeration order must match the order of state method entries
	// in the state map.
	enum States
	{
		ST_IDLE,
		ST_STOP,
		ST_START,
		ST_CHANGE_SPEED,
		ST_MAX_STATES
	};

	// Define the state machine state functions with event data type
	STATE_DECLARE(Motor, 	Idle,			NoEventData)
	STATE_DECLARE(Motor, 	Stop,			NoEventData)
	STATE_DECLARE(Motor, 	Start,			MotorData)
	STATE_DECLARE(Motor, 	ChangeSpeed,	MotorData)

	// State map to define state object order. Each state map entry defines a
	// state object.
	BEGIN_STATE_MAP
		STATE_MAP_ENTRY(&Idle)
		STATE_MAP_ENTRY(&Stop)
		STATE_MAP_ENTRY(&Start)
		STATE_MAP_ENTRY(&ChangeSpeed)
	END_STATE_MAP	
};

#endif
```

The `Motor::SetSpeed()` external event uses the `ASYNC_INVOKE()` macro to invoke the function on the `Motor` thread of control using the `DelegateMQ` library. `ASYNC_INVOKE()` is non-blocking and inserts a message into the `Motor` thread's event queue with pointers to the function and argument, then early returns. Later, the `Motor` thread dequeues the message and calls `Motor::SetSpeed()` on `Motor`'s thread context. The `SetSpeed()` external event function is thread-safe callable by anyone.

```cpp
void Motor::SetSpeed(MotorData* data)
{
	/* ASYNC_INVOKE below effectively executes the following code:
	// Is this function call executing on this state machine thread?
	if (GetThreadId() != Thread::GetCurrentThreadId())
	{
		// Asynchronously re-invoke the SetSpeed() event on Motor's thread
		AsyncInvoke(this, &Motor::SetSpeed, *GetThread(), data);
		return;
	}*/

	// Asynchronously invoke Motor::SetSpeed on the Motor thread of control
	ASYNC_INVOKE(Motor, SetSpeed, data);

	BEGIN_TRANSITION_MAP			              			// - Current State -
		TRANSITION_MAP_ENTRY (ST_START)						// ST_IDLE
		TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)				// ST_STOP
		TRANSITION_MAP_ENTRY (ST_CHANGE_SPEED)				// ST_START
		TRANSITION_MAP_ENTRY (ST_CHANGE_SPEED)				// ST_CHANGE_SPEED
	END_TRANSITION_MAP(data)
}
```

The `Motor` constructor creates the thread at runtime.

```cpp
Motor::Motor() :
	AsyncStateMachine(ST_MAX_STATES),
	m_currentSpeed(0)
{
	CreateThread("Motor");
}
```

# Self-Test Subsystem Example

Self-tests execute a series of tests on hardware and mechanical systems to ensure correct operation. In this example, there are four state machine classes implementing our self-test subsystem as shown in the inheritance diagram below:

![Self-Test Subsystem](Figure_1.png)

## SelfTestEngine

`SelfTestEngine` is thread-safe and the main point of contact for client’s utilizing the self-test subsystem. `CentrifugeTest` and `PressureTest` are members of `SelfTestEngine`. `SelfTestEngine` is responsible for sequencing the individual self-tests in the correct order as shown in the state diagram below. 

![Self-Test Engine](Figure_2.png)

The `Start` event initiates the self-test engine. `SelfTestEngine::Start()` is an asynchronous function that reinvokes the function on the state machine's execution thread. 

```cpp
void SelfTestEngine::Start(const StartData* data)
{
	// Asynchronously invoke SelfTestEngine::Start on the SelfTestEngine thread of control
	ASYNC_INVOKE(SelfTestEngine, Start, data);

	BEGIN_TRANSITION_MAP			              			// - Current State -
		TRANSITION_MAP_ENTRY (ST_START_CENTRIFUGE_TEST)		// ST_IDLE
		TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)				// ST_COMPLETED
		TRANSITION_MAP_ENTRY (CANNOT_HAPPEN)				// ST_FAILED
		TRANSITION_MAP_ENTRY (EVENT_IGNORED)				// ST_START_CENTRIFUGE_TEST
		TRANSITION_MAP_ENTRY (EVENT_IGNORED)				// ST_START_PRESSURE_TEST
	END_TRANSITION_MAP(data)
}
```

When each self-test completes, the `Complete` event fires causing the next self-test to start. After all of the tests are done, the state machine transitions to `Completed` and back to `Idle`. If the `Cancel` event is generated at any time during execution, a transition to the `Failed` state occurs.

The `SelfTest` base class provides three states common to all `SelfTest`-derived state machines: `Idle`, `Completed`, and `Failed`. `SelfTestEngine` then adds two more states: `StartCentrifugeTest` and `StartPressureTest`.

`SelfTestEngine` has one public event function, `Start()`, that starts the self-tests. `SelfTestEngine::StatusCallback` is an asynchronous callback allowing client’s to register for status updates during testing.

```cpp
class SelfTestEngine : public SelfTest
{
public:
	// Clients register for asynchronous self-test status callbacks
	static MulticastDelegateSafe<void(const SelfTestStatus&)> StatusCallback;

	// Singleton instance of SelfTestEngine
	static SelfTestEngine& GetInstance();

	// Start the self-tests. This is a thread-safe asycnhronous function. 
	void Start(const StartData* data);

	static void InvokeStatusCallback(std::string msg);

private:
	SelfTestEngine();
	void Complete();

	// Sub self-test state machines 
	CentrifugeTest m_centrifugeTest;
	PressureTest m_pressureTest;

	StartData m_startData;

	// State enumeration order must match the order of state method entries
	// in the state map.
	enum States
	{
		ST_START_CENTRIFUGE_TEST = SelfTest::ST_MAX_STATES,
		ST_START_PRESSURE_TEST,
		ST_MAX_STATES
	};

	// Define the state machine state functions with event data type
	STATE_DECLARE(SelfTestEngine, 	StartCentrifugeTest,	StartData)
	STATE_DECLARE(SelfTestEngine, 	StartPressureTest,		NoEventData)

	// State map to define state object order. Each state map entry defines a
	// state object.
	BEGIN_STATE_MAP
		STATE_MAP_ENTRY(&Idle)
		STATE_MAP_ENTRY(&Completed)
		STATE_MAP_ENTRY(&Failed)
		STATE_MAP_ENTRY(&StartCentrifugeTest)
		STATE_MAP_ENTRY(&StartPressureTest)
	END_STATE_MAP	
};
```

As mentioned previously, the `SelfTestEngine` registers for asynchronous callbacks from each sub self-tests (i.e. `CentrifugeTest` and `PressureTest`) as shown below. When a sub self-test state machine completes, the `SelfTestEngine::Complete()` function is called. When a sub self-test state machine fails, the `SelfTestEngine::Cancel()` function is called.

Note that `m_centrifugeTest` and `m_pressureTest` share the same `SelfTestEngine` thread instance. This means all self-tests execute on the same thread.

```cpp
SelfTestEngine::SelfTestEngine() :
	SelfTest("SelfTestEngine",  ST_MAX_STATES)
{
	// Set owned state machines to execute on SelfTestEngine thread of control
	m_centrifugeTest.SetThread(GetThread());
	m_pressureTest.SetThread(GetThread());

	// Register for callbacks when sub self-test state machines complete or fail
	m_centrifugeTest.CompletedCallback += MakeDelegate(this, &SelfTestEngine::Complete);
	m_centrifugeTest.FailedCallback += MakeDelegate<SelfTest>(this, &SelfTest::Cancel);
	m_pressureTest.CompletedCallback += MakeDelegate(this, &SelfTestEngine::Complete);
	m_pressureTest.FailedCallback += MakeDelegate<SelfTest>(this, &SelfTest::Cancel);
}
```

The `SelfTest` base class generates the `CompletedCallback` and `FailedCallback` within the `Completed` and `Failed` states respectively as seen below:

```cpp
STATE_DEFINE(SelfTest, Completed, NoEventData)
{
	SelfTestEngine::InvokeStatusCallback("SelfTest::ST_Completed");

	if (CompletedCallback)
		CompletedCallback();

	InternalEvent(ST_IDLE);
}

STATE_DEFINE(SelfTest, Failed, NoEventData)
{
	SelfTestEngine::InvokeStatusCallback("SelfTest::ST_Failed");

	if (FailedCallback)
		FailedCallback();

	InternalEvent(ST_IDLE);
}
```

One might ask why the state machines use asynchronous delegate callbacks. If the state machines are on the same thread, why not use a normal, synchronous callback instead? The problem to prevent is a callback into a currently executing state machine, that is, the call stack wrapping back around into the same class instance. For example, the following call sequence should be prevented: `SelfTestEngine` calls `CentrifugeTest` calls back `SelfTestEngine`. An asynchronous callback allows the stack to unwind and prevents this unwanted behavior.

## CentrifugeTest

The `CentrifugeTest` state machine diagram shown below implements the centrifuge self-test. `CentrifugeTest` uses state machine inheritance by inheriting the `Idle`, `Completed` and `Failed` states from the `SelfTest` class. The `Timer` class is used to provide `Poll` events via asynchronous delegate callbacks.

![Centrifuge Test State Machine](CentrifugeTest.png)

## Timer

The `Timer` class provides a common mechanism to receive function callbacks by registering with `Expired`. `Start()` starts the callbacks at a particular interval. `Stop()` stops the callbacks.

```cpp
class Timer 
{
public:
	/// Client's register with Expired to get timer callbacks
	SinglecastDelegate<void(void)> Expired;

	/// Constructor
	Timer(void);

	/// Destructor
	~Timer(void);

	/// Starts a timer for callbacks on the specified timeout interval.
	/// @param[in]	timeout - the timeout in milliseconds.
	void Start(std::chrono::milliseconds timeout);

	/// Stops a timer.
	void Stop();

	/// Gets the enabled state of a timer.
	/// @return		TRUE if the timer is enabled, FALSE otherwise.
	bool Enabled() { return m_enabled; }

	/// Get the current time in ticks. 
	/// @return The current time in ticks. 
    static std::chrono::milliseconds GetTime();

    // etc...
```

## Run-Time

The program’s `main()` function is shown below. It creates the two threads, registers for callbacks from `SelfTestEngine`, then calls `Start()` to start the self-tests.

```cpp
int main(void)
{	
	try
	{	
		// Create the worker thread
		userInterfaceThread.CreateThread();

		// *** Begin async Motor test ***
		Motor motor;

		auto data = new MotorData();
		data->speed = 100;
		motor.SetSpeed(data);

		data = new MotorData();
		data->speed = 200;
		motor.SetSpeed(data);

		motor.Halt();
		// *** End async Motor test ***

		// *** Begin async self test ***		
		// Register for self-test engine callbacks
		SelfTestEngine::StatusCallback += MakeDelegate(&SelfTestEngineStatusCallback, userInterfaceThread);
		SelfTestEngine::GetInstance().CompletedCallback += MakeDelegate(&SelfTestEngineCompleteCallback, userInterfaceThread);

		// Start self-test engine
		StartData startData;
		startData.shortSelfTest = TRUE;
		SelfTestEngine::GetInstance().Start(&startData);

		// Wait for self-test engine to complete 
		while (!selfTestEngineCompleted)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		// Unregister for self-test engine callbacks
		SelfTestEngine::StatusCallback -= MakeDelegate(&SelfTestEngineStatusCallback, userInterfaceThread);
		SelfTestEngine::GetInstance().CompletedCallback -= MakeDelegate(&SelfTestEngineCompleteCallback, userInterfaceThread);
		// *** End async self test **

		// Exit the worker thread
		userInterfaceThread.ExitThread();
	}
	catch (...)
	{
		std::cerr << "Exception!" << std::endl;
	}

	return 0;
}
```

`SelfTestEngine` generates asynchronous callbacks on the `UserInteface` thread. The `SelfTestEngineStatusCallback()` callback outputs the message to the console.

```cpp
void SelfTestEngineStatusCallback(const SelfTestStatus& status)
{
	// Output status message to the console "user interface"
	cout << status.message.c_str() << endl;
}

void SelfTestEngineCompleteCallback()
{
	selfTestEngineCompleted = TRUE;
}
```

# Conclusion

The `AsyncStateMachine` class allows state machine objects to operate in their own thread context. The `DelegateMQ` library offer cross-thread event generation for easy collaboration between state machines.

