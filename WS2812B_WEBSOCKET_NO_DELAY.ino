/*****************  NEEDED TO MAKE NODEMCU WORK ***************************/
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ESP8266_RAW_PIN_ORDER
/******************  LIBRARY SECTION *************************************/
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>

#include <FS.h>  

/*****************  LED LAYOUT AND SETUP *********************************/
#define NUM_LEDS 300
/*****************  DECLARATIONS  ****************************************/
CRGB leds[NUM_LEDS];
WiFiClient espClient;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
/*****************  GLOBAL VARIABLES  ************************************/
const int ledPin = 4; //marked as D2 on the board
const String ssid = "ssid";
const String password = "password";
int ledMode = 0;
float colorPicker1[] = {0,0,0};
float colorPicker2[] = {0,0,0};
CRGB colorPicker1CRGB = CRGB(0,0,0);
CRGB colorPicker2CRGB = CRGB(0,0,0);
boolean t = false;
unsigned long wait = 200;
unsigned long lastUpdate = 0;

/************************** ALL OFF  ****************************************/
void all_off(){
      fill_solid(&leds[0],300,CRGB(0,0,0));
      FastLED.show();
}
/**************************  ROOT AND COLOR ****************************************/



/************************** SETUP OTA  ****************************************/
void OTASetup(){
  ArduinoOTA.setHostname("ws2812");
  ArduinoOTA.setPassword("passwordpassword");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  }

//setup_wifi

/************************** GLOW  ****************************************/

  
void glowUpdate(){
  static uint16_t cb = 0;
  unsigned long now = millis();

  //gotta love 10 variable declarations in a function
  if(now > lastUpdate+wait){

    if(cb <= 255){
      float r = colorPicker1[0] - (cb * (float)(colorPicker1[0]/255));
      float g = colorPicker1[1] - (cb * (float)(colorPicker1[1]/255));
      float b = colorPicker1[2] - (cb * (float)(colorPicker1[2]/255));
      Serial.printf("r: %f g: %f b: %f",r,g,b);
      Serial.println("");
      fill_solid(&leds[0],300,CRGB(r,g,b));
      } else {
      float r = (cb-255)* (float)(colorPicker1[0]/255);
      float g = (cb-255)* (float)(colorPicker1[1]/255);
      float b = (cb-255)* (float)(colorPicker1[2]/255);
      Serial.printf("r: %f g: %f b: %f",r,g,b);
      Serial.println("");
      fill_solid(&leds[0],300,CRGB(r,g,b));
      }
    FastLED.show();
    cb = (510<=cb ? 0 : cb+1);
    lastUpdate = now;
    }      
  }
/************************** FADE BETWEEN  ****************************************/

  
void fadeUpdate(){
 

  unsigned long now = millis();
  if(now > lastUpdate+wait){
    fill_gradient_RGB(&leds[0],300,colorPicker1CRGB,colorPicker2CRGB);
    FastLED.show();
    lastUpdate = now;
    
    
    }

  
  }

/************************** SOLID BETWEEN  ****************************************/


void solidUpdate(){
  fill_solid(&leds[0],300,CRGB(colorPicker1[0],colorPicker1[1],colorPicker1[2]));
  FastLED.show();
  }  
/************************** RAINBOW BETWEEN  ****************************************/

void rainbowUpdate(){


  unsigned long now = millis();
  
  if((unsigned long)(now > lastUpdate+wait)){
    uint8_t p = beat8(60000/wait);
    fill_rainbow(leds, 300, p, 1);
    lastUpdate = now;
    Serial.println(p);

    FastLED.show();
  }
}

/************************** WIFI  ****************************************/
void setup_wifi(){
  Serial.print("Connecting to");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
    
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
    
   if(!MDNS.begin("ws2812")){
    Serial.println("MDNS Error!");
    }
   else{Serial.println("MDNS active!");
   } 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
  server.onNotFound([]() {
    File main = SPIFFS.open("/home.html","r");
    server.streamFile(main, "text/html");
    main.close();
    });
  server.on("/WebSocket.js", handleJS);
  server.begin();
  }
  
//function to send javascript to browser
void handleJS(){
    File js = SPIFFS.open("/WebSocket.js","r");
    server.streamFile(js, "text/javascript");
    js.close(); 
  }
  
void setup_websocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketHandle);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

/************************** WEBSOCKET HANDLER ****************************************/

void webSocketHandle(uint8_t num, WStype_t type, uint8_t* payload, size_t len){
  Serial.println("webSocket event");
  switch(type){
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        break;
        //all_off();                  // turn all off when websocket connected
      }
    case WStype_TEXT:
      switch(payload[0]) { 
        case '#': {          
          long a = strtol((const char *) &payload[2], nullptr, 16);
          int r = (a >> 16);
          int b = (((a >> 8) << 8)) ^ a;
          int g = ((a) ^ ((r << 16) | b)) >> 8;
          if(payload[1] == '1'){
                colorPicker1[0] = r;
                colorPicker1[1] = g;
                colorPicker1[2] = b;
                colorPicker1CRGB = CRGB(r,g,b);
                Serial.printf("#1 r: %d g: %d b: %d\n", r,g,b);
            } else {
                colorPicker2[0] = r;
                colorPicker2[1] = g;
                colorPicker2[2] = b;
                colorPicker2CRGB = CRGB(r,g,b);
                Serial.printf("#2 r: %d g: %d b: %d\n", r,g,b);
              }

          
          break;
        
    }
    case 'g': {
      all_off();
      ledMode = 1;
      Serial.println("glow pressed");
      break;
      }
      
    case 'f': {
      all_off();
      ledMode = 2;
      Serial.println("fade pressed");
      break;
      }
      
    case 's': {
      all_off();
      ledMode = 3;
      Serial.println("solid pressed");
      break;
      }
    case 't': {
      all_off();
      ledMode=0;
      Serial.println("toggle pressed");
      break;
      }
    case 'r': {
      all_off();
      ledMode = 4;
      Serial.println("rainbow pressed");
      break;
      }
    case 'D': {
      //delay
      wait = strtol ((const char *) &payload[1], nullptr, 0);
      Serial.printf("Delay updated %d\n",wait);
      break;
      }
  }
  }
}
/************************** CONTROL ****************************************/

void control(int i){
  switch(i){
    case 0:
      break;
    case 1:
      //all_off();
      glowUpdate();
      break;
    case 2:
      fadeUpdate();
      break;
    case 3:
      solidUpdate();
      break;
    case 4:
      rainbowUpdate();
      break;
    }
  }
/*****************  SETUP FUNCTIONS  ****************************************/
void setup() 
{
  delay(2000);
  Serial.begin(9600);
  SPIFFS.begin();
  setup_wifi();
  setup_websocket();
  OTASetup();
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255); 
}
/*****************  MAIN LOOP  ****************************************/
void loop() 
{
  
  webSocket.loop();
  MDNS.update();
  server.handleClient();
  ArduinoOTA.handle();
  control(ledMode);

  
  
}
