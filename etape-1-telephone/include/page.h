// Page web servie au téléphone : un joystick tactile qui envoie "x,y" (de -100 à 100)
// à la voiture par WebSocket (port 81), 20 fois par seconde.
#pragma once
#include <Arduino.h>

const char PAGE_PILOTAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="fr"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Voiture RC</title>
<style>
html,body{margin:0;height:100%;background:#111;color:#eee;font-family:sans-serif;
  touch-action:none;user-select:none;-webkit-user-select:none;overflow:hidden}
#etat{position:fixed;top:10px;left:0;right:0;text-align:center;font-size:15px}
#zone{position:fixed;bottom:0;left:0;right:0;height:75%;display:flex;align-items:center;justify-content:center}
#base{width:260px;height:260px;border-radius:50%;background:#222;border:3px solid #444;position:relative}
#manche{width:100px;height:100px;border-radius:50%;background:#e33;position:absolute;left:80px;top:80px}
</style></head><body>
<div id="etat">Connexion...</div>
<div id="zone"><div id="base"><div id="manche"></div></div></div>
<script>
const etat=document.getElementById('etat'),base=document.getElementById('base'),manche=document.getElementById('manche');
let ws,x=0,y=0,doigt=null;
function connecter(){
  ws=new WebSocket('ws://'+location.hostname+':81/');
  ws.onopen=()=>etat.textContent='Connecté';
  ws.onmessage=e=>etat.textContent=e.data;
  ws.onclose=()=>{etat.textContent='Déconnecté, nouvel essai...';setTimeout(connecter,1000)};
}
connecter();
function bouger(px,py){
  const r=base.getBoundingClientRect(),R=r.width/2;
  let dx=px-(r.left+R),dy=py-(r.top+R);
  const d=Math.hypot(dx,dy);
  if(d>R){dx*=R/d;dy*=R/d}
  manche.style.transform=`translate(${dx}px,${dy}px)`;
  x=Math.round(dx/R*100);y=Math.round(-dy/R*100);
}
function lacher(){doigt=null;x=0;y=0;manche.style.transform=''}
base.addEventListener('pointerdown',e=>{doigt=e.pointerId;base.setPointerCapture(doigt);bouger(e.clientX,e.clientY)});
base.addEventListener('pointermove',e=>{if(e.pointerId===doigt)bouger(e.clientX,e.clientY)});
base.addEventListener('pointerup',lacher);
base.addEventListener('pointercancel',lacher);
// Envoi permanent, même manche au centre : la voiture s'arrête seule si le téléphone se tait
setInterval(()=>{if(ws&&ws.readyState===1)ws.send(x+','+y)},50);
</script></body></html>)rawliteral";
