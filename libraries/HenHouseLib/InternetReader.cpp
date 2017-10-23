#include "InternetReader.h"

String InternetReader::WifiData(String command, const int timeout, boolean debug)
{
	if(debug)
	{
		Serial.println(command);
	}
	
	String response = "";
	wifiSerial.print(command);
	long int time = millis();
	while ( (time + timeout) > millis())
	{
		while (wifiSerial.available())
		{
			char c = wifiSerial.read();
			response += c;
		}
	}
	
	if (debug)
	{
		Serial.print(response);
	}

	return response;
}


InternetReader::InternetReader() : wifiSerial(2, 3) {	
	wifiSerial.begin(115200);

	Serial.println("enter wifi!");

	bool debug = true;
	
	
	WifiData("AT\r\n", 100000, debug); //reset module
	WifiData("AT+RST\r\n", 100000, debug); //reset module
	WifiData("AT+CWMODE=1\r\n", 100000, debug); //set station mode

	WifiData("AT+CWLAP\r\n", 100000, debug); //List networks
	
	/*
	WifiData("AT+CWJAP=\"Tox24\",\"internet\"\r\n", 2000, debug);   //connect wifi network	

	while(!wifiSerial.find("OK")) 
	{ //wait for connection
		Serial.println("Wait for connection");
	} 

	Serial.println("Connected!!");
	WifiData("AT+CIFSR\r\n", 1000, debug); 
	WifiData("AT+CIPMUX=1\r\n", 1000, debug); 
	WifiData("AT+CIPSERVER=1,80\r\n", 1000, debug);  		
	*/
}

State InternetReader::ReadState(State state) {
    
    return state;
}