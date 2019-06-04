/* 기존기능
1. 움직임이 20초동안 없으면 헬스데이터 측정에 돌입
2. 비정상 값이 들어오면 그값을 포함한 3번의 측정까지 값을 DB로 보내지 않고 측정하다가 4번째값에도 비정상값이 들어오면 신뢰도가 높은 비정상값으로 인지하고 디비로 전송
3. 측정중 움직였을때 수면상태인지 자다가 뒤척인 상태인지 측정하는 시간텀을 3초
등등
*/

/* #06.05# 수정사항
1. 숙면여부 측정 -자는지 체크하는 함수, 자고있을때 수행하는 함수
2. 측정중 움직였을때 수면상태인지 자다가 뒤척인 상태인지 측정하는 코드를 함수로 묶고 숙면여부 측정에 활용
등등

test 결과: 정상작동
*/

/* 시리얼 모니터 설명
1. awaken count가 깬 횟수고요. 지금 숙면기준을 5번으로 해놨는데 깬횟수가 6회가 되고 잠에서 깨어나면 잠을 잘 못잤다고 판단해서 -1을 전송합니다. 반대의 경우는 +1을 전송하고요. 그밖에는 0을 넣습니다.
2. 숙면의 심박수 범위는 현재 102~110입니다. 테스트용으로 이렇게설정했는데 바꿀수있습니다
3. Time은 초단위로 숙면시간을 잽니다. DB로 보내려면 설정할수 있고요. 현재는 그냥 아두이노내에서 계산만 합니다.
4. Count none sleeping heartRate는 아까 숙면 심박수 범위를 설정힌 102~110 바깥의 값이 측정됐을때 증가합니다. 현재로썬 움직임으로 수면에서 깨어남을 측정하기 때문에 이거로는 따로 수면여부를 측정하지 않고, 심박수가 수면범위내에 일정하게 나오는걸 확인하기 위해 사용합니다. ex) 수면범위에 있는 값이 한번만 측정되면 수면모드로 안치지만 3번연속으로 측정되면 수면모드로 간주.
*/

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

int count_ab=0;// 값의 신뢰도를 위한 변수

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

//진동감지센서로 시간측정
void timeCount(){
  count_ab=0;
  int check_sec_per_count=0;
  Serial.println("<<TimeCount>>");
   while(check_sec_per_count<timecount){
        val=digitalRead(Vib);
       
        if(check_sec_per_count==0){
            Serial.print("Moving Rate= "); Serial.print(val); Serial.println(" ");
            Serial.println("[NOT_MOVED]"); 
            Serial.println(" ");
        }
        if(val==1){
            Serial.println(" ");
            Serial.print("Moving Rate= "); Serial.print(val);
            Serial.println("[MOVED! RESET TO 0...]"); Serial.println(" ");
            delay(1000);
            timeCount();
            break;
        }
        check_sec_per_count++;
        Serial.print("▶");
        delay(1000);
     }
        //client.println(check_sec_per_count); 
        Serial.println(" ");
        delay(1000); 
}

/*
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
*/
//이상값 설정
const int abnormal_heart_low=100;
const int abnormal_heart_high=140;
const int abnormal_temp_low=37;
const int abnormal_temp_high=39;

//수면중인지 확인하는 함수

const int averageBeat_whileSleeping=105; //논문에 따르면 평균 심박수 * 0.9 값이 여기 들어감. 우선 평균심박수를 구해야됨
const int temp_whileSleeping=5; //온도. 온도로도 자는것을 구분할 수 있는지 확인하고 사용.
int really_Sleeping=0;
//int really_Awake=0;
void is_Sleeping(int heartBeat){
  // +- 5의 범위
    if(heartBeat<=averageBeat_whileSleeping+5&&heartBeat>=averageBeat_whileSleeping-3){
      really_Sleeping++; //이게 4이상되어야 수면중이라 판단.
    }
}
int moved_while_Sleeping=0; // 움직이면 1, 깨어나면 -1

