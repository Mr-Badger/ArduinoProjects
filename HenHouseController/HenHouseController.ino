#include "StateReader.h"
#include "GpsReader.h"
#include "ThermoReader.h"
#include "StateMutator.h"
#include "HeaterMutator.h"
#include "DoorMutator.h"
#include "LightMutator.h"
#include "Logger.h"

class Applictaion {
private:
    State _state = State();    
    Logger _logger = Logger();
    
    StateReader* _stateReaders[2] {
        new GpsReader(),
        new ThermoReader()
    };
    
    StateMutator* _stateMutators[3] {
        new DoorMutator(&_logger),
        new HeaterMutator(&_logger),
        new LightMutator(&_logger)
    };

    long GetTimeSeconds(long hours, long minutes, long seconds)
    {     
        return (hours * 60 * 60) + (minutes * 60) + seconds;
    }

public:
    Applictaion()
    {
        Serial.println("Application Start");

        //* Enviroment
        _state.Time = -1; // GetTimeSeconds(6,0,0); //* _state.Time updated in TimeReader

        //* Things
        _state.LightsOn = false;        
        _state.DoorState = 0;
        _state.HeaterOn = false;

        //* User actions
        _state.DoorManualOpen = false;        

        //* goals
        _state.HeatGoal = 10;        
    
        _state.LightsOnTime = GetTimeSeconds(16,0,0);
        _state.LightsOffTime = GetTimeSeconds(22,0,0);

        _state.DoorOpenTime = GetTimeSeconds(8,0,0);
        _state.DoorCloseTime = GetTimeSeconds(21,30,0);

        //* Log
        _state.DoorActivationTime = 0;
    }
    

    void Loop()
    {                
        for(int i = 0; i < sizeof(_stateReaders) / sizeof(int); i++)
        {
            _state = _stateReaders[i]->ReadState(_state);
        }                    

        if(_state.Time != -1)
        {
            _logger.SetTime(_state.Time);

            for(int i = 0; i < sizeof(_stateMutators) / sizeof(int); i++)
            {
                _state = _stateMutators[i]->MutateState(_state);
            }
        }
        else
        {
            //_logger.Debug("Acquiring Time");
        }    
    }
};


Applictaion app;

void setup() {
    Serial.begin(9600);
    Serial.println("Setup");
    app = Applictaion();
}

void loop() {
    app.Loop();
}