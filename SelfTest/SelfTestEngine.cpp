#include "SelfTestEngine.h"

//------------------------------------------------------------------------------
// GetInstance
//------------------------------------------------------------------------------
SelfTestEngine& SelfTestEngine::GetInstance()
{
    static SelfTestEngine instance;
    return instance;
}

//------------------------------------------------------------------------------
// SelfTestEngine
//------------------------------------------------------------------------------
SelfTestEngine::SelfTestEngine() :
    SelfTest("SelfTestEngine", ST_MAX_STATES)
{
    // Set owned state machines to execute on SelfTestEngine thread of control
    m_centrifugeTest.SetThread(GetThread());
    m_pressureTest.SetThread(GetThread());

    // CONNECT SIGNALS (RAII)
    // Register for signals when sub self-test state machines complete or fail.
    // We store the connection handles to ensure they stay connected.
    m_centrifugeCompleteConn = m_centrifugeTest.OnCompleted->Connect(
        MakeDelegate(this, &SelfTestEngine::Complete));

    m_centrifugeFailedConn = m_centrifugeTest.OnFailed->Connect(
        MakeDelegate<SelfTest>(this, &SelfTest::Cancel));

    m_pressureCompleteConn = m_pressureTest.OnCompleted->Connect(
        MakeDelegate(this, &SelfTestEngine::Complete));

    m_pressureFailedConn = m_pressureTest.OnFailed->Connect(
        MakeDelegate<SelfTest>(this, &SelfTest::Cancel));
}

//------------------------------------------------------------------------------
// InvokeStatusSignal
//------------------------------------------------------------------------------
void SelfTestEngine::InvokeStatusSignal(std::string msg)
{
    // Client(s) registered? Dereference SignalPtr to invoke.
    if (OnStatus)
    {
        SelfTestStatus status;
        status.message = msg;
        (*OnStatus)(status);
    }
}

//------------------------------------------------------------------------------
// Start
//------------------------------------------------------------------------------
void SelfTestEngine::Start(const StartData* data)
{
    // Asynchronously invoke on the SelfTestEngine thread of control
    ASYNC_INVOKE(SelfTestEngine, Start, data);

    BEGIN_TRANSITION_MAP			 			 		// - Current State -
        TRANSITION_MAP_ENTRY(ST_START_CENTRIFUGE_TEST)	// ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)				// ST_COMPLETED
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)				// ST_FAILED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)				// ST_START_CENTRIFUGE_TEST
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)				// ST_START_PRESSURE_TEST
    END_TRANSITION_MAP(data)
}

//------------------------------------------------------------------------------
// Complete
//------------------------------------------------------------------------------
void SelfTestEngine::Complete()
{
    ASYNC_INVOKE(SelfTestEngine, Complete);

    BEGIN_TRANSITION_MAP			 			 		// - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)				// ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)				// ST_COMPLETED
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)				// ST_FAILED
        TRANSITION_MAP_ENTRY(ST_START_PRESSURE_TEST)	// ST_START_CENTRIFUGE_TEST
        TRANSITION_MAP_ENTRY(ST_COMPLETED)				// ST_START_PRESSURE_TEST
    END_TRANSITION_MAP(NULL)
}

//------------------------------------------------------------------------------
// StartCentrifugeTest
//------------------------------------------------------------------------------
STATE_DEFINE(SelfTestEngine, StartCentrifugeTest, StartData)
{
    m_startData = *data;

    InvokeStatusSignal("SelfTestEngine::ST_CentrifugeTest");
    m_centrifugeTest.Start(&m_startData);
}

//------------------------------------------------------------------------------
// StartPressureTest
//------------------------------------------------------------------------------
STATE_DEFINE(SelfTestEngine, StartPressureTest, NoEventData)
{
    InvokeStatusSignal("SelfTestEngine::ST_PressureTest");
    m_pressureTest.Start(&m_startData);
}