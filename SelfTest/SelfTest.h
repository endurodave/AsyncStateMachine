#ifndef _SELF_TEST_H
#define _SELF_TEST_H

#include "AsyncStateMachine.h"
#include "DelegateMQ.h"

/// @brief Start event data
struct StartData : public EventData
{
    BOOL shortSelfTest = FALSE;     // TRUE for abbreviated self-tests 
};

/// @brief SelfTest is a subclass state machine for other self-tests to 
/// inherit from. The class has common states for all derived classes to share. 
class SelfTest : public AsyncStateMachine
{
public:
    // Signals generated when the self-test completes or fails.
    // Initialized with SignalPtr for thread-safe RAII connection management.
    dmq::SignalPtr<void(void)> OnCompleted = dmq::MakeSignal<void(void)>();
    dmq::SignalPtr<void(void)> OnFailed = dmq::MakeSignal<void(void)>();

    SelfTest(const std::string& threadName, INT maxStates);
    SelfTest(INT maxStates);

    /// Starts the self-test
    /// @param[in] data - event data sent as part of the Start event
    virtual void Start(const StartData* data) = 0;

    /// Cancels the self-test
    void Cancel();

protected:
    // State enumeration order must match the order of state method entries
    // in the state map.
    enum States
    {
        ST_IDLE,
        ST_COMPLETED,
        ST_FAILED,
        ST_MAX_STATES
    };

    // Define the state machine states
    STATE_DECLARE(SelfTest, Idle, NoEventData)
    ENTRY_DECLARE(SelfTest, EntryIdle, NoEventData)
    STATE_DECLARE(SelfTest, Completed, NoEventData)
    STATE_DECLARE(SelfTest, Failed, NoEventData)
};

#endif