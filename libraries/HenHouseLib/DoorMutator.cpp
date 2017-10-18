#include "DoorMutator.h"

DoorMutator::DoorMutator(Logger *logger)
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

/*
	0:Automatic Closed
	1:Automatic Opening	
	2:Automatic Closing
	3:Automatic Open

	10: Manual Closed
	11: Manual Opening
	12: Manual Closing
	13: Manual Open
*/



State DoorMutator::MutateState(State state)
{
	long doorActivationDuration = 31 * state.TimeFactor;

	if(state.DoorActivationTime > state.Time)
	{
		state.DoorActivationTime = state.DoorActivationTime - 86400;
	}

	
	if(state.ButtonClicked)
	{		
		if(state.DoorState == CLOSED || state.DoorState == MANUAL_CLOSED)
		{
			//* Manual/Automatic Closed -> Manual Opening
			state.DoorState = MANUAL_OPENING;
			state.DoorActivationTime = state.Time;
			StartOpeningDoor();
		}
		else if(state.DoorState == OPEN || state.DoorState == MANUAL_OPEN)
		{
			//* Manual/Automatic Open -> Manual Closing
			state.DoorState = MANUAL_CLOSING;
			state.DoorActivationTime = state.Time;
			StartClosingDoor();
		}
		else if(state.DoorState == MANUAL_OPENING || state.DoorState == MANUAL_CLOSING)
		{
			//* Manual Opening/Closing -> Manual Open/Closed
			if(state.DoorState == MANUAL_OPENING) state.DoorState = MANUAL_OPEN; //* Manual Opening -> Manual Open
			if(state.DoorState == MANUAL_CLOSING) state.DoorState = MANUAL_CLOSED;	//* Manual Closing -> Manual Closed
			StopDoor();
		}
	}

	if(!state.ButtonHold && (state.DoorState == MANUAL_OPENING || state.DoorState == MANUAL_CLOSING))
	{				
		if (state.Time - state.DoorActivationTime > doorActivationDuration)
		{			
			if(state.DoorState == MANUAL_OPENING) state.DoorState = MANUAL_OPEN; //* Manual Opening -> Manual Open
			if(state.DoorState == MANUAL_CLOSING) state.DoorState = MANUAL_CLOSED;	//* Manual Closing -> Manual Closed
			StopDoor();
		}
	}




	
	if(state.ButtonHold)
	{
		if(state.ButtonReleased)
		{
			if (state.DoorState == MANUAL_CLOSING) state.DoorState = MANUAL_CLOSED;
			else if (state.DoorState == MANUAL_OPENING) state.DoorState = MANUAL_OPEN;
				
			StopDoor();
		}
		else
		{
			if (state.DoorState == CLOSED || state.DoorState == MANUAL_CLOSED)
			{
				StartOpeningDoor();
				state.DoorState = MANUAL_OPENING;
			}
			else if (state.DoorState == OPEN || state.DoorState == MANUAL_OPEN)
			{
				StartClosingDoor();
				state.DoorState = MANUAL_CLOSING;
			}		
		}
	}

	if(!state.ButtonHold)
	{
		if (state.DoorOpenTime < state.Time && state.Time < state.DoorCloseTime)
		{
			//* Door open period
			if(state.DoorState == MANUAL_OPEN)
			{
				//* Revert to Automatic
				state.DoorState = OPEN;	
			}
			else if (state.DoorState == CLOSED)
			{
				StartOpeningDoor();
				state.DoorActivationTime = state.Time;
				state.DoorState = OPENING;
			}
			else if (state.DoorState == OPENING || state.DoorState == CLOSING)
			{
				long activationTime = state.Time - state.DoorActivationTime;

				if (state.Time - state.DoorActivationTime > doorActivationDuration)
				{
					StopDoor();
					state.DoorState = OPEN;
				}
			}

			return state;
		}
		else
		{
			if(state.DoorState == MANUAL_CLOSED)
			{
				//* Revert to Automatic
				state.DoorState = CLOSED;	
			}
			else if (state.DoorState == OPEN)
			{
				StartClosingDoor();
				state.DoorActivationTime = state.Time;
				state.DoorState = CLOSING;
			}
			else if (state.DoorState == CLOSING || state.DoorState == OPENING)
			{
				if (state.Time - state.DoorActivationTime > doorActivationDuration)
				{
					StopDoor();
					state.DoorState = CLOSED;
				}
			}
		}
	}
	return state;
}