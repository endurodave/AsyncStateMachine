#include "DelegateLib.h"
#include "SelfTestEngine.h"
#include <iostream>
#include "WorkerThreadStd.h"
#include "DataTypes.h"
#include "Motor.h"

// @see https://github.com/endurodave/AsyncStateMachine
// David Lafreniere

using namespace std;
using namespace DelegateLib;

// A thread to capture self-test status callbacks for output to the "user interface"
WorkerThread userInterfaceThread("UserInterface");

// Simple flag to exit main loop
BOOL selfTestEngineCompleted = FALSE;

//------------------------------------------------------------------------------
// SelfTestEngineStatusCallback
//------------------------------------------------------------------------------
void SelfTestEngineStatusCallback(const SelfTestStatus& status)
{
	// Output status message to the console "user interface"
	cout << status.message.c_str() << endl;
}

//------------------------------------------------------------------------------
// SelfTestEngineCompleteCallback
//------------------------------------------------------------------------------
void SelfTestEngineCompleteCallback()
{
	selfTestEngineCompleted = TRUE;
}

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
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

