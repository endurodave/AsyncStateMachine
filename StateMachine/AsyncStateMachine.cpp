#include "AsyncStateMachine.h"

using namespace DelegateLib;

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
        m_thread = std::make_shared<WorkerThread>(threadName);
        m_thread->CreateThread();
    }
}



