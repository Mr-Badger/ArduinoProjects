#include "DoorMutator.h"

DoorMutator::DoorMutator(Logger* logger)
{
    _logger = logger;
    
    pinMode(POWER_PIN, OUTPUT);
    pinMode(OPEN_PIN, INPUT);
    pinMode(CLOSE_PIN, INPUT);
    
    digitalWrite(OPEN_PIN, HIGH);
    digitalWrite(CLOSE_PIN, LOW);
}

void DoorMutator::StartOpeningDoor()
{
    digitalWrite(OPEN_PIN, HIGH);
    digitalWrite(CLOSE_PIN, LOW);
    analogWrite(POWER_PIN, 255);    
    _logger->Debug("StartOpeningDoor");
}

void DoorMutator::StartClosingDoor()
{
    digitalWrite(OPEN_PIN, LOW);
    digitalWrite(CLOSE_PIN, HIGH);
    analogWrite(POWER_PIN, 255);
    _logger->Debug("StartClosingDoor");
}    

void DoorMutator::StopDoor()
{
    analogWrite(POWER_PIN, 0);    
    _logger->Debug("StopDoor");
}

State DoorMutator::MutateState(State state)
{
    int doorActivationDuration = 25 * 100;    

    if(state.DoorOpenTime < state.Time && state.Time < state.DoorCloseTime)
    {            
        //* Door open period
        
        if(state.DoorState == 0)
        {            
            StartOpeningDoor();
            state.DoorActivationTime = state.Time;
            state.DoorState = 1;
        }
        else if(state.DoorState == 1)
        {
            if(state.Time - state.DoorActivationTime > doorActivationDuration) 
            {
                StopDoor();
                state.DoorState = 2;
            }
        }

        return state;
    }
    else 
    {
        if(state.DoorState == 2)
        {
            StartClosingDoor();
            state.DoorActivationTime = state.Time;            
            state.DoorState = 1;
        }
        else if(state.DoorState == 1)
        {
            if(state.Time - state.DoorActivationTime > doorActivationDuration)
            {
                StopDoor();
                state.DoorState = 0; 
            }
        }
    }

    return state;
}