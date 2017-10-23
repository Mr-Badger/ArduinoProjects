#include "HeaterMutator.h"

void HeaterMutator::TurnOffHeater()
{
    _logger->Debug("Turn off heater");
    digitalWrite(HEATER_PIN, HIGH);
}

void HeaterMutator::TurnOnHeater()
{
    _logger->Debug("Turn on heater");
    digitalWrite(HEATER_PIN, LOW);
}

HeaterMutator::HeaterMutator(Logger* logger)
{
    _logger = logger;
	pinMode(HEATER_PIN, OUTPUT);
	TurnOffHeater();
}

State HeaterMutator::MutateState(State state) 
{    
    //state.HeatGoal = 26;

    if((state.Heat < state.HeatGoal - 2) && !state.HeaterOn)
    {
        TurnOnHeater();
        state.HeaterOn = true;
    }
    else if((state.Heat > state.HeatGoal + 2) && state.HeaterOn)
    {
        TurnOffHeater();
        state.HeaterOn = false;
    }

    return state;
}
