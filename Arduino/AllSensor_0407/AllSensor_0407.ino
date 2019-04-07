#include <PulseSensorPlayground.h>
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

//진동감지센서 설정
#include <EEPROM.h>
#define Vib D0 // 진동감지센서 
int val; //감지센서 value
//

//심박수 설정
#define UpperThreshold 550
#define LowerThreshold 500

bool BPMTiming=false;
bool BeatComplete=false;
int LastTime=0;
int BPM;
int value; 
int heartbeat;
//

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const char* ssid = "AndroidHotspot6930"; // 핫스팟 아이디
const char* password = "12345678"; // 핫스팟 비밀번호

WiFiServer server(80);

PulseSensorPlayground pulseSensor; 

void setup() {
 
  WiFi.forceSleepBegin();
  delay(1);
 
  Serial.println("Adafruit MLX90614 TEST");

  mlx.begin();
  Serial.begin(57600);
  Wire.begin(D1,D2);
  delay(10);

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
  Serial.print(WiFi.localIP()); //ip 출력해주는 함수
  Serial.println("/");
}

void loop() {

int check_sec_per_count=0;  //n초에 가까워졌는지 확인하는 변수

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

// Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); // do not forget this one
  while(true){
     val=digitalRead(Vib);
     client.print("Moving Rate= "); client.print(val);
     client.println("<br>");
     //움직임이 a분동안 없을경우 측정에 들어감.
     if(val==0){
        client.println("NOT_MOVED"); client.println("<br>");
        check_sec_per_count++;
        client.println(check_sec_per_count); client.println("<br>");
       // client.println("."); client.println("<br>");
       // client.println("."); client.println("<br>");
       // client.println("."); client.println("<br>");
        if(check_sec_per_count>20) //20초동안 움직임이 기준값 이하면
          break;
     }else{
      check_sec_per_count=0;
      client.println("MOVED! RESET TO 0..."); client.println("<br>");
     }

     delay(1000);
  }


  //심박수, 온도 측정 들어가서 바로 웹에 출력해줌
  for(int i=0;i<300;i++){

  value=analogRead(A0);
  heartbeat=int(60/(float(value)/1000));
 // client.println("<!DOCTYPE HTML>");
 // client.println("<html>");
  //client.print("Led pin is now: ");
  client.print("Ambient= "); client.print(mlx.readAmbientTempC()); //ambient 주변온도
  client.print("*C\tObject = "); client.print(mlx.readObjectTempC()); client.println("*C"); 
  // Object 재고자 하는 목적의 온도
  client.print("Ambient= "); client.print(mlx.readAmbientTempF());
  client.print("*F\tObject = "); client.print(mlx.readObjectTempF()); client.println("*F");


 //*----------------------*
 //심장센서 코드
  client.print("rare value: ");
  client.println(value);
  client.print("\theartbeat value: ");
  client.println(heartbeat);

  client.println();
  client.println("<br>");
  delay(2000);
 }

//client.println("<br><br>");
//client.println("<a href=\"/LED=ON\"\"><button>Turn on </button></a>");
//client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");
//client.println("</html>");

 
delay(1000);
client.println("<br>");
Serial.println("Client disonnected");
Serial.println("");

}
