#include "GpsReader.h"

/*********************
 *10 to GPS Module TX*
 *09 to GPS Module RX*
 *********************/

SoftwareSerial mySerial(10, 11); // 10 is 9 and 11 is 10 ???
TinyGPS gps;

//long _startTime = 0;
long _gpsTime = -1;
long _gpsMillis = -1;
long _gpsDate = -1;

GpsReader::GpsReader() {
    mySerial.begin(9600);
}

State GpsReader::ReadState(State state) {
    
    bool newdata = false;
    bool shouldUpdateGpsTime = (millis() - _gpsMillis) > (86400000) * 0.25;

    if(_gpsTime == -1 || shouldUpdateGpsTime)
    {
        if (mySerial.available()) {
            char gpsData = mySerial.read();
            if (gps.encode(gpsData)) {
                newdata = true;
            }
        }
        
        if (newdata) {
            unsigned long age;
            int year;
            byte month, day, hour, minute, second, hundredths;
            gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
                    
            _gpsTime = ((long)hour * 3600) + ((long)minute * 60) + second;
            _gpsMillis = millis();
            state.Time = _gpsTime;            
            
            Serial.println(state.Time);
        }    
    }
    else
    {
        state.Time = (_gpsTime + ((millis() - _gpsMillis) / 1)) % 86400;
        //state.Time = (_gpsTime + ((millis() - _gpsMillis) / 1000)) % 86400;
    }

    return state;
}