#include "LightMutator.h"

void LightMutator::TurnOffLight()
{
    _logger->Debug("Turn off light");
    digitalWrite(LIGHT_PIN, HIGH);
}

void LightMutator::TurnOnLight()
{
    _logger->Debug("Turn on light");
    digitalWrite(LIGHT_PIN, LOW);
}

LightMutator::LightMutator(Logger* logger)
{
    _logger = logger;
	pinMode(LIGHT_PIN, OUTPUT);
	TurnOffLight();
}

State LightMutator::MutateState(State state) 
{
	if(state.Time == -1) return state;

    if(state.LightsOnTime < state.Time && state.Time < state.LightsOffTime)
    {
        if(!state.LightsOn)
        {
            TurnOnLight();
            state.LightsOn = true;
        }
    }
    else
    {
        if(state.LightsOn)
        {
            TurnOffLight();
            state.LightsOn = false;
        }
    }

    return state;
}
