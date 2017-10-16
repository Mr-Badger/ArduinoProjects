#ifndef StateReader_h
#define StateReader_h

#include "State.h"

class StateReader
{
public:
    virtual State ReadState(State state);
};

#endif