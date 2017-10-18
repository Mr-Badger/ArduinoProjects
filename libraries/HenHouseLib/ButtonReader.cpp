#include "ButtonReader.h"


ButtonReader::ButtonReader()
{
	pinMode(BUTTON_PIN,INPUT);
}

State ButtonReader::ReadState(State state) {
	int buttonPressed = digitalRead(BUTTON_PIN);	

	state.ButtonHold = false;
	state.ButtonClicked = false;
	state.ButtonReleased = false;

	if(buttonPressed == HIGH)
	{				
		if(lastState == 0)
		{
			Serial.println("Button Down");
			downTime = millis();
			lastState = 1;
		}
		else if(lastState == 1)
		{
			if(millis() - downTime > 500)
			{
				state.ButtonHold = true;
				//Serial.println("Button Hold");
			}
		}
	}
	else 
	{
		if(lastState == 1)
		{
			if(millis() - downTime < 500)
			{
				state.ButtonClicked = true;
				Serial.println("Button Click");
			}
			else
			{
				state.ButtonHold = true;
			}

			state.ButtonReleased = true;
			Serial.println("Button Released");
		}

		lastState = 0;
	}

    return state;
}