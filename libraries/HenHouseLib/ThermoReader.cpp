#include "ThermoReader.h"

State ThermoReader::ReadState(State state) {
    int heatState = analogRead(THERMO_PIN);
    double Temp = log((10240000 / heatState) - 10000);
    Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
    state.Heat = Temp - 273.15; // Convert Kelvin to Celcius    
    return state;
}