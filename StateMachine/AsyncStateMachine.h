#ifndef _ASYNC_STATE_MACHINE_H
#define _ASYNC_STATE_MACHINE_H

#include "StateMachine.h"
#include "DelegateLib.h"
#include "WorkerThreadStd.h"
#include <string>

// Helper function to simplify asynchronous function invoke
template <typename C, typename F, typename T, typename... Args>
void AsyncInvoke(C obj, F func, T& thread, Args&&... args)
{
    // Check if obj and func are nullptrs; throw exception if they are
    if (!obj)
        throw std::invalid_argument("Object pointer (obj) cannot be nullptr.");
 
    if (!func)
        throw std::invalid_argument("Function pointer (func) cannot be nullptr.");

    // Perform runtime checks for each argument to ensure none of the required arguments are nullptr
    bool invalidArgumentFound = ((args == nullptr) || ...);
    if (invalidArgumentFound)
        throw std::invalid_argument("One or more arguments are nullptr.");
    
    // Asynchronously call target function
    DelegateLib::MakeDelegate(obj, func, thread)(std::forward<Args>(args)...);
}

// Macro to simplify async function invocation within a state machine external event
// TODO: Check for GetThread() null, throw exception
#define ASYNC_INVOKE(stateMachine, stateName, ...) \
    if (!GetThread()) \
        throw std::runtime_error("Thread is not initialized (nullptr)."); \
    if (GetThread()->GetThreadId() != WorkerThread::GetCurrentThreadId()) { \
        AsyncInvoke(this, &stateMachine::stateName, *GetThread(), ##__VA_ARGS__); \
        return; \
    }

// AsyncStateMachine is a StateMachine and a WorkerThread. WorkerThread provides
// a C++ std::thread worker thread with a message queue. A delegate asynchronous 
// function invocation inserts a message into the message queue using 
// DelegateThread::DispatchDelegate(). External state machine events utilize 
// ASYNC_INVOKE to generate an external event on the state machine's worker thread.
// WorkerThread is a std::thread, however any OS thread API is possible. The only 
// requirement is that the thread implement DispatchDelegate() for queue insertion.
// The destination thread must dequeue the delegate message and call DelegateInvoke().
class AsyncStateMachine : public StateMachine
{
public:
    ///	Constructor.
    ///	@param[in] maxStates - the maximum number of state machine states.
    AsyncStateMachine(BYTE maxStates, BYTE initialState = 0);

    /// Destructor
    virtual ~AsyncStateMachine();

    void CreateThread(const std::string& threadName);

    void SetThread(std::shared_ptr<WorkerThread> thread) { m_thread = thread;  }

    std::shared_ptr<WorkerThread> GetThread() { return m_thread; }

private:
    std::shared_ptr<WorkerThread> m_thread = nullptr;
};

#endif // _ASYNC_STATE_MACHINE_H
