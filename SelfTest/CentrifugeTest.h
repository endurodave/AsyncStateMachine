#ifndef _CENTRIFUGE_TEST_H
#define _CENTRIFUGE_TEST_H

#include "SelfTest.h"

// @brief CentrifugeTest shows StateMachine features including state machine
// inheritance, state function override, and guard/entry/exit actions. 
class CentrifugeTest : public SelfTest
{
public:
    CentrifugeTest();
    virtual void Start(const StartData* data);

private:
    void Poll();

    Timer m_pollTimer;
    dmq::ScopedConnection m_pollTimerConn; // RAII connection handle

    INT m_speed;

    enum States
    {
        ST_START_TEST = SelfTest::ST_MAX_STATES,
        ST_ACCELERATION,
        ST_WAIT_FOR_ACCELERATION,
        ST_DECELERATION,
        ST_WAIT_FOR_DECELERATION,
        ST_MAX_STATES
    };

    STATE_DECLARE(CentrifugeTest, Idle, NoEventData)
        STATE_DECLARE(CentrifugeTest, StartTest, StartData)
        GUARD_DECLARE(CentrifugeTest, GuardStartTest, NoEventData)
        STATE_DECLARE(CentrifugeTest, Acceleration, NoEventData)
        STATE_DECLARE(CentrifugeTest, WaitForAcceleration, NoEventData)
        EXIT_DECLARE(CentrifugeTest, ExitWaitForAcceleration)
        STATE_DECLARE(CentrifugeTest, Deceleration, NoEventData)
        STATE_DECLARE(CentrifugeTest, WaitForDeceleration, NoEventData)
        EXIT_DECLARE(CentrifugeTest, ExitWaitForDeceleration)

        BEGIN_STATE_MAP_EX
        STATE_MAP_ENTRY_ALL_EX(&Idle, 0, &EntryIdle, 0)
        STATE_MAP_ENTRY_EX(&Completed)
        STATE_MAP_ENTRY_EX(&Failed)
        STATE_MAP_ENTRY_ALL_EX(&StartTest, &GuardStartTest, 0, 0)
        STATE_MAP_ENTRY_EX(&Acceleration)
        STATE_MAP_ENTRY_ALL_EX(&WaitForAcceleration, 0, 0, &ExitWaitForAcceleration)
        STATE_MAP_ENTRY_EX(&Deceleration)
        STATE_MAP_ENTRY_ALL_EX(&WaitForDeceleration, 0, 0, &ExitWaitForDeceleration)
        END_STATE_MAP_EX
};

#endif