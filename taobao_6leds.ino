#include <FS.h>                   //this needs to be first, or it all crashes and burns...
//#define BLYNK_DEBUG           // Comment this out to disable debug and save space
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#define FASTLED_ESP8266_RAW_PIN_ORDER
//for LED status
#include "FastLED.h"

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <BlynkSimpleEsp8266.h>
bool shouldSaveConfig = false; //flag for saving data

//---FastLED 参数---
char blynk_token[34] = "";
#define LED_PIN 12 // ESP8266 PIN 12对应D6，根据你的情况做修改。如果你也是将LED灯带的数据线接在D6上，就无需修改。
#define NUM_LEDS 6 
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

void saveConfigCallback () {  //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void setup()
{
  delay(3000);
  Serial.begin(115200);
  Serial.println();

    //--- FastLED基本参数，LED灯数量，LED等待连接ESP8266 PIN---
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(led_bright);
  #define FRAMES_PER_SECOND 120

  fill_gradient(leds, NUM_LEDS, CHSV(50, 255,255), CHSV(100,255,255), LONGEST_HUES); 
  FastLED.show();



  //SPIFFS.format();    //clean FS, for testing
  Serial.println("Mounting FS...");    //read configuration from FS json

  if (SPIFFS.begin()) {
    Serial.println("Mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("Failed to load json config");
        }
      }
    }
  } else {
    Serial.println("Failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 33);   // was 32 length
  
  Serial.println(blynk_token);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);   //set config save notify callback

  //set static ip
  // this is for connecting to Office router not GargoyleTest but it can be changed in AP mode at 192.168.4.1
  //wifiManager.setSTAStaticIPConfig(IPAddress(192,168,10,111), IPAddress(192,168,10,90), IPAddress(255,255,255,0));
  
  wifiManager.addParameter(&custom_blynk_token);   //add all your parameters here

  //wifiManager.resetSettings();  //reset settings - for testing

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep, in seconds
  wifiManager.setTimeout(600);   // 10 minutes to enter data and then Wemos resets to try again.

  //fetches ssid and pass and tries to connect, if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("SmartLed")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }


  strcpy(blynk_token, custom_blynk_token.getValue());    //read updated parameters

  if (shouldSaveConfig) {      //save the custom parameters to FS
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  
  Blynk.config(blynk_token, "smartled.cc", 8080);
  Blynk.connect();
  Blynk.syncAll(); 
  
}

 
  void loop()
{
  FastLED.setBrightness(led_bright);
  
  Blynk.run();
  if(masterSwitch == 0) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.show();
      //delay(30);灯颜色变换延迟，去掉前面双//启用。
    }
  }
  if(autoMode == 0 && masterSwitch == 1) {
    for (int i = 0; i < NUM_LEDS; i++){
      leds[i] = CRGB(r, g, b);
      FastLED.show();
      //delay(30);灯颜色变换延迟，去掉前面双//启用。
    }
  }
  
  if(autoMode == 1 && masterSwitch == 1) {
    fill_rainbow( leds, NUM_LEDS, gHue, 7); // 彩虹模式，FastLED支持多种彩虹模式，自己可以去看FastLED wiki
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
