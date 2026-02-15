/**
 * @file main.cpp
 * @brief Self-Test Engine Example Entry Point utilizing Signal-Slot architecture.
 *
 * @details
 * This example demonstrates a robust, multi-threaded architecture for a
 * "Self-Test Engine" using the DelegateMQ library and an Asynchronous State Machine.
 *
 * **System Architecture:**
 * - **SelfTestEngine (Master):** A Singleton State Machine running on its own
 * dedicated worker thread. It coordinates the execution of sub-tests.
 * - **Sub-Tests:** Independent State Machines (e.g., CentrifugeTest) that inherit
 * from a common AsyncStateMachine-based SelfTest class.
 * - **User Interface:** A simulated UI thread that receives status updates
 * asynchronously via Signals.
 *
 * **Key DelegateMQ & Signal-Slot Features:**
 * 1. **Thread-Safe Signals (SignalPtr):**
 * - Uses dmq::SignalPtr to provide a publisher/subscriber mechanism for
 * task completion and status updates.
 * 2. **RAII Connection Management:**
 * - Uses dmq::ScopedConnection to manage signal lifetimes. Connections are
 * automatically severed when the ScopedConnection object is destroyed.
 * 3. **Asynchronous Marshaling:**
 * - MakeDelegate() transparently marshals callbacks from worker threads to
 * the userInterfaceThread, ensuring thread-safe UI updates.
 * 4. **Asynchronous Invocation:**
 * - Utilizes ASYNC_INVOKE to ensure state transitions occur on the
 * appropriate thread of control.
 *
 * @see https://github.com/endurodave/cpp-signal-slot-fsm
 * David Lafreniere
 */

#include "DelegateMQ.h"
#include "SelfTestEngine.h"
#include <iostream>
#include "DataTypes.h"
#include "Motor.h"
#include <atomic>
#include <chrono>

// @see https://github.com/endurodave/cpp-signal-slot-fsm
// David Lafreniere

using namespace std;
using namespace dmq;

// A thread to capture self-test status callbacks for output to the "user interface"
Thread userInterfaceThread("UserInterface");

// Simple flag to exit main loop (Atomic for thread safety)
std::atomic<bool> selfTestEngineCompleted(false);

std::atomic<bool> processTimerExit(false);
static void ProcessTimers()
{
	while (!processTimerExit.load())
	{
		// Process all delegate-based timers
		Timer::ProcessTimers();
		std::this_thread::sleep_for(std::chrono::microseconds(50));
	}
}

//------------------------------------------------------------------------------
// OnSelfTestEngineStatus
//------------------------------------------------------------------------------
void OnSelfTestEngineStatus(const SelfTestStatus& status)
{
	// Output status message to the console "user interface"
	cout << status.message.c_str() << endl;
}

//------------------------------------------------------------------------------
// OnSelfTestEngineComplete
//------------------------------------------------------------------------------
void OnSelfTestEngineComplete()
{
	selfTestEngineCompleted = true;
}

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(void)
{
	// Start the thread that will run ProcessTimers
	std::thread timerThread(ProcessTimers);

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
		// -------------------------------------------------------------------------
		// CONNECT SIGNALS (RAII)
		// -------------------------------------------------------------------------
		// We must store the connection handles!
		// If these fall out of scope, they automatically disconnect.
		ScopedConnection statusConn;
		ScopedConnection completeConn;

		// Register for status updates (Static Signal)
		statusConn = SelfTestEngine::OnStatus->Connect(
			MakeDelegate(&OnSelfTestEngineStatus, userInterfaceThread)
		);

		// Register for completion (Instance Signal from base class)
		completeConn = SelfTestEngine::GetInstance().OnCompleted->Connect(
			MakeDelegate(&OnSelfTestEngineComplete, userInterfaceThread)
		);

		// Start self-test engine
		StartData startData;
		startData.shortSelfTest = TRUE;
		SelfTestEngine::GetInstance().Start(&startData);

		// Wait for self-test engine to complete 
		while (!selfTestEngineCompleted)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		// -------------------------------------------------------------------------
		// DISCONNECT
		// -------------------------------------------------------------------------
		// Explicitly disconnect (optional, as destructors handle this automatically)
		statusConn.Disconnect();
		completeConn.Disconnect();
		// *** End async self test **

		// Exit the worker thread
		userInterfaceThread.ExitThread();
	}
	catch (...)
	{
		std::cerr << "Exception!" << std::endl;
	}

	// Ensure the timer thread completes before main exits
	processTimerExit.store(true);
	if (timerThread.joinable())
		timerThread.join();

	return 0;
}