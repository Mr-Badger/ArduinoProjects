/****************************************************************
* ESP8266
* Arduino Mega Using Serial2
* AT commands examples for test:
*    AT     =====> ESP8266 returns OK
*    AT+RST =====> ESP8266 restart and returns OK
*    AT+GMR =====> ESP8266 returns AT Version; SDK version; id; OK
*    AT+CWMODE? => ESP8266 returns mode type
*    AT+CWLAP ===> ESP8266 returs close access points
*    AT+CIFSR ===> ESP8266 returs designided IP
*    AT+CIPSERVER=1,4040: set modo SERVER port: 4040
*    AT+CWMODE=3 ==> Conect ESP8266 ascombined mode (Access Point and Server)
*    AT+CWSAP="Acc_Point_name","password",wifi_Channel,cript# ==> ej. AT+CWSAP="ESP_8266_AP,"1234",3,0
*    AT+CWJAP="SSID","password" ==> Connect to WiFi network
*    AT+CWJAP="ROVAI TIMECAP","mjr747@1"
*
* Marcelo Jose Rovai 14Jan16
******************************************************************/

#define esp8266 Serial2
#define CH_PD 4 
#define speed8266 115200 // This is the speed that worked with my ESP8266
void setup() 
{
  esp8266.begin (speed8266); 
  Serial.begin(9600);
  reset8266(); // Pin CH_PD need a reset before start communication
}

void loop() 
{
  while(esp8266.available())
  {
    Serial.write(esp8266.read());
  }
  while(Serial.available())
  {
    esp8266.write(Serial.read());
  }
}

/*************************************************/
// Reset funtion to accept communication
void reset8266 ()
{
  pinMode(CH_PD, OUTPUT);
  digitalWrite(CH_PD, LOW);
  delay(300);
  digitalWrite(CH_PD, HIGH);
}
