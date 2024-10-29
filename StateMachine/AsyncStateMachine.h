#ifndef _ASYNC_STATE_MACHINE_H
#define _ASYNC_STATE_MACHINE_H

// @see https://github.com/endurodave/AsyncStateMachine
// David Lafreniere

#include "StateMachine.h"
#include "DelegateLib.h"
#include "WorkerThreadStd.h"
#include <string>

/// Helper function to simplify asynchronous function invoke
/// @param[in] obj - a class instance 
/// @param[in] func - a class function to invoke
/// @param[in] thread - a thread to invoke the function on
/// @param[in] args - the class function argument(s) passed to func
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
// stateMachine - the state machine class name
// stateName - the state machine external event function name
// ... - the stateName function argument, if any
#define ASYNC_INVOKE(stateMachine, stateName, ...) \
    { \
        if (!GetThread()) \
            throw std::runtime_error("Thread is not initialized (nullptr)."); \
        if (GetThread()->GetThreadId() != WorkerThread::GetCurrentThreadId()) { \
            AsyncInvoke(this, &stateMachine::stateName, *GetThread(), ##__VA_ARGS__); \
            return; \
        } \
    }

// AsyncStateMachine is a StateMachine and uses a WorkerThread. WorkerThread provides
// a C++ std::thread worker thread with a message queue. A delegate asynchronous 
// function invocation inserts a message into the message queue using 
// DelegateThread::DispatchDelegate(). External state machine events utilize 
// ASYNC_INVOKE to generate an external event on the state machine's worker thread.
// WorkerThread uses a std::thread, however any OS thread API is possible. The only 
// requirement is that the thread implement DispatchDelegate() for queue insertion.
// The destination thread must dequeue the delegate message and call DelegateInvoke().
// See https://github.com/endurodave/AsyncMulticastDelegateModern
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
    /// @param[in] thread - a WorkerThread instance
    void SetThread(std::shared_ptr<WorkerThread> thread) { m_thread = thread;  }

    /// Get the thread attached to this state machine
    /// @return A WorkerThread instance
    std::shared_ptr<WorkerThread> GetThread() { return m_thread; }

protected:
    /// @see StateMachine::ExternalEvent()
    void ExternalEvent(BYTE newState, const EventData* pData = NULL);

private:
    // The worker thread instance the state machine executes on
    std::shared_ptr<WorkerThread> m_thread = nullptr;
};

#endif // _ASYNC_STATE_MACHINE_H
