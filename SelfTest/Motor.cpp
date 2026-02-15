#include "Motor.h"
#include <iostream>

using namespace std;

Motor::Motor() :
    AsyncStateMachine(ST_MAX_STATES),
    m_currentSpeed(0)
{
    CreateThread("Motor");
}
    
// set motor speed external event
void Motor::SetSpeed(MotorData* data)
{
    /* ASYNC_INVOKE below effectively executes the following code:
    // Is this function call executing on this state machine thread?
    if (GetThreadId() != Thread::GetCurrentThreadId())
    {
        // Asynchronously re-invoke the SetSpeed() event on Motor's thread
        AsyncInvoke(this, &Motor::SetSpeed, *GetThread(), data);
        return;
    }*/

    // Asynchronously invoke Motor::SetSpeed on the Motor thread of control
    ASYNC_INVOKE(Motor, SetSpeed, data);

    BEGIN_TRANSITION_MAP                                    // - Current State -
        TRANSITION_MAP_ENTRY(ST_START)                      // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)                 // ST_STOP
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)               // ST_START
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)               // ST_CHANGE_SPEED
    END_TRANSITION_MAP(data)
}

// halt motor external event
void Motor::Halt()
{
    ASYNC_INVOKE(Motor, Halt);

    BEGIN_TRANSITION_MAP                                    // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)                 // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)                 // ST_STOP
        TRANSITION_MAP_ENTRY(ST_STOP)                       // ST_START
        TRANSITION_MAP_ENTRY(ST_STOP)                       // ST_CHANGE_SPEED
    END_TRANSITION_MAP(NULL)
}

// state machine sits here when motor is not running
STATE_DEFINE(Motor, Idle, NoEventData)
{
    cout << "Motor::ST_Idle" << endl;
}

// stop the motor 
STATE_DEFINE(Motor, Stop, NoEventData)
{
    cout << "Motor::ST_Stop" << endl;
    m_currentSpeed = 0; 

    // perform the stop motor processing here
    // transition to Idle via an internal event
    InternalEvent(ST_IDLE);
}

// start the motor going
STATE_DEFINE(Motor, Start, MotorData)
{
    cout << "Motor::ST_Start : Speed is " << data->speed << endl;
    m_currentSpeed = data->speed;

    // set initial motor speed processing here
}

// changes the motor speed once the motor is moving
STATE_DEFINE(Motor, ChangeSpeed, MotorData)
{
    cout << "Motor::ST_ChangeSpeed : Speed is " << data->speed << endl;
    m_currentSpeed = data->speed;

    // perform the change motor speed to data->speed here
}
