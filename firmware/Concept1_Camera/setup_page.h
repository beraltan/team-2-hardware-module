#pragma once
const char SETUP_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Team 2 · Camera setup</title><style>
*{box-sizing:border-box}body{margin:0;background:#101922;color:#eef5fa;font:16px system-ui;padding:24px}main{max-width:460px;margin:6vh auto}h1{font-size:32px}p{line-height:1.6;color:#b6c9d8}label{display:block;margin:20px 0 8px}input,button{width:100%;padding:14px;border-radius:10px;border:1px solid #536b7e;font:inherit}input{background:#172734;color:white}button{margin-top:22px;background:#8ae5c5;color:#10231c;font-weight:700;cursor:pointer}button:disabled{opacity:.5}#status{padding:16px;background:#203344;border-radius:12px;white-space:pre-line}small{color:#b6c9d8}a{color:#8ae5c5}</style>
<main><small>TEAM 2 / BANK HEIST</small><h1>Connect your camera</h1>
<p>Enter the venue’s 2.4 GHz Wi-Fi details. Use a personal network or hotspot; eduroam and networks requiring a browser sign-in are not supported.</p>
<form id="setup"><label for="ssid">Network name (SSID)</label><input id="ssid" name="ssid" required maxlength="32" autocomplete="off" autocapitalize="none" spellcheck="false">
<label for="password">Wi-Fi password</label><input id="password" name="password" type="password" maxlength="64" autocomplete="new-password">
<small>Leave blank only for an open network.</small><button id="save">Connect and save</button></form>
<p id="status" role="status" aria-live="polite">Checking camera…</p>
<p>Public OOCSI sharing stays off during setup. Failed credentials are not saved. Stay on the camera network even if your phone says “no internet”.</p></main>
<script>
const form=document.querySelector('#setup'), statusBox=document.querySelector('#status'), button=document.querySelector('#save');
let token='',busy=false;
async function poll(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error();const d=await r.json();token=d.token;busy=d.state==='connecting';button.disabled=busy;statusBox.textContent=d.message+(d.ip?'\nVenue IP: '+d.ip:'');}catch{statusBox.textContent='Reconnect to the camera Wi-Fi, then reload this page.';}setTimeout(poll,1500);}
form.addEventListener('submit',async e=>{e.preventDefault();if(busy)return;button.disabled=true;try{const r=await fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json','X-Setup-Token':token},body:JSON.stringify({ssid:form.ssid.value,password:form.password.value})});const d=await r.json();if(!r.ok)throw Error(d.error||'Could not connect');form.password.value='';statusBox.textContent='Connecting… this can take 30 seconds.';busy=true;}catch(e){statusBox.textContent=e.message;button.disabled=false;}});poll();
</script></html>)HTML";
