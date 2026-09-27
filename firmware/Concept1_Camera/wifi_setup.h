#pragma once
#include <WebServer.h>
#include <DNSServer.h>
#include "setup_page.h"
WebServer setupServer(80);
DNSServer setupDNS;
String setupSSID, setupPassword, setupToken, candidateSSID, candidatePassword;
String setupState="idle", setupMessage="Enter the venue Wi-Fi details.";
uint32_t connectionStarted=0, retryAt=0;
bool candidatePending=false, stationAllowed=true;
volatile uint32_t portalServicedAt=0;

bool setupLocal() { return setupServer.client().localIP()==WiFi.softAPIP(); }
void setupJSON(int code, JsonDocument& data) {
  String body; serializeJson(data,body);
  setupServer.sendHeader("Cache-Control","no-store");
  setupServer.send(code,"application/json",body);
}
void beginSetupPortal() {
  uint64_t mac=ESP.getEfuseMac();
  char suffix[7];snprintf(suffix,sizeof(suffix),"%06lx",(unsigned long)(mac&0xffffff));
  setupSSID=String("Team2-Camera-")+suffix;
  setupPassword=prefs.getString("ap_password","");
  if(setupPassword.length()<12){char p[17];snprintf(p,sizeof(p),"%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random());setupPassword=p;prefs.putString("ap_password",p);}
  char t[17];snprintf(t,sizeof(t),"%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random());setupToken=t;
  WiFi.persistent(false);WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(IPAddress(192,168,4,1),IPAddress(192,168,4,1),IPAddress(255,255,255,0));
  WiFi.softAP(setupSSID.c_str(),setupPassword.c_str());
  setupDNS.start(53,"*",WiFi.softAPIP());
  const char* headers[]={"X-Setup-Token"};setupServer.collectHeaders(headers,1);
  setupServer.on("/",HTTP_GET,[]{
    setupServer.sendHeader("Cache-Control","no-store");
    if(!setupLocal()){
      setupServer.send(200,"text/html","<!doctype html><meta name='viewport' content='width=device-width,initial-scale=1'><title>Team 2 Camera</title><body style='font:18px system-ui;max-width:600px;margin:60px auto;padding:24px'><h1>Team 2 Camera</h1><p>The camera is reachable on the venue network. USB radar and Wi-Fi can operate together.</p><p><a href='/api/status'>View connection status</a></p><p>To change Wi-Fi credentials, join the camera setup network and open http://192.168.4.1.</p></body>");return;
    }
    setupServer.send_P(200,"text/html",SETUP_PAGE);
  });
  setupServer.on("/api/status",HTTP_GET,[]{
    StaticJsonDocument<512> d;d["state"]=setupState;d["message"]=setupMessage;
    if(setupLocal())d["token"]=setupToken;
    d["wifi_connected"]=WiFi.status()==WL_CONNECTED;d["oocsi_enabled"]=netEnabled;d["oocsi_connected"]=netConnected;d["uptime_ms"]=millis();
    if(WiFi.status()==WL_CONNECTED)d["ip"]=WiFi.localIP().toString();setupJSON(200,d);
  });
  setupServer.on("/api/wifi",HTTP_POST,[]{
    StaticJsonDocument<512> d;
    if(!setupLocal()||setupServer.header("X-Setup-Token")!=setupToken){d["error"]="Reload the setup page.";setupJSON(403,d);return;}
    if(candidatePending){d["error"]="Connection already in progress.";setupJSON(409,d);return;}
    if(setupServer.arg("plain").length()>512 || deserializeJson(d,setupServer.arg("plain"))){d.clear();d["error"]="Invalid request.";setupJSON(400,d);return;}
    String s=d["ssid"] | "",p=d["password"] | "";
    bool hex=true;for(unsigned int i=0;i<p.length();i++)if(!isxdigit((unsigned char)p[i]))hex=false;
    if(!s.length()||s.length()>32||(p.length()>0&&p.length()<8)||p.length()>64||(p.length()==64&&!hex)){
      d.clear();d["error"]="Use a 1–32 byte SSID and an 8–63 character password (or leave blank for an open network).";setupJSON(400,d);return;
    }
    // Provisioning never enables public transmission, including an old saved opt-in.
    netEnabled=false;prefs.putBool("enabled",false);
    candidateSSID=s;candidatePassword=p;candidatePending=true;stationAllowed=true;
    setupState="connecting";setupMessage="Connecting to the venue network…";connectionStarted=millis();
    WiFi.disconnect();WiFi.begin(candidateSSID.c_str(),candidatePassword.c_str());
    d.clear();d["accepted"]=true;setupJSON(202,d);
  });
  setupServer.onNotFound([]{if(!setupLocal()){setupServer.send(403);return;}setupServer.sendHeader("Location","http://192.168.4.1/");setupServer.send(302,"text/plain","");});
  setupServer.begin();
  stationAllowed=prefs.getBool("wifi_on",true);
  if(stationAllowed&&strlen(wifiSSID))WiFi.begin(wifiSSID,wifiPassword);
}
void serviceSetupPortal() {
  portalServicedAt=millis();
  setupDNS.processNextRequest();setupServer.handleClient();
  if(candidatePending){
    if(WiFi.status()==WL_CONNECTED && WiFi.SSID()==candidateSSID){
      prefs.putString("ssid",candidateSSID);prefs.putString("password",candidatePassword);prefs.putBool("wifi_on",true);
      strlcpy(wifiSSID,candidateSSID.c_str(),sizeof(wifiSSID));strlcpy(wifiPassword,candidatePassword.c_str(),sizeof(wifiPassword));
      candidatePassword="";candidatePending=false;setupState="connected";setupMessage="Connected. Credentials saved. Public sharing is off. You can now leave the camera Wi-Fi.";
    }else if(millis()-connectionStarted>30000){
      WiFi.disconnect();candidatePassword="";candidatePending=false;setupState="failed";setupMessage="Could not connect. Check the password, 2.4 GHz coverage and external antenna, then try again.";
      retryAt=millis();
    }
  }else if(stationAllowed&&strlen(wifiSSID)&&WiFi.status()!=WL_CONNECTED&&millis()-retryAt>15000){retryAt=millis();WiFi.begin(wifiSSID,wifiPassword);}
  if(!candidatePending&&setupState!="failed"){
    if(WiFi.status()==WL_CONNECTED){setupState="connected";setupMessage=netEnabled?"Connected. Public OOCSI sharing was separately enabled.":"Connected. Public sharing is off. You can now leave the camera Wi-Fi.";}
    else if(strlen(wifiSSID)&&stationAllowed){setupState="reconnecting";setupMessage="Trying the saved venue network. Enter new details below if needed.";}
  }
}
