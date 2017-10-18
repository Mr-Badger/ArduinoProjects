#ifndef ButtonReader_h
#define ButtonReader_h

#include "arduino.h"
#include "StateReader.h"

class ButtonReader : public StateReader 
{
private:
	const int BUTTON_PIN = 7;
	int lastState = 0;
	long downTime = 0;
public: 
	ButtonReader();
    virtual State ReadState(State state);
};

#endif