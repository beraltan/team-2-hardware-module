#pragma once
#include <stdint.h>
#include <stddef.h>

namespace laser {
constexpr bool same(const char*a,const char*b){return *a==*b&&(!*a||same(a+1,b+1));}
constexpr size_t length(const char*s){return *s?1+length(s+1):0;}
template<size_t N> constexpr void copy(char(&out)[N],const char*in){size_t i=0;for(;in[i]&&i<N-1;i++)out[i]=in[i];out[i]=0;}
enum class Scope {Blocked,Test,Live};
struct Context {Scope scope=Scope::Blocked;char owner[65]={};char round[97]={};};
struct Beat {const char*owner;const char*round;const char*state;long sequence;bool solved;};

class RoundLease {
 public:
  static constexpr uint32_t LEASE_MS=3000;
  constexpr bool ingest(const Beat&b,uint32_t now){
    if(!*b.owner||length(b.owner)>64||!*b.round||length(b.round)>96||b.sequence<=0||
      !(same(b.state,"idle")||same(b.state,"armed")||same(b.state,"running")||same(b.state,"won")||same(b.state,"lost")||same(b.state,"fault")))return false;
    const bool wasFresh=fresh(now);
    if(wasFresh&&!same(owner_,b.owner)){conflict_=true;conflictAt_=now;return false;}
    if(!same(owner_,b.owner)){copy(owner_,b.owner);sequence_=-1;confirmations_=0;}
    if(b.sequence<=sequence_)return false; // Duplicates cannot renew a lease.
    if(!wasFresh)confirmations_=0;
    sequence_=b.sequence;lastAt_=now;if(confirmations_<2)confirmations_++;
    copy(round_,b.round);
    accepts_=!b.solved&&(same(b.state,"idle")||same(b.state,"armed")||same(b.state,"running"));
    if(conflict_&&uint32_t(now-conflictAt_)>=LEASE_MS)conflict_=false;
    return true;
  }
  constexpr bool fresh(uint32_t now)const{return *owner_&&uint32_t(now-lastAt_)<LEASE_MS;}
  constexpr bool conflict()const{return conflict_;}
  constexpr Context context(bool connected,uint32_t now,const char*testRound)const{
    Context c;
    if(!connected)return c;
    if(!fresh(now)){c.scope=Scope::Test;copy(c.round,testRound);return c;}
    if(conflict_||confirmations_<2||!accepts_)return c;
    c.scope=Scope::Live;copy(c.owner,owner_);copy(c.round,round_);return c;
  }
 private:
  char owner_[65]={},round_[97]={};long sequence_=-1;
  uint32_t lastAt_=0,conflictAt_=0;uint8_t confirmations_=0;bool conflict_=false,accepts_=false;
};

class DebouncedInput {
 public:
  constexpr void begin(bool value,uint32_t now){value_=candidate_=value;changedAt_=now;}
  constexpr bool update(bool reading,uint32_t now,uint32_t debounce){
    if(reading!=candidate_){candidate_=reading;changedAt_=now;}
    if(candidate_==value_||uint32_t(now-changedAt_)<debounce)return false;
    value_=candidate_;return true;
  }
  constexpr bool value()const{return value_;}
 private:
  bool value_=false,candidate_=false;uint32_t changedAt_=0;
};

constexpr bool analogBroken(int raw,bool previous,bool below,int enter,int clear){
  return below?(previous?raw<clear:raw<=enter):(previous?raw>clear:raw>=enter);
}
struct Event {Context context;uint32_t at=0,sequence=0;uint16_t beam=0;bool finish=false;};
constexpr bool deliverable(const Event&e,const Context&current,uint32_t now){
  return uint32_t(now-e.at)<=750&&current.scope!=Scope::Blocked&&e.context.scope==current.scope&&
    same(e.context.owner,current.owner)&&same(e.context.round,current.round);
}
}
