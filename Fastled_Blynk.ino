/*************************************************************
 一个送给妹子的礼物
 注意:带*的位置，必须修改为你自己的参数。
 资料来源于:https://www.makeuseof.com/tag/computer-lighting-nodemcu-wifi/
 blynk:https://www.blynk.cc
 arduino:https://www.arduino.cc/
 FastLED:http://fastled.io/
*************************************************************/

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "FastLED.h"

//---FastLED 参数---
#define LED_PIN 12 // ESP8266 PIN 12对应D6，根据你的情况做修改。如果你也是将LED灯带的数据线接在D6上，就无需修改。
#define NUM_LEDS 120 //* LED灯数量，根据你的情况做修改。
//#define BRIGHTNESS 255 // LED亮度，最高255，此选项因增加亮度控制器，已无用。
#define LED_TYPE WS2812B //灯带型号，一般不需要改变
#define COLOR_ORDER GRB //颜色指令
CRGB leds[NUM_LEDS];

//---RGB颜色值 (0-255) ---
int r = 255;
int g = 255;
int b = 255;
int led_bright = 128;

//--主开关和彩虹模式变量
int masterSwitch = 1;
int autoMode = 1;

//--- 色调值---
uint8_t gHue = 0; 

//---WIFI设置和BLYNK服务器和验证码---
char auth[] = "";//* blynk验证码，在blynk APP中获取
char ssid[] = ""; //* 你家WIFI的名称
char pass[] = ""; //* 你家WIFI密码
char server[] = "smartled.cc";  //BLYNK服务器地址，请勿修改，除非你有自己的BLYNK服务器
int port = 8080; //ESP8266连接BLYNK服务器端口，请勿修改

void setup() {

  // 开关保护延迟
  delay(2000);
  Serial.begin(9600);

  //--- FastLED基本参数，LED灯数量，LED等待连接ESP8266 PIN---
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(led_bright);
  #define FRAMES_PER_SECOND 120

  //---ESP8266 WIFI设置和Blynk验证码以及服务器地址 ---
  Blynk.begin(auth, ssid, pass, server, port);

  
}
void(* resetFunc) (void) = 0;

BLYNK_CONNECTED() {
  Blynk.syncAll();  
}



void loop() {

  FastLED.setBrightness(led_bright);
  
  Blynk.run();
  if(masterSwitch == 0) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
      //delay(30);
    }
  }
  if(autoMode == 0 && masterSwitch == 1) {
    for (int i = 0; i < NUM_LEDS; i++){
      leds[i] = CRGB(r, g, b);
      FastLED.show();
      //delay(30);
    }
  }
  
  if(autoMode == 1 && masterSwitch == 1) {
    fill_rainbow( leds, NUM_LEDS, gHue, 7); 
    FastLED.show(); 
    FastLED.delay(1000/FRAMES_PER_SECOND); 
    EVERY_N_MILLISECONDS(20) {
      gHue++; 
    } 
  }
}

//---主开关 Button---
BLYNK_WRITE(V0) {
  masterSwitch = param.asInt();
}

//--- 红色 色值控制 Slider ---
BLYNK_WRITE(V1) {
  r = param.asInt();
}

//--- 绿色 色值控制 Slider ---
BLYNK_WRITE(V2) {
  g = param.asInt();
}

//--- 蓝色 色值控制 Slider ---
BLYNK_WRITE(V3) {
  b = param.asInt(); 
}

//--- 彩虹/手动控制颜色 Button ---
BLYNK_WRITE(V4) {
  autoMode = param.asInt();
}

//--- 亮度 Slider ---
BLYNK_WRITE(V5) {
  led_bright = param.asInt();
}
