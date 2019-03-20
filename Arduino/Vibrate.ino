#include <ESP8266WiFi.h>
#include <EEPROM.h>
int val;
#define USE_SERIAL Serial
#define Vib A0

void setup()
{

  Serial.begin(115200);
  delay(10);
}

void loop()
{
  val=analogRead(Vib);
  Serial.print(val);
  Serial.println(" ");
  delay(100);  
  
}
