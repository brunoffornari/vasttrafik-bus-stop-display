// ============================================================================
// setup_page.h - HTML of the setup page (served by setup_portal.cpp)
// ============================================================================
#pragma once
#include <pgmspace.h>

static const char kSetupPage[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Bus Stop Setup</title>
<style>
  body { font-family: -apple-system, system-ui, sans-serif; margin: 0; background: #121212; color: #e0e0e0; }
  main { max-width: 440px; margin: 0 auto; padding: 20px 16px 40px; }
  h1 { color: #00aaff; font-size: 24px; }
  h2 { font-size: 18px; margin-top: 28px; }
  label { display: block; margin: 14px 0 6px; font-size: 14px; color: #aaa; }
  input, button { width: 100%; box-sizing: border-box; font-size: 17px; padding: 12px; border-radius: 8px; border: 1px solid #444; background: #1e1e1e; color: #fff; }
  button { background: #00aaff; border: 0; color: #00131f; font-weight: 600; margin-top: 16px; }
  button.secondary { background: #2a2a2a; color: #e0e0e0; border: 1px solid #444; }
  button:disabled { opacity: .5; }
  #msg, #stopMsg { margin-top: 12px; min-height: 22px; color: #ffaa00; }
  .stop { padding: 12px; margin-top: 8px; border: 1px solid #444; border-radius: 8px; background: #1e1e1e; }
  .stop.selected { border-color: #00aaff; background: #12303f; }
  .hint { font-size: 14px; color: #8a94a6; line-height: 1.4; margin: 6px 0 0; }
  .hidden { display: none; }
</style>
</head>
<body>
<main>
  <h1>Bus Stop Setup</h1>

  <section id="step1">
    <h2>1. WiFi</h2>
    <label for="ssid">Network</label>
    <input id="ssid" list="nets" placeholder="Choose or type a network" autocapitalize="none" autocorrect="off">
    <datalist id="nets"></datalist>
    <label for="password">Password</label>
    <input id="password" type="password" autocapitalize="none" autocorrect="off">

    <h2>Västtrafik key</h2>
    <p class="hint">Every display needs its own free key. Get yours before you start at
      developer.vasttrafik.se: create an application and subscribe it to the Planera Resa v4 API.</p>
    <label for="clientId">Client ID</label>
    <input id="clientId" autocapitalize="none" autocorrect="off" spellcheck="false">
    <label for="clientSecret">Client secret</label>
    <input id="clientSecret" type="password" autocapitalize="none" autocorrect="off" spellcheck="false">

    <h2>Updates over WiFi</h2>
    <label for="otaPassword">Update password (optional)</label>
    <input id="otaPassword" type="password" autocapitalize="none" autocorrect="off">
    <p class="hint">Choose a password to update the display from your computer without a cable.
      Leave it empty to turn WiFi updates off.</p>
    <button class="secondary" id="rescan">Scan again</button>
    <button id="connect">Connect</button>
    <div id="msg"></div>
  </section>

  <section id="step2" class="hidden">
    <h2>2. Bus stop</h2>
    <label for="query">Search stop name</label>
    <input id="query" placeholder="e.g. Gräddgatan" autocapitalize="none" autocorrect="off">
    <button class="secondary" id="search">Search</button>
    <div id="results"></div>
    <div id="stopMsg"></div>
    <button id="save" disabled>Save and start</button>
  </section>
</main>

<script>
const $ = id => document.getElementById(id);
let chosenStop = null;

async function getJson(url, options) {
  const response = await fetch(url, options);
  return response.json();
}

async function scan() {
  $('rescan').disabled = true;
  $('msg').textContent = 'Scanning...';
  try {
    const networks = await getJson('/scan');
    $('nets').innerHTML = networks.map(n => '<option value="' + n.replace(/"/g, '&quot;') + '">').join('');
    $('msg').textContent = networks.length ? '' : 'No networks found.';
  } catch (e) {
    $('msg').textContent = 'Scan failed. Try again.';
  }
  $('rescan').disabled = false;
}

async function connect() {
  const ssid = $('ssid').value.trim();
  const clientId = $('clientId').value.trim();
  const clientSecret = $('clientSecret').value.trim();
  if (!ssid) { $('msg').textContent = 'Choose a network.'; return; }
  if (!clientId || !clientSecret) { $('msg').textContent = 'Enter your Västtrafik Client ID and secret.'; return; }
  $('connect').disabled = true;
  $('msg').textContent = 'Connecting...';

  const body = new URLSearchParams({ ssid: ssid, password: $('password').value,
    clientId: clientId, clientSecret: clientSecret, otaPassword: $('otaPassword').value });
  try { await fetch('/connect', { method: 'POST', body: body }); } catch (e) {}

  // The phone may lose the hotspot for a moment, so keep asking until we get an answer.
  for (let i = 0; i < 40; i++) {
    await new Promise(r => setTimeout(r, 1000));
    try {
      const status = await getJson('/status');
      if (status.state === 'connected') {
        $('step1').classList.add('hidden');
        $('step2').classList.remove('hidden');
        return;
      }
      if (status.state === 'failed') {
        $('msg').textContent = status.error || 'Could not connect.';
        break;
      }
    } catch (e) {}
  }
  if (!$('msg').textContent || $('msg').textContent === 'Connecting...') $('msg').textContent = 'Could not connect.';
  $('connect').disabled = false;
}

async function search() {
  const query = $('query').value.trim();
  if (!query) return;
  $('stopMsg').textContent = 'Searching...';
  $('results').innerHTML = '';
  chosenStop = null;
  $('save').disabled = true;
  try {
    const stops = await getJson('/stops?q=' + encodeURIComponent(query));
    if (!Array.isArray(stops)) throw new Error('bad');
    $('stopMsg').textContent = stops.length ? '' : 'No stops found.';
    stops.forEach(stop => {
      const div = document.createElement('div');
      div.className = 'stop';
      div.textContent = stop.name;
      div.onclick = () => {
        document.querySelectorAll('.stop').forEach(s => s.classList.remove('selected'));
        div.classList.add('selected');
        chosenStop = stop;
        $('save').disabled = false;
      };
      $('results').appendChild(div);
    });
  } catch (e) {
    $('stopMsg').textContent = 'Search failed. Try again.';
  }
}

async function save() {
  if (!chosenStop) return;
  $('save').disabled = true;
  const body = new URLSearchParams({ stopId: chosenStop.id });
  try {
    await fetch('/save', { method: 'POST', body: body });
    $('step2').innerHTML = '<h2>All set!</h2><p>The bus stop display is restarting with <b>' +
      chosenStop.name + '</b>. You can close this page.</p>';
  } catch (e) {
    $('stopMsg').textContent = 'Save failed. Try again.';
    $('save').disabled = false;
  }
}

$('rescan').onclick = scan;
$('connect').onclick = connect;
$('search').onclick = search;
$('query').addEventListener('keydown', e => { if (e.key === 'Enter') search(); });
$('save').onclick = save;
scan();
</script>
</body>
</html>
)rawliteral";
