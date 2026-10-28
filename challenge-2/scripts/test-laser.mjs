// Protocol integration against the actual backed-up hacker page, without a browser/network.
import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {webcrypto} from 'node:crypto';
import vm from 'node:vm';
const html=await readFile(process.argv[2]||new URL('../../../software-module/bank-heist.html',import.meta.url),'utf8');
const bundle=[...html.matchAll(/<script>([\s\S]*?)<\/script>/g)].map(m=>m[1]).find(s=>s.includes('const modules={};'));
assert(bundle,'Backed-up game bundle not found');
const context=vm.createContext({window:{addEventListener(){}},testing:true,demo:false,crypto:webcrypto});
vm.runInContext(bundle+';globalThis.protocol={Controller:modules.controller.Controller,HardwareTest:modules.hardwareTest.HardwareTest};',context);
const {Controller,HardwareTest}=context.protocol;
function kit(){
 const c=new Controller(0,'crew');c.live(0);c.connected=true;
 c.receive({sender:'camera',data:{radar_seq:1,radar_boot:'b',radar_valid:true,radar_x_mm:-900,radar_y_mm:1800}},0);
 c.game.setup_checked=true;c.start(0);c.game.layout={zones:[],key:'fixture'};
 return c;
}
const packet=(event,id,round,extra={})=>({sender:'laser-c3',data:{module:'laser',device_id:'team2-laser-123456',boot_id:'abcd1234',event,event_id:id,round_id:round,...(event==='button_pressed'?{}:{beam_id:1}),...extra}});
test('C3 edge format charges five seconds once; held state and retries do not add penalties',()=>{
 const c=kit(),round=c.laser.roundId,before=c.game.remaining;
 const cut=packet('beam_broken','abcd1234-1',round);c.receive(cut,.1);
 assert.equal(c.game.penalties,1);assert(Math.abs(before-c.game.remaining-5.1)<1e-6);
 c.receive(cut,.1);c.receive(packet('state',undefined,round,{broken:true}),.1);assert.equal(c.game.penalties,1);
 c.receive(packet('state',undefined,round,{broken:false}),.2);c.receive(packet('beam_broken','abcd1234-2',round),.2);assert.equal(c.game.penalties,2);
});
test('finish packet completes Laser once without a penalty or a clock reset; old rounds are ignored',()=>{
 const c=kit(),round=c.laser.roundId;
 c.receive(packet('button_pressed','abcd1234-3','previous-round'),.1);assert.equal(c.laser.state,'idle');
 const before=c.game.remaining;c.receive(packet('button_pressed','abcd1234-4',round),.1);
 assert.equal(c.laser.state,'solved');assert.equal(c.game.penalties,0);assert(c.game.remaining<=before);
 const result=c.laser.takeResult();assert.equal(result.device_id,'team2-laser-123456');assert.equal(result.input_event_id,'abcd1234-4');
 c.receive(packet('button_pressed','abcd1234-4',round),.2);assert.equal(c.laser.takeResult(),null);
});
test('no-controller test round is accepted by the workshop and ignored by a live heist',()=>{
 const bench=new HardwareTest(),crew=kit();bench.selectModule('laser');
 const cut=packet('beam_broken','abcd1234-1','test-abcd1234');
 for(const receiver of [bench,crew]){if(receiver===bench){receiver.ingest(cut,0);receiver.ingest(cut,.1);receiver.ingest(packet('button_pressed','abcd1234-2','test-abcd1234'),.2);}else receiver.receive(cut,.1);}
 assert.equal(bench.snapshot(.2).laser.cuts,1);assert.equal(bench.snapshot(.2).laser.finishes,1);assert.equal(crew.game.penalties,0);assert.equal(crew.laser.state,'idle');
});

test('alignment uses fresh receiver states, not just an online heartbeat',()=>{
 const b=new HardwareTest();b.selectModule('laser');b.setLaserAlignment(true);
 b.ingest(packet('online',undefined,'test'),0);assert.equal(b.snapshot(0).laser.alignment_status,'waiting');
 b.ingest(packet('state',undefined,'test',{broken:false,input_raw:1}),1);assert.equal(b.snapshot(1).laser.alignment_status,'detected');
 b.ingest(packet('state',undefined,'test',{broken:true,input_raw:0}),2);assert.equal(b.snapshot(2).laser.alignment_status,'missing');
 b.ingest(packet('online',undefined,'test'),8);assert.equal(b.snapshot(8).laser.alignment_status,'stale');
});

test('alignment suppresses only local counts, including retries after it ends',()=>{
 const b=new HardwareTest();b.selectModule('laser');b.setLaserAlignment(true);
 const cut=packet('beam_broken','align-1','test'),finish=packet('button_pressed','align-2','test');
 b.ingest(cut,0);b.ingest(finish,.1);b.setLaserAlignment(false);b.ingest(cut,.2);b.ingest(finish,.3);
 assert.equal(b.snapshot(.3).laser.cuts,0);assert.equal(b.snapshot(.3).laser.finishes,0);
 b.ingest(packet('beam_broken','new-cut','test'),.4);assert.equal(b.snapshot(.4).laser.cuts,1);
});
