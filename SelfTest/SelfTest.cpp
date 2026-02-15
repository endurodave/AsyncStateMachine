#include "SelfTest.h"
#include "SelfTestEngine.h"

//------------------------------------------------------------------------------
// SelfTest
//------------------------------------------------------------------------------
SelfTest::SelfTest(const std::string& name, INT maxStates) :
    AsyncStateMachine(maxStates)
{
    CreateThread(name);
}

//------------------------------------------------------------------------------
// SelfTest
//------------------------------------------------------------------------------
SelfTest::SelfTest(INT maxStates) :
    AsyncStateMachine(maxStates)
{
}

//------------------------------------------------------------------------------
// Cancel
//------------------------------------------------------------------------------
void SelfTest::Cancel()
{
    // ASYNC_INVOKE ensures the transition logic runs on this machine's thread
    ASYNC_INVOKE(SelfTest, Cancel);

    if (GetCurrentState() != ST_IDLE)
        ExternalEvent(ST_FAILED);
}

//------------------------------------------------------------------------------
// Idle
//------------------------------------------------------------------------------
STATE_DEFINE(SelfTest, Idle, NoEventData)
{
    SelfTestEngine::InvokeStatusSignal("SelfTest::ST_Idle");
}

//------------------------------------------------------------------------------
// EntryIdle
//------------------------------------------------------------------------------
ENTRY_DEFINE(SelfTest, EntryIdle, NoEventData)
{
    SelfTestEngine::InvokeStatusSignal("SelfTest::EN_EntryIdle");
}

//------------------------------------------------------------------------------
// Completed
//------------------------------------------------------------------------------
STATE_DEFINE(SelfTest, Completed, NoEventData)
{
    SelfTestEngine::InvokeStatusSignal("SelfTest::ST_Completed");

    // Use SignalPtr and dereference to invoke connected slots
    if (OnCompleted)
        (*OnCompleted)();

    InternalEvent(ST_IDLE);
}

//------------------------------------------------------------------------------
// Failed
//------------------------------------------------------------------------------
STATE_DEFINE(SelfTest, Failed, NoEventData)
{
    SelfTestEngine::InvokeStatusSignal("SelfTest::ST_Failed");

    // Use SignalPtr and dereference to invoke connected slots
    if (OnFailed)
        (*OnFailed)();

    InternalEvent(ST_IDLE);
}