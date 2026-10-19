// Page web servie au téléphone : la vidéo de l'ESP32-CAM en haut, le joystick en bas.
// Le joystick envoie "x,y" (de -100 à 100) par WebSocket (port 81), 20 fois par seconde.
// À utiliser téléphone en mode portrait.
#pragma once
#include <Arduino.h>
#include "config.h"

const char PAGE_PILOTAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="fr"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Voiture RC</title>
<style>
html,body{margin:0;height:100%;background:#111;color:#eee;font-family:sans-serif;
  touch-action:none;user-select:none;-webkit-user-select:none;overflow:hidden}
#video{position:fixed;top:0;left:0;right:0;height:50%;background:#000;display:flex;align-items:center;justify-content:center}
#video img{max-width:100%;max-height:100%}
#etat{position:fixed;top:50%;left:0;right:0;text-align:center;font-size:15px;padding:8px}
#phare{position:fixed;top:calc(50% + 6px);right:10px;padding:6px 12px;border-radius:8px;border:0;background:#333;color:#eee;font-size:15px}
#phare.allume{background:#fc3;color:#111}
#zone{position:fixed;bottom:0;left:0;right:0;height:45%;display:flex;align-items:center;justify-content:center}
#base{width:220px;height:220px;border-radius:50%;background:#222;border:3px solid #444;position:relative}
#manche{width:90px;height:90px;border-radius:50%;background:#e33;position:absolute;left:65px;top:65px}
</style></head><body>
<div id="video"><img id="flux" alt="Caméra hors ligne"></div>
<div id="etat">Connexion...</div>
<button id="phare">Phare</button>
<div id="zone"><div id="base"><div id="manche"></div></div></div>
<script>
const CAMERA='http://)rawliteral" IP_CAMERA R"rawliteral(';
const etat=document.getElementById('etat'),base=document.getElementById('base'),manche=document.getElementById('manche');
const flux=document.getElementById('flux'),phare=document.getElementById('phare');
let ws,x=0,y=0,doigt=null,phareAllume=false;
function connecter(){
  ws=new WebSocket('ws://'+location.hostname+':81/');
  ws.onopen=()=>etat.textContent='Connecté';
  ws.onmessage=e=>etat.textContent=e.data;
  ws.onclose=()=>{etat.textContent='Déconnecté, nouvel essai...';setTimeout(connecter,1000)};
}
connecter();
// Vidéo : on relance le flux s'il coupe (caméra redémarrée, hors de portée...)
function lancerVideo(){flux.src=CAMERA+':81/flux?t='+Date.now()}
flux.onerror=()=>setTimeout(lancerVideo,2000);
lancerVideo();
phare.onclick=()=>{
  phareAllume=!phareAllume;
  phare.classList.toggle('allume',phareAllume);
  fetch(CAMERA+'/phare?etat='+(phareAllume?1:0),{mode:'no-cors'}).catch(()=>{});
};
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
