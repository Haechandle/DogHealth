#include <ArduinoHttpClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
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

//심박수 설정
#define UpperThreshold 550
#define LowerThreshold 500

bool BPMTiming=false;
bool BeatComplete=false;
int LastTime=0;
int BPM;


//서버 관련
const char* ssid = "AndroidHotspot6930"; // 핫스팟 아이디
const char* password = "12345678"; // 핫스팟 비밀번호
//const char* host="http://192.168.43.75";
//const char* host="http://localhost";
const char* host="http://cksemf.dothome.co.kr/";
const long interval=10000; //값 넣는 시간간격
const int timecount=20; //몇초동안 안 움직이는지 측정하는 시간 변수 
unsigned long previousMillis = 0;
WiFiClient client;
WiFiServer server(80);
HTTPClient http;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseSensorPlayground pulseSensor; 
/*
bool HTTPClient::connect(void){
  if(connected()){
    end();
  }
}*/

//진동감지센서로 시간측정
void timeCount(){

  int check_sec_per_count=0;
  
   while(check_sec_per_count<timecount){
        val=digitalRead(Vib);
        if(check_sec_per_count==0){
          Serial.print("Moving Rate= "); Serial.print(val); Serial.println(" ");
          Serial.println("[NOT_MOVED]"); 
          Serial.println(" ");
        }
        if(val==1){
             check_sec_per_count=0;
             Serial.println(" ");
             Serial.print("Moving Rate= "); Serial.print(val);
             Serial.println("[MOVED! RESET TO 0...]"); Serial.println(" ");
             delay(3000);
             continue;
        }
        check_sec_per_count++;
        Serial.print("▶");
        delay(1000);
     }
        //client.println(check_sec_per_count); 
        Serial.println(" ");
        delay(1000); 
}

// 주변온도 측정을 통해 산책을 하였는지 여부를 확인하는 함수 checkifStroll

const int exerciseHeartbeat=110;
const int temp_diff=5;
float beforeOutTemp=0;
float nowOutTemp=0;
int timewalked=0; //0~86400
int term_to_second=interval/1000; //초로 변환
int haveStrolled=0; //산책여부를 나타내는 반환변수. 0은 산책x 1은 산책함 2는 산책안한지 24시간지남 
boolean daypass=false; //산책한지 24시간이 지났나를 확인하는 변수
unsigned int place=0;
int checkifStroll(){
    int beforeplace=place;
    nowOutTemp=mlx.readAmbientTempC();
    if(timewalked>86400){
      Serial.println("have been passed 24 hours since last walk");
      daypass=true;
    }
   
    int value=analogRead(A0); 
    int heartbeat=int(60/(float(value)/1000));
    
    haveStrolled=0; //디폴트(산책을 안한상태)

    if(abs(beforeOutTemp-nowOutTemp)>temp_diff){
      place+=1; //장소가 바뀌면 값이 하나씩 증가
    }
    if(place>beforeplace&&heartbeat>exerciseHeartbeat){
        haveStrolled=1; //주변온도차가 5도이상이고, 심박수가 110이상일때
        daypass=false; //24시간 지난여부 초기화
        timewalked=0; //시간 측정 다시
    }
    beforeOutTemp=nowOutTemp;
    timewalked+term_to_second=timewalked; 
    if(daypass==true){
      haveStrolled=2;
    }
    return haveStrolled;  
}


void setup() {
 
  //WiFi.forceSleepBegin();
  delay(1);

  mlx.begin();
  Serial.begin(57600);
  Wire.begin(D1,D2);
  delay(10);
  
  beforeOutTemp=mlx.readAmbientTempC(); //초기 외부값

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("http://");
  server.begin();
  Serial.println("Server started");

  timeCount();
}

void loop() {

    val=digitalRead(Vib);
    if(val>0){
      Serial.println("moved");
      delay(3000);
        val=digitalRead(Vib);
        if(val>0){
            Serial.println("Awaken");
            timeCount();
         }
      
   }
    
    unsigned long currentMillis=millis();
    if(currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

    int value=analogRead(A0); 
    int heartbeat=int(60/(float(value)/1000));
    int heart = heartbeat; //심장박동수
    float temp = mlx.readObjectTempC(); //체온
    
    int strolled= checkifStroll();
    
    String phpHost =host;
    phpHost+="/input.php?temp=" + String(temp) + "&heart=" + String(heart);
   //phpHost+="/input.php?temp=" + String(temp) + "&heart=" + String(heart)+"&walk="+String(strolled);
  
    Serial.print("Connect to ");
    Serial.println(phpHost);
    
    http.begin(client, phpHost);
   
    http.setTimeout(10000);
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
  int value=analogRead(A0); 
  int heartbeat=int(60/(float(value)/1000));
  int heart = heartbeat;
  float temp = mlx.readObjectTempC();

  
  client.print("HTTP/1.1 200 OK");
  client.print("Content-Type: text/html\r\n\r\n");
  client.println("");
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
  
  Serial.println("클라이언트 연결 해제");

}
