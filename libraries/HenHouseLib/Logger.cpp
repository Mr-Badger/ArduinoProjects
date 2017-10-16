#include "Logger.h"

long _time;

Logger::Logger() {}

void Logger::SetTime(long time) {
    _time = time;
}

void Logger::Debug(char* message) {

    long hours = _time / 3600;
        
    long mintues = (_time % 3600) / 60;
    long seconds = _time % 60;

    Serial.print(hours);
    Serial.print(":");
    Serial.print(mintues);
    Serial.print(":");
    Serial.print(seconds);
    Serial.print(" ");
    Serial.println(message);

}