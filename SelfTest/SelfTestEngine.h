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
	// Clients register for asynchronous self-test status callbacks via Signal-Slot
	static inline dmq::SignalPtr<void(const SelfTestStatus&)> OnStatus =
		dmq::MakeSignal<void(const SelfTestStatus&)>();

	static SelfTestEngine& GetInstance();

	void Start(const StartData* data);

	static void InvokeStatusSignal(std::string msg);

private:
	SelfTestEngine();
	void Complete();

	CentrifugeTest m_centrifugeTest;
	PressureTest m_pressureTest;

	StartData m_startData;

	// RAII Connection Handles
	// These MUST be stored to keep the signal connections alive
	dmq::ScopedConnection m_centrifugeCompleteConn;
	dmq::ScopedConnection m_centrifugeFailedConn;
	dmq::ScopedConnection m_pressureCompleteConn;
	dmq::ScopedConnection m_pressureFailedConn;

	enum States
	{
		ST_START_CENTRIFUGE_TEST = SelfTest::ST_MAX_STATES,
		ST_START_PRESSURE_TEST,
		ST_MAX_STATES
	};

	STATE_DECLARE(SelfTestEngine, StartCentrifugeTest, StartData)
	STATE_DECLARE(SelfTestEngine, StartPressureTest, NoEventData)

	BEGIN_STATE_MAP
		STATE_MAP_ENTRY(&Idle)
		STATE_MAP_ENTRY(&Completed)
		STATE_MAP_ENTRY(&Failed)
		STATE_MAP_ENTRY(&StartCentrifugeTest)
		STATE_MAP_ENTRY(&StartPressureTest)
	END_STATE_MAP
};

#endif