void setup() {
 
  //WiFi.forceSleepBegin();
  delay(1);

  mlx.begin();
  Serial.begin(57600);
  Wire.begin(D1,D2);
  delay(10);
  
  //beforeOutTemp=mlx.readAmbientTempC(); //초기 외부값

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

//숙면 관련 변수들
int awaken_Count=0; //자다가 깬 횟수
const int well_Slept = 5; //깬 횟수가 well_Sleeped의 수를 넘지 않으면 숙면을 취했다고 가정.
int how_much_time_Slept=0; //0~86400
int term_to_second=interval/1000;

void loop() {
  
    // 측정중에 움직임이 감지되면 깨어나서 움직이는건지 자면서 뒤척이는건지 판단
    val=digitalRead(Vib);
    if(val>0){
      Serial.println("moved");
      delay(3000);
      val=digitalRead(Vib);
      moved_while_Sleeping = 1; //움직이면 1반환
      delay(500);
      if(val>0){
           Serial.println("Awaken");
           timeCount();
           moved_while_Sleeping = -1; //깨어났기 때문에 -1반환
           delay(500);
      }
    }
   
    /*else{
       moved_while_Sleeping = 0; //안움직이면 0반환
    }*/

    unsigned long currentMillis=millis();
    if(currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      
      int value=analogRead(A0); 
      int heartbeat=int(60/(float(value)/1000));
      int heart = heartbeat; //심장박동수
      float temp = mlx.readObjectTempC(); //체온
    
    //int strolled= checkifStroll(); //산책 여부 값 저장
    
      count_ab++;
      String phpHost=host;
     
      is_Sleeping(heart);
      // 자고있을때. 이것으로 측정하여 숙면시간과 숙면시간중 깬 횟수를 count.
      while(really_Sleeping>3){
            delay(10);
            Serial.println("<<<<<<<<Sleeping Case>>>>>>>>");Serial.println();   //Test 시리얼 모니터.
            Serial.print("Time: "); Serial.print(how_much_time_Slept);
            Serial.print("     Awaken count: "); Serial.println(awaken_Count);
                       
            Serial.print("Count none sleeping heartRate: "); Serial.println(really_Sleeping);
            
            how_much_time_Slept+=term_to_second; //총 수면시간 측정
            delay(1000);
            int moved=moved_while_Sleeping;
            if(moved==1){
                awaken_Count+=1;
            }
            //일어났을때
            if(moved==-1){
                int goodOrbad=0;
                if(awaken_Count<=well_Slept){
                    goodOrbad=1; //잘잤을때 1을 보냄. 기준은 well_Slept 값보다 덜 깼을때
                    phpHost+="/input.php?temp=" + String(temp) + "&heart=" + String(heart) + "&goodOrbad=" + String(goodOrbad);
                }else{
                    goodOrbad=-1; //잘못잤을때 0을 보냄. 기준은 well_Slept 값보다 더 깼을때
                    phpHost+="/input.php?temp=" + String(temp) + "&heart=" + String(heart) + "&goodOrbad=" + String(goodOrbad);
                }
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
            moved_while_Sleeping=0;
            how_much_time_Slept=0;
            awaken_Count=0;
            really_Sleeping=0;
            break;
          }
     break;
     }
      // 쉬고있을때. 정상값일땐 바로 들어가고 이상값일땐 넘어감
     while(heart>=abnormal_heart_low&&heart<=abnormal_heart_high&&temp>=abnormal_temp_low&&temp<=abnormal_temp_high){
        count_ab=0;
    
        phpHost+="/input.php?temp=" + String(temp) + "&heart=" + String(heart) + "&goodOrbad=" + "0";
      
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
         break;
      }
      Serial.print("Count:");Serial.print(count_ab);
      Serial.print("\tHeart:");Serial.print(heart);
      Serial.print("\tTemp:");Serial.println(temp);
      if(count_ab>3){ //신뢰도 높은 이상값
         Serial.println("Abnormal Value!!");
         phpHost+="/input.php?temp=" + String(temp) + "&heart=" + String(heart) + "&goodOrbad=" + "0";
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
      
     
     //phpHost+="/input.php?temp=" + String(temp) + "&heart=" + String(heart)+"&walk="+String(strolled);
     //산책여부 변수도 같이 보내주는 것.
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
