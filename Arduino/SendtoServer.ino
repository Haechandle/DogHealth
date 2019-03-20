#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <Wire.h>

#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const char* ssid = "AndroidHotspot6930";
const char* password = "12345678";

int ledPin = 2;

WiFiServer server(80);

void setup() {
  WiFi.forceSleepBegin();
  delay(1);
 
 
  Serial.println("Adafruit MLX90614 TEST");

  mlx.begin();
  Serial.begin(115200);
  Wire.begin(D1,D2);
  delay(10);

pinMode(ledPin, OUTPUT);
digitalWrite(ledPin, HIGH);

 

// Set WiFi to station mode and disconnect from an AP if it was previously connected
WiFi.mode(WIFI_STA);
WiFi.disconnect();

delay(100);

Serial.println("Setup done");

 

// Connect to WiFi network

Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);

 

WiFi.begin(ssid, password);

 
while (WiFi.status() != WL_CONNECTED) {

delay(500);

Serial.print(".");

}

Serial.println("");
Serial.println("WiFi connected");

 

// Start the server

server.begin();
Serial.println("Server started");

 

// Print the IP address

Serial.print("Use this URL to connect: ");
Serial.print("http://");
Serial.print(WiFi.localIP());
Serial.println("/");

 
}

 

void loop() {

// Check if a client has connected

WiFiClient client = server.available();
if (!client) {
  return;
}

// Wait until the client sends some data
Serial.println("new client");
while(!client.available()){
  delay(1);
}

 

// Read the first line of the request

String request = client.readStringUntil('\r');
Serial.println(request);
client.flush();

// Match the request

int value = LOW;
if (request.indexOf("/LED=ON") != -1) {
  digitalWrite(ledPin, LOW);
  value = HIGH;
}

if (request.indexOf("/LED=OFF") != -1) {
  digitalWrite(ledPin, HIGH);
  value = LOW;
}

// Set ledPin according to the request
//digitalWrite(ledPin, value);

 

// Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); // do not forget this one
  for(int i=0;i<100;i++){

 // client.println("<!DOCTYPE HTML>");
 // client.println("<html>");
  //client.print("Led pin is now: ");
  client.print("Ambient= "); client.print(mlx.readAmbientTempC()); //ambient 주변온도
  client.print("*C\tObject = "); client.print(mlx.readObjectTempC()); client.println("*C"); 
  // Object 재고자 하는 목적의 온도
  client.print("Ambient= "); client.print(mlx.readAmbientTempF());
  client.print("*F\tObject = "); client.print(mlx.readObjectTempF()); client.println("*F");

  client.println();
  client.println("<br>");
  delay(1000);
  }
/*if(value == HIGH) {
client.print("On");
} else {
client.print("Off");
}*/

//client.println("<br><br>");
//client.println("<a href=\"/LED=ON\"\"><button>Turn on </button></a>");
//client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");
//client.println("</html>");

 

delay(1);
client.println("<br>");
Serial.println("Client disonnected");
Serial.println("");

}
