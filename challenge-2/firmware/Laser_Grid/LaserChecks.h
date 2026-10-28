#pragma once
#include "LaserLogic.h"
namespace laser_checks {
constexpr laser::Beat beat(long n,const char*round="laser-1",const char*owner="crew",const char*state="running",bool solved=false){return {owner,round,state,n,solved};}
constexpr bool confirmation(){laser::RoundLease l;l.ingest(beat(1),0);if(l.context(true,0,"test").scope!=laser::Scope::Blocked)return false;l.ingest(beat(2),200);return l.context(true,200,"test").scope==laser::Scope::Live;}
constexpr bool replay(){laser::RoundLease l;l.ingest(beat(1),0);l.ingest(beat(2),200);if(l.ingest(beat(1,"old"),1000))return false;return laser::same(l.context(true,1000,"test").round,"laser-1")&&l.context(true,3200,"test").scope==laser::Scope::Test;}
constexpr bool conflict(){laser::RoundLease l;l.ingest(beat(1),0);l.ingest(beat(2),200);l.ingest(beat(1,"other","extra"),250);if(l.context(true,250,"test").scope!=laser::Scope::Blocked)return false;l.ingest(beat(3),1000);l.ingest(beat(4),3300);return l.context(true,3300,"test").scope==laser::Scope::Live;}
constexpr bool takeover(){laser::RoundLease l;l.ingest(beat(1),0);l.ingest(beat(2),200);l.ingest(beat(1,"new","extra"),3300);if(l.context(true,3300,"test").scope!=laser::Scope::Blocked)return false;l.ingest(beat(2,"new","extra"),3500);return laser::same(l.context(true,3500,"test").round,"new");}
constexpr bool edge(){laser::DebouncedInput d;d.begin(false,0);if(d.update(true,1,20)||d.update(false,5,20)||d.update(true,10,20)||d.update(true,29,20))return false;if(!d.update(true,30,20))return false;return !d.update(true,200,20)&&d.value();}
constexpr bool held(){laser::DebouncedInput d;d.begin(true,0);if(d.update(true,100,25))return false;d.update(false,110,25);if(!d.update(false,135,25))return false;d.update(true,150,25);return d.update(true,175,25);}
constexpr bool rounds(){laser::RoundLease l;l.ingest(beat(1),0);l.ingest(beat(2),200);laser::Event e;e.context=l.context(true,200,"test");e.at=200;l.ingest(beat(3,"laser-2"),300);return !laser::deliverable(e,l.context(true,300,"test"),300);}
constexpr bool expiry(){laser::RoundLease l;l.ingest(beat(1),0);l.ingest(beat(2),200);laser::Event e;e.context=l.context(true,200,"test");e.at=200;return laser::deliverable(e,e.context,950)&&!laser::deliverable(e,e.context,951)&&!laser::deliverable(e,l.context(false,300,"test"),300);}
constexpr bool state(const char*s,bool solved=false){laser::RoundLease l;l.ingest(beat(1,"r","crew",s,solved),0);l.ingest(beat(2,"r","crew",s,solved),200);return l.context(true,200,"test").scope==laser::Scope::Blocked;}
constexpr bool wrap(){laser::DebouncedInput d;d.begin(false,UINT32_MAX-20);d.update(true,UINT32_MAX-10,20);return !d.update(true,5,20)&&d.update(true,10,20);}
static_assert(confirmation(),"Two fresh beats are needed for live events");
static_assert(replay(),"Old beats cannot alter round or renew freshness");
static_assert(conflict(),"Competing controllers block live events until quiet");
static_assert(takeover(),"An expired controller can be replaced after confirmation");
static_assert(edge(),"Contact bounce and a held beam must not repeat edges");
static_assert(held(),"A held startup/button value is a baseline, not a press");
static_assert(rounds(),"Queued edges cannot move to a new laser round");
static_assert(expiry(),"Old/disconnected input must not replay");
static_assert(state("lost"),"Lockdown blocks live events");
static_assert(state("fault"),"Fault blocks live events");
static_assert(state("won"),"Completed heists block live events");
static_assert(state("running",true),"A solved laser module blocks later events");
static_assert(wrap(),"Debounce tolerates millis rollover");
static_assert(laser::analogBroken(1400,false,true,1500,1800)&&laser::analogBroken(1700,true,true,1500,1800)&&!laser::analogBroken(1800,true,true,1500,1800),"Analog low threshold needs hysteresis");
static_assert(laser::analogBroken(2700,false,false,2600,2300)&&laser::analogBroken(2400,true,false,2600,2300)&&!laser::analogBroken(2300,true,false,2600,2300),"Analog high threshold needs hysteresis");
}
