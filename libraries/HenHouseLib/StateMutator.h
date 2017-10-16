#ifndef StateMutator_h
#define StateMutator_h

#include "State.h"

class StateMutator
{
public:
    virtual State MutateState(State state);
};

#endif