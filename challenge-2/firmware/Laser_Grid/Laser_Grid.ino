#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Team2Config.h>
#include "LaserConfig.h"
#include "LaserChecks.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#if !CONFIG_IDF_TARGET_ESP32C3
#error "Select Seeed XIAO ESP32-C3 (not C6) for this sketch."
#endif

// Raw OOCSI TCP framing follows the existing camera and installed OOCSI library.
// Sampling runs separately: a slow TCP reconnect cannot hide a short input edge.
WiFiClient client;
portMUX_TYPE inputMutex=portMUX_INITIALIZER_UNLOCKED;
laser::RoundLease lease;
laser::DebouncedInput beamFilters[BEAM_COUNT],buttonFilter;
QueueHandle_t events=nullptr;
bool wireReady=false,inputReady=false,beamLevels[BEAM_COUNT]={},finishHeld=false;
int rawLevels[BEAM_COUNT]={};
uint32_t eventSequence=0,dropped=0,lastAttempt=0,lastWifiAttempt=0,lastByte=0,lastStatus=0;
char bootId[9],deviceId[40],handle[64],testRound[40];
String incoming;
bool greeted=false;

bool readBroken(size_t i){
  const auto&b=BEAMS[i];const int raw=b.kind==ReceiverKind::Analog?analogRead(b.pin):digitalRead(b.pin);
  portENTER_CRITICAL(&inputMutex);rawLevels[i]=raw;portEXIT_CRITICAL(&inputMutex);
  return b.kind==ReceiverKind::Analog?laser::analogBroken(raw,beamFilters[i].value(),b.brokenLow,b.brokenThreshold,b.clearThreshold):raw==(b.brokenLow?LOW:HIGH);
}
void enqueueEdge(bool finish,uint16_t beam,uint32_t now){
  laser::Event e;
  portENTER_CRITICAL(&inputMutex);
  e.context=lease.context(wireReady,now,testRound);e.sequence=++eventSequence;
  portEXIT_CRITICAL(&inputMutex);
  if(e.context.scope==laser::Scope::Blocked)return;
  e.at=now;e.finish=finish;e.beam=beam;
  if(xQueueSend(events,&e,0)!=pdTRUE){portENTER_CRITICAL(&inputMutex);dropped++;portEXIT_CRITICAL(&inputMutex);}
}
void sampleInputs(void*){
  TickType_t wake=xTaskGetTickCount();const TickType_t ticks=pdMS_TO_TICKS(SAMPLE_MS)?pdMS_TO_TICKS(SAMPLE_MS):1;
  while(true){
    const uint32_t now=millis();
    for(size_t i=0;i<BEAM_COUNT;i++){
      const bool changed=beamFilters[i].update(readBroken(i),now,BEAM_DEBOUNCE_MS);
      portENTER_CRITICAL(&inputMutex);beamLevels[i]=beamFilters[i].value();portEXIT_CRITICAL(&inputMutex);
      if(changed&&beamFilters[i].value())enqueueEdge(false,BEAMS[i].id,now);
    }
    const bool pressed=digitalRead(FINISH_BUTTON_PIN)==(FINISH_ACTIVE_LOW?LOW:HIGH);
    const bool changed=buttonFilter.update(pressed,now,BUTTON_DEBOUNCE_MS);
    portENTER_CRITICAL(&inputMutex);finishHeld=buttonFilter.value();portEXIT_CRITICAL(&inputMutex);
    if(changed&&buttonFilter.value())enqueueEdge(true,0,now);
    vTaskDelayUntil(&wake,ticks);
  }
}
laser::Context currentContext(uint32_t now){
  portENTER_CRITICAL(&inputMutex);const auto c=lease.context(wireReady,now,testRound);portEXIT_CRITICAL(&inputMutex);return c;
}
void disconnectSocket(){
  portENTER_CRITICAL(&inputMutex);wireReady=false;lease=laser::RoundLease();portEXIT_CRITICAL(&inputMutex);
  if(events)xQueueReset(events);client.stop();greeted=false;incoming="";
}
bool sendData(JsonDocument&data){
  String body;serializeJson(data,body);const String frame=String("sendraw ")+team2::TEAM_CHANNEL+" "+body+"\n";
  if(client.write(reinterpret_cast<const uint8_t*>(frame.c_str()),frame.length())!=frame.length()){disconnectSocket();return false;}
  return true;
}
void processLine(const String&line){
  if(line=="ping"||line==" "||line=="."){client.println('.');return;}
  if(line.startsWith("error")){disconnectSocket();return;}
  if(!greeted){
    if(line.indexOf("welcome")<0)return;
    client.println(String("subscribe ")+team2::TEAM_CHANNEL);greeted=true;
    portENTER_CRITICAL(&inputMutex);wireReady=true;portEXIT_CRITICAL(&inputMutex);
    lastStatus=millis()-STATUS_MS;return;
  }
  JsonDocument packet;if(deserializeJson(packet,line))return;
  if(packet["recipient"]!=team2::TEAM_CHANNEL)return;
  JsonVariantConst d=packet["data"].is<JsonObjectConst>()?packet["data"].as<JsonVariantConst>():packet.as<JsonVariantConst>();
  if(!d["toktik_beat"].is<long>()||!d["laser_solved"].is<bool>())return;
  const char*state=d["heist_state"]|"";if(!*state)state=d["toktik_state"]|"";
  const laser::Beat b={d["toktik_session"]|"",d["laser_round_id"]|"",state,d["toktik_beat"].as<long>(),d["laser_solved"].as<bool>()};
  portENTER_CRITICAL(&inputMutex);lease.ingest(b,millis());portEXIT_CRITICAL(&inputMutex);
}
void sendEdges(){
  laser::Event e;
  for(uint8_t n=0;n<8&&events&&xQueueReceive(events,&e,0)==pdTRUE;n++){
    const uint32_t now=millis();if(!laser::deliverable(e,currentContext(now),now))continue;
    char id[32];snprintf(id,sizeof(id),"%s-%lu",bootId,static_cast<unsigned long>(e.sequence));
    JsonDocument d;d["module"]="laser";d["event"]=e.finish?"button_pressed":"beam_broken";
    d["device_id"]=deviceId;d["boot_id"]=bootId;d["event_id"]=id;d["round_id"]=e.context.round;
    if(!e.finish)d["beam_id"]=e.beam;
    if(!sendData(d))return;
  }
}
void sendStatus(uint32_t now){
  const auto context=currentContext(now);
  bool levels[BEAM_COUNT],held,conflict,fresh;int raw[BEAM_COUNT];uint32_t count,lost;
  portENTER_CRITICAL(&inputMutex);
  for(size_t i=0;i<BEAM_COUNT;i++){levels[i]=beamLevels[i];raw[i]=rawLevels[i];}
  held=finishHeld;count=eventSequence;lost=dropped;conflict=lease.conflict();fresh=lease.fresh(now);
  portEXIT_CRITICAL(&inputMutex);
  JsonDocument d;d["module"]="laser";d["event"]="online";d["device_id"]=deviceId;d["boot_id"]=bootId;
  d["input_ready"]=inputReady;d["finish_held"]=held;d["event_counter"]=count;d["dropped_events"]=lost;
  d["controller_link"]=fresh?(conflict?"conflict":"fresh"):"test";
  if(!sendData(d))return;
  // Receiver baselines remain observable for mirror alignment even if live
  // events are blocked. A diagnostic test round cannot change the live heist.
  const char*statusRound=context.scope==laser::Scope::Blocked?testRound:context.round;
  for(size_t i=0;i<BEAM_COUNT;i++){
    d.clear();d["module"]="laser";d["event"]="state";d["device_id"]=deviceId;
    d["round_id"]=statusRound;d["beam_id"]=BEAMS[i].id;d["broken"]=levels[i];d["input_raw"]=raw[i];
    if(!sendData(d))return;
  }
}
void setup(){
  Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
#endif
  analogReadResolution(12);const uint32_t now=millis();
  for(size_t i=0;i<BEAM_COUNT;i++){pinMode(BEAMS[i].pin,BEAMS[i].inputMode);beamFilters[i].begin(readBroken(i),now);beamLevels[i]=beamFilters[i].value();}
  pinMode(FINISH_BUTTON_PIN,FINISH_ACTIVE_LOW?INPUT_PULLUP:INPUT_PULLDOWN);
  buttonFilter.begin(digitalRead(FINISH_BUTTON_PIN)==(FINISH_ACTIVE_LOW?LOW:HIGH),now);finishHeld=buttonFilter.value();
  snprintf(bootId,sizeof(bootId),"%08lx",static_cast<unsigned long>(esp_random()));
  snprintf(deviceId,sizeof(deviceId),"team2-laser-%06lx",static_cast<unsigned long>(ESP.getEfuseMac()&0xffffff));
  snprintf(handle,sizeof(handle),"%s_%s",deviceId,bootId);snprintf(testRound,sizeof(testRound),"test-%s",bootId);
  events=xQueueCreate(16,sizeof(laser::Event));
  if(events)inputReady=xTaskCreate(sampleInputs,"laser-input",4096,nullptr,2,nullptr)==pdPASS;
  if(team2::WIFI_CONFIGURED){WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.begin(team2::WIFI_SSID,team2::WIFI_PASSWORD);}
}
void loop(){
  if(!team2::WIFI_CONFIGURED){delay(20);return;}
  const uint32_t now=millis();
  if(WiFi.status()!=WL_CONNECTED){
    if(greeted||client.connected())disconnectSocket();
    if(uint32_t(now-lastWifiAttempt)>=10000){lastWifiAttempt=now;WiFi.reconnect();}
    delay(2);return;
  }
  if(!client.connected()){
    if(greeted)disconnectSocket();
    if(uint32_t(now-lastAttempt)>=3000){lastAttempt=now;if(client.connect(team2::OOCSI_SERVER,4444,250)){client.println(String(handle)+"(JSON)");lastByte=now;}}
    delay(2);return;
  }
  for(size_t n=0;n<4096&&client.available();n++){
    const char c=client.read();lastByte=millis();
    if(c=='\n'){const String line=incoming;incoming="";processLine(line);}
    else if(c!='\r'){if(incoming.length()<8192)incoming+=c;else{disconnectSocket();break;}}
  }
  const uint32_t afterRead=millis(); // Incoming bytes can have a newer timestamp than loop entry.
  if((!greeted&&uint32_t(afterRead-lastByte)>5000)||uint32_t(afterRead-lastByte)>30000){disconnectSocket();delay(2);return;}
  if(greeted){sendEdges();if(greeted&&uint32_t(afterRead-lastStatus)>=STATUS_MS){lastStatus=afterRead;sendStatus(millis());}}
  delay(2);
}
