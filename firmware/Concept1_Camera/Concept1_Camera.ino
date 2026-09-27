// Team 2 Concept 1. LD2450 -> D7/GPIO20 RX, D6/GPIO21 TX, 5V, GND.
// Common-cathode RGB: D0/GPIO2 red, D1/GPIO3 green, D2/GPIO4 blue.
// Each LED anode needs its own current-limiting resistor. LED is optional for USB testing.
// OOCSI TCP wire protocol follows the official iddi/oocsi-python client (MIT).
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Preferences.h>

struct Target { int x, y, speed, resolution; bool present; };
struct Reading { Target targets[3]; bool valid; int x,y,count; uint32_t seq,frame,ms; };
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;
Reading shared = {};
char guideState[12]="stop";
char gameState[12] = "idle";
char gameSession[48] = "";
uint32_t gameBeat=0, heartbeatAt=0, guideAt=0;
bool haveHeartbeat=false, netEnabled=false, netConnected=false;
char wifiSSID[33]="", wifiPassword[65]="", teamChannel[100]="";
char bootID[16];
uint8_t frameBuffer[30]; size_t used=0;
Target latest[3] = {};
uint32_t frames=0, lastFrameAt=0, sequence=0;
String serialLine;
Preferences prefs;
#include "wifi_setup.h"

int word(const uint8_t*p) { return p[0] | (p[1]<<8); }
int signedValue(const uint8_t*p) { int v=word(p); return (v&0x8000)?(v&0x7fff):-(v&0x7fff); }

void heartbeat(JsonVariantConst d) {
  if(!d["toktik_beat"].is<uint32_t>() || !d["toktik_state"].is<const char*>()) return;
  const char* s=d["toktik_state"]; const char* session=d["toktik_session"] | "";
  if(!*session || strlen(session)>=sizeof(gameSession)) return;
  if(strcmp(s,"idle")&&strcmp(s,"running")&&strcmp(s,"won")&&strcmp(s,"lost")&&strcmp(s,"fault")) return;
  uint32_t beat=d["toktik_beat"];
  portENTER_CRITICAL(&mutex);
  if(strcmp(session,gameSession)!=0 || !haveHeartbeat || beat>gameBeat) {
    strlcpy(gameSession,session,sizeof(gameSession)); strlcpy(gameState,s,sizeof(gameState));
    const char* guide=d["toktik_guide"] | "stop";
    if(strcmp(guide,"left")&&strcmp(guide,"right")&&strcmp(guide,"forward")&&strcmp(guide,"back")) guide="stop";
    if(strcmp(guideState,guide))guideAt=millis();
    strlcpy(guideState,guide,sizeof(guideState));
    gameBeat=beat; heartbeatAt=millis(); haveHeartbeat=true;
  }
  portEXIT_CRITICAL(&mutex);
}

void serialCommand() {
  while(Serial.available()) {
    char c=Serial.read();
    if(c=='\n') {
      StaticJsonDocument<2048> d;
      if(!deserializeJson(d,serialLine)) {
        const char* command=d["command"] | "";
        if(!strcmp(command,"setup_info")) {
          StaticJsonDocument<256> info;info["type"]="setup";info["ssid"]=setupSSID;info["password"]=setupPassword;info["url"]="http://192.168.4.1";serializeJson(info,Serial);Serial.println();
        }
        if(!strcmp(command,"heartbeat")) heartbeat(d.as<JsonVariantConst>());
        if(!strcmp(command,"wifi_config")) {
          const char* ssid=d["ssid"] | ""; const char* password=d["password"] | "";
          const char* channel=d["channel"] | "";
          if(strlen(ssid)>0 && strlen(ssid)<=32 && strlen(password)<=64 && strlen(channel)>0 && strlen(channel)<100 && !strchr(channel,' ') && !strchr(channel,'\n')) {
            prefs.putString("ssid",ssid); prefs.putString("password",password); prefs.putString("channel",channel);
            prefs.putBool("enabled",d["enabled"] | false);
            prefs.putBool("wifi_on",true);
            Serial.println("{\"type\":\"config\",\"saved\":true,\"restarting\":true}"); Serial.flush(); delay(100); ESP.restart();
          }
        }
        if(!strcmp(command,"wifi_disable")) { prefs.putBool("enabled",false); prefs.putBool("wifi_on",false); Serial.println("{\"type\":\"config\",\"wifi_disabled\":true}"); Serial.flush(); delay(100); ESP.restart(); }
      }
      serialLine="";
    } else if(c!='\r') { if(serialLine.length()<2047) serialLine+=c; else serialLine=""; }
  }
}

