#ifndef _SELF_TEST_ENGINE_H
#define _SELF_TEST_ENGINE_H

#include "SelfTest.h"
#include "CentrifugeTest.h"
#include "PressureTest.h"
#include "DelegateMQ.h"

using namespace dmq;

struct SelfTestStatus
{
	std::string message;
};

/// @brief The master self-test state machine used to coordinate the execution of the 
/// sub self-test state machines. The class is thread-safe. 
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

#endif