#include "StateReader.h"
#include "GpsReader.h"
#include "ThermoReader.h"
#include "ButtonReader.h"
#include "InternetReader.h"
#include "StateMutator.h"
#include "HeaterMutator.h"
#include "DoorMutator.h"
#include "LightMutator.h"
#include "Logger.h"

class Applictaion
{
  private:
	State _state = State();
	Logger _logger = Logger();
	long _lastReport;

	StateReader *_stateReaders[3]{
		new GpsReader(),
		new ThermoReader(),
		new ButtonReader(),
		//new InternetReader()
	};

	StateMutator *_stateMutators[3]{
		new DoorMutator(&_logger),
		new HeaterMutator(&_logger),
		new LightMutator(&_logger)};

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
		_state.ButtonClicked = false;

		//* goals
		_state.HeatGoal = 10;		

		_state.LightsOnTime = GetTimeSeconds(16, 0, 0);
		_state.LightsOffTime = GetTimeSeconds(22, 0, 0);

		_state.DoorActivationDuration = 26;
		_state.DoorOpenTime = GetTimeSeconds(8, 0, 0);
		_state.DoorCloseTime = GetTimeSeconds(21, 30, 0);

		//* Log
		_state.DoorActivationTime = 0;
		_state.HeaterChangedTime = 0;
		_state.TimeFactor = 1;
	}


	void Loop()
	{		
		for (int i = 0; i < sizeof(_stateReaders) / sizeof(int); i++)
		{
			_state = _stateReaders[i]->ReadState(_state);
		}


		_logger.SetTime(_state.Time);

		for (int i = 0; i < sizeof(_stateMutators) / sizeof(int); i++)
		{
			_state = _stateMutators[i]->MutateState(_state);
		}


		//if(millis() - _lastReport > 600000 / _state.TimeFactor)
		if(millis() - _lastReport > 6000 / _state.TimeFactor)
		{
			char buffer [255];			
			sprintf(buffer,"Heat: %d, Lights: %d, Heater: %d, Door: %d",_state.Heat,_state.LightsOn,_state.HeaterOn,_state.DoorState);
			_logger.Debug(buffer);
			
			_lastReport = millis();
		}


	}
};

Applictaion app;

void setup()
{
	Serial.begin(9600);
	Serial.println("Setup");

	//InternetReader * ir = new InternetReader();

	app = Applictaion();
}

void loop()
{
	app.Loop();
}