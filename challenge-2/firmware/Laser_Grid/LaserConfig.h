#pragma once
#include <Arduino.h>

// One TinyTronics REDLASERSENSOR on XIAO ESP32-C3.
// Receiver VCC = USB VBUS/5V. OUT must pass through 10k/15k divider to D1.
// Measure blocked polarity before the demo; true below means blocked = LOW.
enum class ReceiverKind { Digital, Analog };
struct BeamConfig {
  uint8_t pin; uint16_t id; ReceiverKind kind; bool brokenLow;
  uint8_t inputMode; uint16_t brokenThreshold; uint16_t clearThreshold;
};
constexpr BeamConfig BEAMS[]={
  {D1,1,ReceiverKind::Digital,true,INPUT,1500,1800},
  // Additional divided receiver: {D2,2,ReceiverKind::Digital,true,INPUT,1500,1800},
};
constexpr size_t BEAM_COUNT=sizeof(BEAMS)/sizeof(BEAMS[0]);
constexpr uint8_t FINISH_BUTTON_PIN=D4; // C3 D4 = GPIO6. Bare switch to GND.
constexpr bool FINISH_ACTIVE_LOW=true;
constexpr uint32_t SAMPLE_MS=5,BEAM_DEBOUNCE_MS=20,BUTTON_DEBOUNCE_MS=25;
constexpr uint32_t STATUS_MS=2000;

constexpr bool validWiring(){
  for(size_t i=0;i<BEAM_COUNT;i++){
    if(BEAMS[i].pin==FINISH_BUTTON_PIN)return false;
    if(BEAMS[i].kind==ReceiverKind::Analog){
      if(BEAMS[i].brokenThreshold>4095||BEAMS[i].clearThreshold>4095)return false;
      if(BEAMS[i].brokenLow?BEAMS[i].brokenThreshold>=BEAMS[i].clearThreshold:BEAMS[i].brokenThreshold<=BEAMS[i].clearThreshold)return false;
    }
    for(size_t j=0;j<i;j++)if(BEAMS[i].pin==BEAMS[j].pin||BEAMS[i].id==BEAMS[j].id)return false;
  }
  return true;
}
static_assert(BEAM_COUNT>=1&&BEAM_COUNT<=4,"Use one to four separate beam inputs");
static_assert(validWiring(),"Each beam needs unique pin/ID, no button overlap, and valid analog hysteresis");
