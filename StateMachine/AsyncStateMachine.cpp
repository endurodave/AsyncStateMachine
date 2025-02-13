#include "AsyncStateMachine.h"

using namespace dmq;

AsyncStateMachine::AsyncStateMachine(BYTE maxStates, BYTE initialState) :
    StateMachine(maxStates, initialState)
{
}

AsyncStateMachine::~AsyncStateMachine()
{
}

void AsyncStateMachine::CreateThread(const std::string& threadName)
{
    if (m_thread == nullptr)
    {
        m_thread = std::make_shared<Thread>(threadName);
        m_thread->CreateThread();
    }
}

void AsyncStateMachine::ExternalEvent(BYTE newState, const EventData* pData)
{
    // An asyc state machine external event must only be called on the 
    // GetThread() thread. Typically this means an external event function
    // is missing the ASYNC_INVOKE macro.
    if (GetThread()->GetThreadId() != Thread::GetCurrentThreadId())
        throw std::runtime_error("External event called on wrong thread.");

    StateMachine::ExternalEvent(newState, pData);
}



