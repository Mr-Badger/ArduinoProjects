#ifndef DoorMutator_h
#define DoorMutator_h

#include "arduino.h"
#include "Logger.h"
#include "StateMutator.h"

class DoorMutator : public StateMutator
{
private:
    void StartOpeningDoor();
    void StartClosingDoor();
    void StopDoor();

    Logger* _logger;    

    const int OPEN_PIN = 5;
    const int CLOSE_PIN = 6;
	const int POWER_PIN = 3; // Uses PWN
	
	const int CLOSED = 0;
	const int OPENING = 1;
	const int CLOSING = 2;
	const int OPEN = 3;

	const int MANUAL_CLOSED = 10;
	const int MANUAL_OPENING = 11;
	const int MANUAL_CLOSING = 12;
	const int MANUAL_OPEN = 13;
	
    
public:
    DoorMutator(Logger* logger);
    virtual State MutateState(State state);    
};

#endif