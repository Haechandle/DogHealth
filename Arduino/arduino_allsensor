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
const char* ssid = "iPhone"; // 핫스팟 아이디
const char* password = "0123456789"; // 핫스팟 비밀번호
const char* host=""; //서버주소
const long interval=5000;
unsigned long previousMillis = 0;
WiFiClient client;
WiFiServer server(80);
HTTPClient http;

Adafruit_MLX90614 mlx = Adafruit_MLX90614(); //체온 라이브러리
PulseSensorPlayground pulseSensor; // 심장박동 라이브러리

void setup() {
  
  delay(1);
 
  Serial.println("Adafruit MLX90614 TEST");

  mlx.begin(); //체온 측정
  Serial.begin(57600);
  Wire.begin(D1,D2);
  delay(10);

// Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);

  Serial.println("Setup done");

// 와이파이와 연결

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
  
// 서버 연결

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

unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

  while(true){
     val=digitalRead(Vib);
     client.print("Moving Rate= "); client.print(val);
     client.println("<br>");
     //움직임이 20초 동안 없을경우 측정에 들어감.
     if(val==0){ //val==0이면 움직이지 않는것
        client.println("NOT_MOVED"); client.println("<br>");
        check_sec_per_count++; //시간체킹
        client.println(check_sec_per_count); client.println("<br>"); //시간체킹출력
        client.println("."); client.println("<br>");
        client.println("."); client.println("<br>");
        client.println("."); client.println("<br>");
        if(check_sec_per_count>20) //20초동안 움직임이 기준값 이하면
          break;
     }else{
      check_sec_per_count=0;
      client.println("MOVED! RESET TO 0..."); client.println("<br>");
     }

     delay(1000);
  }


  //심박수, 온도 측정 들어가서 바로 웹에 출력해줌
    int value=analogRead(A0); 
    int heartbeat=int(60/(float(value)/1000)); //심장박동 수 계산
    float heart = heartbeat; //심장박동수 값
    float temp = mlx.readObjectTempC(); //체온 값
    
    String phpHost =host; //php부분
    phpHost+="/doginsert.php?temp=" + String(temp) + "&heart=" + String(heart);
    Serial.print("Connect to ");
    Serial.println(phpHost);
    
    http.begin(client, phpHost);
   
    http.setTimeout(8000);
    int httpCode = http.GET();
    
    if(httpCode > 0) {
      Serial.printf("GET code : %d\n\n", httpCode);

      if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } 
    else {
      Serial.printf("GET failed, error no: %d, error: %s\n", httpCode , http.errorToString(httpCode).c_str());
    }
    http.end();
    }

 // 웹서버 부분
  client = server.available();
  if(!client) return;

  Serial.println("새로운 클라이언트");
  client.setTimeout(5000);

  String request = client.readStringUntil('\r');
  Serial.println("request: ");
  Serial.println(request);

  while(client.available()) {
    client.read();
  }
  
  
  client.print("HTTP/1.1 200 OK");
  client.print("Content-Type: text/html\r\n\r\n");
  client.print("<!DOCTYPE HTML>");
  client.print("<html>");
  client.print("<head>"); 
  client.print("<meta charset=\"UTF-8\" http-equiv=\"refresh\" content=\"1\">");
  client.print("<title>DHT senrsor test Webpage</title>");
  client.print("</head>");
  client.print("<body>");
  client.print("<h2>DHT senrsor test Webpage</h2>");
  client.print("<br>");
  client.print("Temperature : ");
  client.print(temp);
  client.print(" °C");
  client.print("<br>");
  client.print("Heart : ");
  client.print(heart);
  client.print(" BPM");
  client.print("</body>");
  client.print("</html>");
  delay(5000);
  Serial.println("클라이언트 연결 해제");

}