void radarRead() {
  while(Serial1.available()) {
    uint8_t b=Serial1.read();
    if(used==30) { memmove(frameBuffer,frameBuffer+1,29); used=29; }
    frameBuffer[used++]=b;
    if(used==30 && frameBuffer[0]==0xAA && frameBuffer[1]==0xFF && frameBuffer[2]==3 && frameBuffer[3]==0 && frameBuffer[28]==0x55 && frameBuffer[29]==0xCC) {
      for(int i=0;i<3;i++) { const uint8_t*p=frameBuffer+4+i*8; bool present=false; for(int j=0;j<8;j++) present|=p[j]!=0;
        latest[i]={signedValue(p),signedValue(p+2),signedValue(p+4),word(p+6),present}; }
      frames++; lastFrameAt=millis(); used=0;
    }
  }
}

void payload(JsonDocument& d, const Reading&r) {
  d["guidance_supported"]=true;
  d["type"]="radar"; d["radar_boot"]=bootID; d["radar_seq"]=r.seq; d["radar_frame"]=r.frame;
  d["radar_device_ms"]=r.ms; d["radar_valid"]=r.valid; d["radar_x_mm"]=r.x; d["radar_y_mm"]=r.y; d["radar_target_count"]=r.count;
}

void networkTask(void*) {
  // Network timeouts run on their own task so UART reading and LED deadlines continue.
  beginSetupPortal();
  WiFiClient client; client.setTimeout(1000);
  String input; uint32_t lastTry=0, sentSeq=0, lastByte=millis(); bool greeted=false;
  String handle=String("team2_camera_")+bootID;
  while(true) {
    serviceSetupPortal();
    if(!netEnabled){netConnected=false;client.stop();greeted=false;vTaskDelay(pdMS_TO_TICKS(10));continue;}
    if(WiFi.status()!=WL_CONNECTED) { netConnected=false; client.stop(); greeted=false; if(millis()-lastTry>10000){lastTry=millis();WiFi.reconnect();} }
    else if(!client.connected()) {
      netConnected=false; greeted=false; input="";
      if(millis()-lastTry>3000) { lastTry=millis(); if(client.connect("oocsi.id.tue.nl",4444,1000)) {client.println(handle+"(JSON)");lastByte=millis();} }
    } else {
      while(client.available()) {
        char c=client.read(); lastByte=millis();
        if(c=='\n') {
          if(input.startsWith("ping")||input.startsWith(".")) client.println(".");
          else if(input.startsWith("{")) {
            StaticJsonDocument<2048> d;
            if(!deserializeJson(d,input)) {
              if(!greeted) {greeted=true;netConnected=true;client.println(String("subscribe ")+teamChannel);}
              else if(d["recipient"]==teamChannel) heartbeat(d.as<JsonVariantConst>());
            }
          } else if(input.startsWith("error")) client.stop();
          input="";
        } else if(c!='\r') { if(input.length()<2047) input+=c; else {input="";client.stop();} }
      }
      if(!greeted && millis()-lastByte>3000) client.stop();
      if(millis()-lastByte>30000) client.stop();
      Reading r; portENTER_CRITICAL(&mutex); r=shared; portEXIT_CRITICAL(&mutex);
      if(greeted && r.seq!=sentSeq) {
        StaticJsonDocument<768> d; payload(d,r); String out; serializeJson(d,out);
        client.println(String("sendraw ")+teamChannel+" "+out); sentSeq=r.seq;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200); Serial1.setRxBufferSize(2048); Serial1.begin(256000,SERIAL_8N1,20,21);
  pinMode(2,OUTPUT);pinMode(3,OUTPUT);pinMode(4,OUTPUT);
  digitalWrite(2,LOW);digitalWrite(3,LOW);digitalWrite(4,LOW);
  snprintf(bootID,sizeof(bootID),"%08lx",(unsigned long)esp_random());
  prefs.begin("team2camera",false);
  strlcpy(wifiSSID,prefs.getString("ssid","").c_str(),sizeof(wifiSSID));
  strlcpy(wifiPassword,prefs.getString("password","").c_str(),sizeof(wifiPassword));
  strlcpy(teamChannel,prefs.getString("channel","").c_str(),sizeof(teamChannel));
  netEnabled=prefs.getBool("enabled",false)&&strlen(wifiSSID)&&strlen(teamChannel);
  xTaskCreate(networkTask,"oocsi",8192,nullptr,1,nullptr);
}

void loop() {
  radarRead(); serialCommand(); uint32_t now=millis(); static uint32_t lastPublish=0;
  bool valid=frames && now-lastFrameAt<400; int count=0,x=0,y=0;
  for(auto&t:latest) if(valid&&t.present&&t.x>=-1500&&t.x<=1500&&t.y>=1500&&t.y<=4500) {count++;x=t.x;y=t.y;}
  valid=valid&&count==1;
  char state[12],guide[12]; uint32_t age,cueAge; bool heard;
  portENTER_CRITICAL(&mutex); strlcpy(state,gameState,sizeof(state));strlcpy(guide,guideState,sizeof(guide)); age=now-heartbeatAt;cueAge=now-guideAt;heard=haveHeartbeat; portEXIT_CRITICAL(&mutex);
  bool running=!strcmp(state,"running"), fault=!strcmp(state,"fault");
  bool green=running&&valid&&heard&&age<2000;
  bool amber=(running&&!green)||fault;
  bool flash=amber&&((now/300)%2==0);
  int pulses=!strcmp(guide,"left")?1:!strcmp(guide,"right")?2:!strcmp(guide,"forward")?3:!strcmp(guide,"back")?4:0;
  // Four possible pulse counts, followed by a clear pause. Colour reinforces the axis.
  int phase=cueAge%2000; bool on=phase<pulses*300 && phase%300<150;
  bool blue=green&&pulses>0&&pulses<=2&&on;
  bool forwardGreen=green&&pulses>=3&&on;
  digitalWrite(2,(flash||(green&&pulses==0))?HIGH:LOW);digitalWrite(3,(flash||forwardGreen)?HIGH:LOW);digitalWrite(4,blue?HIGH:LOW);
  if(now-lastPublish>=200) {
    lastPublish=now; Reading r={}; memcpy(r.targets,latest,sizeof(latest));r.valid=valid;r.x=valid?x:0;r.y=valid?y:0;r.count=count;r.seq=++sequence;r.frame=frames;r.ms=now;
    portENTER_CRITICAL(&mutex);shared=r;portEXIT_CRITICAL(&mutex);
    StaticJsonDocument<2048> d;payload(d,r);d["wifi_enabled"]=stationAllowed&&strlen(wifiSSID);d["oocsi_enabled"]=netEnabled;d["oocsi_connected"]=netConnected;d["wifi_connected"]=WiFi.status()==WL_CONNECTED;d["setup_ssid"]=setupSSID;
    d["wifi_ip"]=WiFi.localIP().toString();d["wifi_gateway"]=WiFi.gatewayIP().toString();d["wifi_rssi_dbm"]=WiFi.RSSI();d["portal_age_ms"]=now-portalServicedAt;
    d["led"]=green?guide:amber?"fault-amber":"off";d["guidance_supported"]=true; d["heartbeat_age_ms"]=heard?age:0;
    JsonArray a=d.createNestedArray("targets");
    if(frames&&now-lastFrameAt<400) for(int i=0;i<3;i++) if(latest[i].present){JsonObject t=a.createNestedObject();t["id"]=i+1;t["x"]=latest[i].x;t["y"]=latest[i].y;t["speed"]=latest[i].speed;t["resolution"]=latest[i].resolution;}
    // USB CDC can disappear without blocking sensor/LED work indefinitely.
    if(Serial && Serial.availableForWrite()>0){serializeJson(d,Serial);Serial.println();}
  }
  delay(1);
}
