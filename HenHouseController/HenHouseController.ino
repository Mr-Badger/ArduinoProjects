#include "StateReader.h"
#include "GpsReader.h"
#include "ThermoReader.h"
#include "ButtonReader.h"
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
		new ButtonReader()
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

		_state.DoorOpenTime = GetTimeSeconds(8, 0, 0);
		_state.DoorCloseTime = GetTimeSeconds(21, 30, 0);

		//* Log
		_state.DoorActivationTime = 0;
		_state.TimeFactor = 1;
	}


	void Loop()
	{		
		for (int i = 0; i < sizeof(_stateReaders) / sizeof(int); i++)
		{
			_state = _stateReaders[i]->ReadState(_state);
		}

		if (_state.Time != -1)
		{
			_logger.SetTime(_state.Time);

			for (int i = 0; i < sizeof(_stateMutators) / sizeof(int); i++)
			{
				_state = _stateMutators[i]->MutateState(_state);
			}
		}
		else
		{
			//_logger.Debug("Acquiring Time");
		}

		if(millis() - _lastReport > 600000 / _state.TimeFactor)
		{
			Serial.print("Time: ");
			Serial.print(_state.Time / 3600);
			Serial.print(":");
			Serial.print((_state.Time % 3600) / 60);
			Serial.print(":");
			Serial.print(_state.Time % 60);

			Serial.print(" Heat: ");
			Serial.print(_state.Heat);			

			Serial.print(", Lights: ");
			Serial.print(_state.LightsOn);

			Serial.print(", Heater: ");
			Serial.print(_state.HeaterOn);

			Serial.print(", Door: ");
			Serial.println(_state.DoorState);

			_lastReport = millis();
		}


	}
};

Applictaion app;

void setup()
{
	Serial.begin(9600);
	Serial.println("Setup");
	app = Applictaion();
}

void loop()
{
	app.Loop();
}