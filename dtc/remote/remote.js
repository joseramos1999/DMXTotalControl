/*
  DMX Total Control - remote control: the SHOW screen of
  the computer on a phone or a tablet, through a WebSocket.
  Licensed under the Apache License, Version 2.0.
*/
"use strict";

const STRINGS = {
  es: {
    connecting: "Conectando…", offline: "Sin conexión con DMX Total Control. Reintentando…",
    wrongPin: "PIN incorrecto", noPages: "Este show no tiene páginas de SHOW todavía.",
    release: "Liberar", centre: "Centro", flash: "Flash", tap: "Tap", nothing: "No suena nada",
    onComputer: "Solo en el ordenador", master: "Master", noGroup: "Sin grupo",
    go: "GO", back: "Atrás", empty: "Vacío", page: "Página", playbacks: "Playbacks",
    released: "Liberada", on: "Encendida", off: "Apagada", paused: "En pausa",
    stopped: "Parada", start: "Iniciar", pause: "Pausa", reset: "Reiniciar",
    universe: "Universo %1",
    full: "Full", blackout: "Blackout", hue: "Tono", saturation: "Saturación",
    resetColour: "Restablecer color", strobe: "Estrobo", flashExcl: "Flash excl.", fullExcl: "Full excl.",
    fixture1: "%1 fixture", fixtures: "%1 fixtures", hz: "%1 Hz", ab: "A/B"
  },
  en: {
    connecting: "Connecting…", offline: "No connection with DMX Total Control. Trying again…",
    wrongPin: "Wrong PIN", noPages: "This show has no SHOW pages yet.",
    release: "Release", centre: "Centre", flash: "Flash", tap: "Tap", nothing: "Nothing plays",
    onComputer: "On the computer only", master: "Master", noGroup: "No group",
    go: "GO", back: "Back", empty: "Empty", page: "Page", playbacks: "Playbacks",
    released: "Released", on: "On", off: "Off", paused: "Paused",
    stopped: "Stopped", start: "Start", pause: "Pause", reset: "Reset",
    universe: "Universe %1",
    full: "Full", blackout: "Blackout", hue: "Hue", saturation: "Saturation",
    resetColour: "Reset colour", strobe: "Strobe", flashExcl: "Flash excl.", fullExcl: "Full excl.",
    fixture1: "%1 fixture", fixtures: "%1 fixtures", hz: "%1 Hz", ab: "A/B"
  },
  pt: {
    connecting: "Conectando…", offline: "Sem conexão com o DMX Total Control. Tentando de novo…",
    wrongPin: "PIN incorreto", noPages: "Este show ainda não tem páginas de SHOW.",
    release: "Liberar", centre: "Centro", flash: "Flash", tap: "Tap", nothing: "Nada tocando",
    onComputer: "Só no computador", master: "Master", noGroup: "Sem grupo",
    go: "GO", back: "Voltar", empty: "Vazio", page: "Página", playbacks: "Playbacks",
    released: "Liberada", on: "Ligada", off: "Desligada", paused: "Em pausa",
    stopped: "Parada", start: "Iniciar", pause: "Pausar", reset: "Zerar",
    universe: "Universo %1",
    full: "Full", blackout: "Blackout", hue: "Matiz", saturation: "Saturação",
    resetColour: "Redefinir cor", strobe: "Strobo", flashExcl: "Flash excl.", fullExcl: "Full excl.",
    fixture1: "%1 fixture", fixtures: "%1 fixtures", hz: "%1 Hz", ab: "A/B"
  },
  fr: {
    connecting: "Connexion…", offline: "Pas de connexion avec DMX Total Control. Nouvel essai…",
    wrongPin: "PIN incorrect", noPages: "Ce show n'a pas encore de pages SHOW.",
    release: "Libérer", centre: "Centre", flash: "Flash", tap: "Tap", nothing: "Rien ne joue",
    onComputer: "Sur l'ordinateur seulement", master: "Master", noGroup: "Sans groupe",
    go: "GO", back: "Retour", empty: "Vide", page: "Page", playbacks: "Playbacks",
    released: "Libérée", on: "Allumée", off: "Éteinte", paused: "En pause",
    stopped: "Arrêtée", start: "Démarrer", pause: "Pause", reset: "Réinitialiser",
    universe: "Univers %1",
    full: "Full", blackout: "Blackout", hue: "Teinte", saturation: "Saturation",
    resetColour: "Réinitialiser la couleur", strobe: "Stroboscope", flashExcl: "Flash excl.", fullExcl: "Full excl.",
    fixture1: "%1 fixture", fixtures: "%1 fixtures", hz: "%1 Hz", ab: "A/B"
  },
  it: {
    connecting: "Connessione…", offline: "Nessuna connessione con DMX Total Control. Nuovo tentativo…",
    wrongPin: "PIN errato", noPages: "Questo show non ha ancora pagine SHOW.",
    release: "Rilascia", centre: "Centro", flash: "Flash", tap: "Tap", nothing: "Non suona niente",
    onComputer: "Solo sul computer", master: "Master", noGroup: "Nessun gruppo",
    go: "GO", back: "Indietro", empty: "Vuoto", page: "Pagina", playbacks: "Playback",
    released: "Rilasciata", on: "Accesa", off: "Spenta", paused: "In pausa",
    stopped: "Ferma", start: "Avvia", pause: "Pausa", reset: "Azzera",
    universe: "Universo %1",
    full: "Full", blackout: "Blackout", hue: "Tonalità", saturation: "Saturazione",
    resetColour: "Ripristina colore", strobe: "Strobo", flashExcl: "Flash escl.", fullExcl: "Full escl.",
    fixture1: "%1 fixture", fixtures: "%1 fixture", hz: "%1 Hz", ab: "A/B"
  },
  de: {
    connecting: "Verbinden…", offline: "Keine Verbindung mit DMX Total Control. Neuer Versuch…",
    wrongPin: "Falsche PIN", noPages: "Diese Show hat noch keine SHOW-Seiten.",
    release: "Freigeben", centre: "Mitte", flash: "Flash", tap: "Tap", nothing: "Nichts läuft",
    onComputer: "Nur am Computer", master: "Master", noGroup: "Keine Gruppe",
    go: "GO", back: "Zurück", empty: "Leer", page: "Seite", playbacks: "Playbacks",
    released: "Freigegeben", on: "An", off: "Aus", paused: "Pausiert",
    stopped: "Gestoppt", start: "Starten", pause: "Pause", reset: "Zurücksetzen",
    universe: "Universum %1",
    full: "Voll", blackout: "Blackout", hue: "Farbton", saturation: "Sättigung",
    resetColour: "Farbe zurücksetzen", strobe: "Stroboskop", flashExcl: "Flash exkl.", fullExcl: "Voll exkl.",
    fixture1: "%1 Fixture", fixtures: "%1 Fixtures", hz: "%1 Hz", ab: "A/B"
  }
};
// the language of the phone, if the remote has it
const LANG = ((code) => STRINGS[code] ? code : "en")((navigator.language || "en").toLowerCase().slice(0, 2));
const tr = (key) => STRINGS[LANG][key] || key;
document.documentElement.lang = LANG;

// what the page keeps for this visit only (the way to the name and back,
// moveToName); a browser that keeps nothing: nothing
const session = {
  get: (key) => { try { return sessionStorage.getItem(key); } catch (err) { return null; } },
  set: (key, value) => { try { sessionStorage.setItem(key, value); } catch (err) { /* (kept nowhere) */ } },
  remove: (key) => { try { sessionStorage.removeItem(key); } catch (err) { /* (kept nowhere) */ } }
};
// an address of numbers, as the QR code has it ("192.168.1.20"), and the
// page there ("http://192.168.1.20:8800/")
const NUMERIC_HOST = /^\d{1,3}(\.\d{1,3}){3}$/;
const NUMERIC_PAGE = /^http:\/\/\d{1,3}(\.\d{1,3}){3}:\d+\/$/;

// the QR code of Options opens "#pin=…" (and "&page=…"): kept as if they
// were typed, then taken out of the address bar and the history, so that
// the PIN doesn't show (what follows '#' never reaches the computer). The
// page that goes to the name and back adds "from" (the address it came
// from) and "stay" (back on the address: it stays there). Whether it had a
// PIN
function takeFromAddress() {
  if (!location.hash)
    return false;
  const given = new URLSearchParams(location.hash.slice(1));
  history.replaceState(null, "", location.pathname + location.search);
  if (/^\d+$/.test(given.get("page") || ""))
    localStorage.setItem("dtc.page", given.get("page"));
  // (only an address of numbers of this program: never a page elsewhere)
  if (NUMERIC_PAGE.test(given.get("from") || ""))
    session.set("dtc.from", given.get("from"));
  if (given.has("stay"))
    session.set("dtc.stay", "1");
  if (!given.has("pin"))
    return false;
  localStorage.setItem("dtc.pin", given.get("pin").trim());
  return true;
}
takeFromAddress();

let socket = null;
let layout = null;
let state = null;
let pageIndex = Number(localStorage.getItem("dtc.page") || 0);
let pin = localStorage.getItem("dtc.pin") || "";
let updaters = [];
// the DMX levels of the universes the page shows (universe -> 512 bytes),
// sent by the computer only while they are on screen; the sections that
// draw them, and the universes they show
const levels = new Map();
let levelPainters = [];
let watched = [];
// the timers count on the clock of the computer (ms): the clock of the
// phone minus clockOffset (set by the layout and the "clock" messages)
let clockOffset = 0;
function serverNow() {
  return performance.now() - clockOffset;
}

const $ = (id) => document.getElementById(id);
const el = (tag, cls, text) => {
  const e = document.createElement(tag);
  if (cls) e.className = cls;
  if (text !== undefined) e.textContent = text;
  return e;
};

/* ------------------------------------------------------------ connection */

function send(message) {
  if (socket && socket.readyState === WebSocket.OPEN)
    socket.send(JSON.stringify(message));
}

function connect() {
  socket = new WebSocket((location.protocol === "https:" ? "wss://" : "ws://") + location.host + "/");
  let opened = false;
  socket.onopen = () => {
    opened = true;
    failures = 0;
    send({ cmd: "hello", pin: pin });
  };
  socket.onmessage = (event) => {
    const m = JSON.parse(event.data);
    if (m.type === "auth") {
      $("offline").classList.add("hidden");
      if (!m.ok) {
        $("pin").classList.remove("hidden");
        $("pinError").textContent = pin ? tr("wrongPin") : "";
        $("pinError").classList.toggle("hidden", !pin);
        $("pinInput").focus();
      } else {
        $("pin").classList.add("hidden");
        $("status").classList.add("ok");
      }
    } else if (m.type === "layout") {
      if (m.clock !== undefined)
        clockOffset = performance.now() - m.clock;
      layout = m;
      render();
      moveToName();
    } else if (m.type === "clock") {
      clockOffset = performance.now() - m.now;
    } else if (m.type === "state") {
      state = m;
      update();
    } else if (m.type === "levels") {
      const changed = [];
      for (const u in m.universes || {}) {
        const bytes = atob(m.universes[u]), values = new Uint8Array(bytes.length);
        for (let i = 0; i < bytes.length; i++)
          values[i] = bytes.charCodeAt(i);
        levels.set(Number(u), values);
        changed.push(Number(u));
      }
      paintLevels(changed);
    }
  };
  socket.onclose = () => {
    // (the computer lets go of what a phone that goes holds down)
    releaseHolds(false);
    $("status").classList.remove("ok");
    $("offlineText").textContent = tr("offline");
    $("offline").classList.remove("hidden");
    if (!opened)
      nameFailed();
    setTimeout(connect, 2000);
  };
}

/* ------------------------------------------------------------ the name */

/* A page opened by an address of numbers (the QR code: every phone opens
   it) goes to the name of the computer (http://dmxtotalcontrol.local:8800/)
   when it reaches this same computer by it: an icon of the home screen made
   then still opens it when the router gives the computer another address.
   It asks once per visit, after a layout; a phone that can't stays on the
   address without noticing anything. The PIN and the page go along after
   '#' (the name is another site for the browser: what the page keeps here
   isn't there), and the address it came from too */
let nameAsked = false;
function moveToName() {
  if (nameAsked || !layout.host || !layout.instance || !layout.port)
    return;
  if (!NUMERIC_HOST.test(location.hostname) || session.get("dtc.stay"))
    return;
  nameAsked = true;
  // (once: back here, when the name stops working, it stays)
  session.set("dtc.stay", "1");
  const instance = layout.instance, page = "http://" + layout.host + ":" + layout.port + "/";
  whoIsAt(page).then((who) => {
    // (no answer, or another computer: an old answer for the name)
    if (who && who.id === instance)
      goTo(page, { from: location.origin + "/" });
  });
}

/* The name stopped answering (3 tries in a row, none opened): back to the
   address it came from, when that one answers (none does when the computer
   is off or the program closed: the name is tried again) */
let failures = 0, goingBack = false;
function nameFailed() {
  const from = session.get("dtc.from");
  if (!from || goingBack || ++failures < 3)
    return;
  goingBack = true;
  whoIsAt(from).then((who) => {
    goingBack = false;
    failures = 0;
    // (the name answered meanwhile: it stays)
    if (!who || !who.id || (socket && socket.readyState === WebSocket.OPEN))
      return;
    session.remove("dtc.from");
    goTo(from, { stay: "1" });
  });
}

// the program that answers at the page $page ({id, host}), or null when
// none does within 2.5 s
function whoIsAt(page) {
  if (typeof fetch !== "function")
    return Promise.resolve(null);
  const abort = typeof AbortController === "function" ? new AbortController() : null;
  const late = new Promise((resolve) => setTimeout(() => {
    if (abort) abort.abort();
    resolve(null);
  }, 2500));
  const asked = fetch(page + "whoami", { cache: "no-store", signal: abort ? abort.signal : undefined })
    .then((reply) => reply.ok ? reply.json() : null)
    .catch(() => null);
  return Promise.race([asked, late]);
}

// to the page $page, with the PIN, the page shown and $more after '#',
// once no finger is on the screen (nothing held down is let go of)
function goTo(page, more) {
  if (fingers.size > 0) {
    setTimeout(() => goTo(page, more), 300);
    return;
  }
  const carried = new URLSearchParams(more);
  if (pin)
    carried.set("pin", pin);
  carried.set("page", String(pageIndex));
  location.replace(page + "#" + carried.toString());
}

$("pinForm").addEventListener("submit", (e) => {
  e.preventDefault();
  pin = $("pinInput").value.trim();
  localStorage.setItem("dtc.pin", pin);
  send({ cmd: "hello", pin: pin });
});
// (the code scanned again while the page is open: only what follows '#' changes)
window.addEventListener("hashchange", () => {
  if (!takeFromAddress())
    return;
  pin = localStorage.getItem("dtc.pin") || "";
  send({ cmd: "hello", pin: pin });
});
$("blackout").addEventListener("click", () => send({ cmd: "global", action: "blackout" }));
$("fullscreen").addEventListener("click", () => {
  if (document.fullscreenElement)
    document.exitFullscreen();
  else if (document.documentElement.requestFullscreen)
    document.documentElement.requestFullscreen();
});

/* ------------------------------------------------------------ helpers */

// continuous controls send at most once per frame
const pending = new Map();
function sendSoon(key, message) {
  const wasEmpty = pending.size === 0;
  pending.set(key, message);
  if (wasEmpty)
    requestAnimationFrame(() => {
      pending.forEach((m) => send(m));
      pending.clear();
    });
}

function drag(target, onMove, onEnd) {
  target.addEventListener("pointerdown", (e) => {
    target.setPointerCapture(e.pointerId);
    onMove(e, true);
    const move = (ev) => onMove(ev, false);
    const up = () => {
      target.removeEventListener("pointermove", move);
      target.removeEventListener("pointerup", up);
      target.removeEventListener("pointercancel", up);
      if (onEnd) onEnd();
    };
    target.addEventListener("pointermove", move);
    target.addEventListener("pointerup", up);
    target.addEventListener("pointercancel", up);
  });
}

/* What the fingers hold down: the flashes of the playbacks, the SHOW buttons
   and the Flash of the SHOW faders. "what" -> { up: the message that lets go
   of it, pointers }. It goes down with the first finger on it and up with the
   last one, wherever that finger is let go: a new layout replaces the buttons
   under the fingers, and their release still has to reach the computer */
const holds = new Map();
let holdPainters = [];
function paintHolds() {
  holdPainters.forEach((paint) => paint());
}
function holdDown(what, pointerId, down, up) {
  let hold = holds.get(what);
  if (!hold) {
    hold = { up: up, pointers: new Set() };
    holds.set(what, hold);
    send(down);
  }
  hold.pointers.add(pointerId);
  paintHolds();
}
function holdUp(pointerId) {
  let changed = false;
  holds.forEach((hold, what) => {
    if (!hold.pointers.delete(pointerId) || hold.pointers.size > 0) return;
    holds.delete(what);
    send(hold.up);
    changed = true;
  });
  if (changed) paintHolds();
}
function releaseHolds(tell) {
  if (tell) holds.forEach((hold) => send(hold.up));
  holds.clear();
  paintHolds();
}
// a finger on an element: it keeps the element (a pointer already gone can't)
function capture(element, pointerId) {
  try { element.setPointerCapture(pointerId); } catch (err) { /* (a pointer already gone) */ }
}
function flashKey(pbPage, slot) {
  return "playback/" + pbPage + "/" + slot;
}
function flashMessage(pbPage, slot, down) {
  return { cmd: "playback", action: "flash", pbPage: pbPage, slot: slot, down: down };
}
// the fingers on the screen now (the page goes to the name only with none)
const fingers = new Set();
document.addEventListener("pointerdown", (e) => fingers.add(e.pointerId), true);
function fingerUp(e) {
  fingers.delete(e.pointerId);
  holdUp(e.pointerId);
}
document.addEventListener("pointerup", fingerUp);
document.addEventListener("pointercancel", fingerUp);
// (the phone put away with a finger on something; and no DMX levels while
// the page isn't seen, the ones of now when it is again)
document.addEventListener("visibilitychange", () => {
  if (document.hidden) {
    releaseHolds(true);
    fingers.clear();
  }
  watchLevels();
});

/* The DMX levels: the computer sends the universes the page shows (none
   while the page is hidden), and the sections draw them as they come */
function watchLevels() {
  send({ cmd: "monitor", universes: document.hidden ? [] : watched });
}
function paintLevels(universes) {
  levelPainters.forEach((p) => { if (!universes || universes.indexOf(p.universe) >= 0) p.paint(); });
}

function hsvToHex(h, s, v) {
  const f = (n) => {
    const k = (n + h * 6) % 6;
    return v - v * s * Math.max(0, Math.min(k, 4 - k, 1));
  };
  const c = (x) => Math.round(x * 255).toString(16).padStart(2, "0");
  return "#" + c(f(5)) + c(f(3)) + c(f(1));
}

function hexToHsv(hex) {
  const r = parseInt(hex.substr(1, 2), 16) / 255, g = parseInt(hex.substr(3, 2), 16) / 255, b = parseInt(hex.substr(5, 2), 16) / 255;
  const max = Math.max(r, g, b), min = Math.min(r, g, b), d = max - min;
  let h = 0;
  if (d > 0) {
    if (max === r) h = ((g - b) / d) % 6;
    else if (max === g) h = (b - r) / d + 2;
    else h = (r - g) / d + 4;
    h /= 6;
    if (h < 0) h += 1;
  }
  return { h: h, s: max === 0 ? 0 : d / max, v: max };
}

/* A section with its own background (as on the computer): a tint of the
   surface, 35 % of the colour (dimmed at night, as the colours of the
   buttons); its texts keep reading (WCAG 4.5, else the text of the theme or
   the opposite of it: no white glare at night) */
function rgbOf(hex) {
  const m = /^#([0-9a-f]{6})$/i.exec(String(hex).trim());
  if (!m) return null;
  const n = parseInt(m[1], 16);
  return [(n >> 16) & 255, (n >> 8) & 255, n & 255];
}
function luminance(rgb) {
  const lin = (v) => { v /= 255; return v <= 0.03928 ? v / 12.92 : Math.pow((v + 0.055) / 1.055, 2.4); };
  return 0.2126 * lin(rgb[0]) + 0.7152 * lin(rgb[1]) + 0.0722 * lin(rgb[2]);
}
function contrast(a, b) {
  const x = luminance(a), y = luminance(b);
  return (Math.max(x, y) + 0.05) / (Math.min(x, y) + 0.05);
}
function tintSection(section, colour) {
  const css = getComputedStyle(document.documentElement);
  const surface = rgbOf(css.getPropertyValue("--surface")), tint = rgbOf(colour);
  if (!surface || !tint) return;
  const level = (parseFloat(css.getPropertyValue("--category")) || 100) / 100;
  // 35 %, less where the faintest text of the theme wouldn't read (2:1),
  // as the program does (SectionView)
  const mixAt = (t) => surface.map((s, i) => Math.round(s + (tint[i] * level - s) * t));
  const faint = rgbOf(css.getPropertyValue("--faint"));
  let t = 0.35, bg = mixAt(t);
  while (faint && contrast(faint, bg) < 2.0 && t > 0.1) {
    t -= 0.05;
    bg = mixAt(t);
  }
  section.style.background = "rgb(" + bg.join(",") + ")";
  const text = rgbOf(css.getPropertyValue("--text"));
  if (!text) return;
  const other = luminance(text) > 0.18 ? [17, 17, 17] : [255, 255, 255];
  const best = contrast(text, bg) >= contrast(other, bg) ? text : other;
  for (const name of ["--text", "--dim", "--faint"]) {
    const c = rgbOf(css.getPropertyValue(name));
    if (c && contrast(c, bg) < 4.5)
      section.style.setProperty(name, "rgb(" + best.join(",") + ")");
  }
}

function sizeCanvas(canvas) {
  const ratio = window.devicePixelRatio || 1;
  const w = canvas.clientWidth, h = canvas.clientHeight;
  if (canvas.width !== Math.round(w * ratio) || canvas.height !== Math.round(h * ratio)) {
    canvas.width = Math.round(w * ratio);
    canvas.height = Math.round(h * ratio);
  }
  const ctx = canvas.getContext("2d");
  ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
  return { ctx: ctx, w: w, h: h };
}

/* ------------------------------------------------------------ rendering */

function render() {
  updaters = [];
  holdPainters = [];
  levelPainters = [];
  watched = [];
  const pages = layout ? layout.pages : [];
  $("title").textContent = layout && layout.title ? layout.title : "DMX Total Control";
  document.title = $("title").textContent;
  // the theme of the program
  const theme = layout && layout.theme ? layout.theme : "dark";
  document.documentElement.dataset.theme = theme;
  const themeColor = document.querySelector('meta[name="theme-color"]');
  if (themeColor) themeColor.content = getComputedStyle(document.documentElement).getPropertyValue("--bg").trim();
  if (pageIndex >= pages.length) pageIndex = 0;

  const tabs = $("pages");
  tabs.innerHTML = "";
  pages.forEach((p, i) => {
    const b = el("button", i === pageIndex ? "current" : "", p.name || String(i + 1));
    b.addEventListener("click", () => {
      pageIndex = i;
      localStorage.setItem("dtc.page", String(i));
      render();
    });
    tabs.appendChild(b);
  });

  const surface = $("surface");
  surface.innerHTML = "";
  if (pages.length === 0) {
    surface.appendChild(el("div", "placeholder", tr("noPages")));
    watchLevels();
    return;
  }
  surface.appendChild(buildNode(pages[pageIndex].root));
  paintHolds();
  update();
  // (the levels of the universes this page shows, and no others)
  watchLevels();
}

function buildNode(node) {
  if (node.split === "h" || node.split === "v") {
    const box = el("div", "split " + node.split);
    const a = buildNode(node.a), b = buildNode(node.b);
    const ratio = node.ratio || 0.5;
    a.style.flex = ratio + " 1 0";
    b.style.flex = (1 - ratio) + " 1 0";
    box.appendChild(a);
    box.appendChild(b);
    return box;
  }
  const section = el("div", "section" + (node.type === "empty" && !node.background ? " empty" : ""));
  if (node.background)
    tintSection(section, node.background);
  if (node.title)
    section.appendChild(el("div", "sectionTitle", node.title));
  const content = el("div", "content");
  section.appendChild(content);
  const builder = BUILDERS[node.type];
  if (builder)
    builder(content, node);
  else if (node.type !== "empty")
    content.appendChild(el("div", "placeholder", tr("onComputer")));
  return section;
}

function key(node, cell) {
  return pageIndex + "/" + node.id + (cell === undefined ? "" : "/" + cell);
}

/* A SHOW fader or dial: the value at a position of its travel (its range,
   0..1; an older computer sends none) and its text, % or BPM */
function rangeValue(item, v) {
  // (a fader that plays a scene at a speed stops it at the end of its travel)
  if (item.run && (item.invert ? v >= 1 : v <= 0)) return 0;
  const min = item.min !== undefined ? item.min : 0, max = item.max !== undefined ? item.max : 1;
  return min + (max - min) * (item.invert ? 1 - v : v);
}
/* The text of a value under a fader or dial: smaller where a long word
   («Stopped») doesn't fit a narrow cell, as the program does */
function setValueText(el, text) {
  if (el.textContent === String(text)) return;
  el.textContent = text;
  el.style.fontSize = "";
  if (el.clientWidth > 0 && el.scrollWidth > el.clientWidth)
    el.style.fontSize = Math.max(7, Math.floor(10 * el.clientWidth / el.scrollWidth)) + "px";
}
function valueText(item, value) {
  if (item.bpm) return Math.round(20 + value * 280);
  // a speed: x0.1 at the bottom, x1 in the middle, x10 at the top (a scene
  // played at a speed: stopped at zero)
  if (item.speed) {
    if (item.run && value <= 0) return tr("stopped");
    const factor = Math.pow(10, value * 2 - 1);
    return "×" + factor.toFixed(factor < 1 ? 2 : 1);
  }
  return Math.round(value * 100) + "%";
}
// the value of the state (an older computer: from the position)
function stateValue(item, st) {
  return st.value !== undefined ? st.value : rangeValue(item, st.level);
}

/* The time of a timer section, as the program shows it (SectionView): mm:ss
   or h:mm:ss, and the tenths of a stopwatch */
function timerText(ms, countdown) {
  const s = Math.floor(ms / 1000);
  const h = Math.floor(s / 3600), m = Math.floor((s % 3600) / 60), sec = s % 60;
  const pad = (n) => (n < 10 ? "0" : "") + n;
  return (h > 0 ? h + ":" + pad(m) : pad(m)) + ":" + pad(sec) + (countdown ? "" : "." + Math.floor((ms % 1000) / 100));
}

const TYPE_COLOURS = {
  Scene: "#3d8bfd", Chaser: "#f59f00", Sequence: "#f76707", EFX: "#ae3ec9", RGBMatrix: "#2fb344",
  CurveFX: "#e8590c", FXScene: "#be4bdb", Collection: "#17a2b8", Show: "#d6336c", Cuelist: "#f03e3e",
  Audio: "#74b816", Video: "#74b816"
};

/* A playback: its name, the cue it plays and the next one, GO, Back, its
   fader (its intensity) and Flash, held while the finger is down */
function playbackStrip(node, slot, pbPage) {
  const strip = el("div", "pbstrip empty");
  // the number on its colour (lit while it plays), the name under it
  const top = el("div", "top"), number = el("span", "number", String(slot + 1)), name = el("div", "name", "");
  top.appendChild(number);
  strip.appendChild(top);
  strip.appendChild(name);
  const cue = el("div", "cue", ""), nextCue = el("div", "next", ""), progress = el("div", "progress"), bar = el("div", "bar");
  progress.appendChild(bar);
  if (node.showCue !== false) {
    strip.appendChild(cue);
    strip.appendChild(nextCue);
    strip.appendChild(progress);
  }
  const at = () => ({ cmd: "playback", pbPage: pbPage(), slot: slot });

  // GO and Back on a tap (click): the strips scroll sideways on a phone, and
  // a swipe that starts on one of them must not fire it
  const go = el("button", "go", tr("go"));
  go.addEventListener("click", () => send(Object.assign(at(), { action: "go" })));
  strip.appendChild(go);
  if (node.showBack !== false) {
    const back = el("button", "back", tr("back"));
    back.addEventListener("click", () => send(Object.assign(at(), { action: "back" })));
    strip.appendChild(back);
  }

  const track = el("div", "track"), fill = el("div", "fill"), knob = el("div", "knob"), value = el("div", "value", "");
  track.appendChild(fill);
  track.appendChild(knob);
  strip.appendChild(track);
  strip.appendChild(value);
  let level = 0, dragging = false, empty = true;
  // an A/B fader is no intensity: «A/B» and how far its crossfade is, as on
  // the computer (each travel is a GO)
  let crossfade = false, fading = "";
  const show = (v) => {
    fill.style.height = (v * 100) + "%";
    knob.style.bottom = (v * 100) + "%";
    value.textContent = empty ? "" : crossfade ? tr("ab") + fading : Math.round(v * 100) + "%";
  };
  show(0);
  // (the state does not move a fader while a finger drags it)
  drag(track, (e) => {
    dragging = true;
    const r = track.getBoundingClientRect();
    level = Math.max(0, Math.min(1, 1 - (e.clientY - r.top) / r.height));
    show(level);
    sendSoon("pb" + slot, Object.assign(at(), { action: "level", value: level }));
  }, () => { dragging = false; });

  let flash = null;
  if (node.showFlash !== false) {
    // a real press: down while a finger is on it, up when the last one leaves
    // (the fingers are followed by the page: see holdDown)
    flash = el("button", "flash", tr("flash"));
    flash.addEventListener("pointerdown", (e) => {
      capture(flash, e.pointerId);
      const page = pbPage();
      holdDown(flashKey(page, slot), e.pointerId, flashMessage(page, slot, true), flashMessage(page, slot, false));
    });
    const button = flash;
    holdPainters.push(() => button.classList.toggle("pressed", holds.has(flashKey(pbPage(), slot))));
    strip.appendChild(flash);
  }

  updaters.push((s) => {
    const p = (s.playbacks || {})[pbPage() + "/" + slot];
    empty = !p;
    strip.classList.toggle("empty", empty);
    // short (a phone on its side, a small section): one line of name and cue
    strip.classList.toggle("compact", strip.clientHeight > 0 && strip.clientHeight < 340);
    if (!p) {
      name.textContent = tr("empty");
      cue.textContent = "";
      nextCue.textContent = "";
      bar.style.width = "0%";
      go.className = "go";
      if (flash) flash.classList.remove("active");
      crossfade = false;
      track.classList.remove("ab");
      if (!dragging) { level = 0; show(0); }
      return;
    }
    strip.style.setProperty("--tint", p.color || TYPE_COLOURS[p.type] || "#868e96");
    name.textContent = p.name || "";
    // 1 on, 2 fading or waiting, 3 paused
    const running = p.state >= 1;
    strip.classList.toggle("playing", running);
    go.className = "go" + (running ? " active" : "") + (p.state === 2 ? " waiting" : "") + (p.state === 3 ? " paused" : "");
    if (p.type === "Cuelist") {
      cue.textContent = running && p.cueNumber ? p.cueNumber + (p.cueName ? " · " + p.cueName : "") : tr("released");
      nextCue.textContent = p.nextNumber ? "▸ " + p.nextNumber + (p.nextName ? " · " + p.nextName : "") : "";
    } else {
      cue.textContent = p.state === 3 ? tr("paused") : running ? tr("on") : tr("off");
      nextCue.textContent = "";
    }
    cue.classList.toggle("on", running);
    bar.style.width = running ? (Math.max(0, Math.min(1, p.progress)) * 100) + "%" : "0%";
    if (flash) flash.classList.toggle("active", !!p.flashing);
    crossfade = !!p.crossfade;
    fading = running ? " " + Math.round(Math.max(0, Math.min(1, p.progress)) * 100) + "%" : "";
    track.classList.toggle("ab", crossfade);
    // (a finger on it: the fader stays under it, its text goes on)
    if (!dragging)
      level = p.level;
    show(level);
  });
  return strip;
}

// the rhythm of a scene with steps (a cuelist) inside its button: two halves
// that light one after the other, a step each (an FX at the BPM of the show:
// a beat each); only while it plays
function rhythmBar(button, functionId) {
  const bar = el("div", "rhythm"), halves = [el("div", "half"), el("div", "half")];
  halves.forEach((half) => bar.appendChild(half));
  button.appendChild(bar);
  let last = -1, left = true;
  updaters.push((s) => {
    const steps = s.steps || {};
    const step = steps[String(functionId)] !== undefined ? steps[String(functionId)] : -1;
    if (step >= 0 && last >= 0 && step !== last) left = !left;
    last = step;
    bar.classList.toggle("on", step >= 0);
    halves[0].classList.toggle("lit", left);
    halves[1].classList.toggle("lit", !left);
  });
}

/* A bar the finger drags sideways (the hue, saturation and strobe speed of a
   mixer): filled up to its value, or a rainbow (the hue), and its knob */
function hbar(rainbow, onMove, onEnd) {
  const bar = el("div", "hbar"), fill = el("div", rainbow ? "rainbow" : "fill"), knob = el("div", "knob");
  bar.appendChild(fill);
  bar.appendChild(knob);
  drag(bar, (e, first) => {
    const r = bar.getBoundingClientRect();
    onMove(Math.max(0, Math.min(1, (e.clientX - r.left) / r.width)), first);
  }, onEnd);
  return {
    bar: bar, fill: fill,
    show(v) {
      if (!rainbow) fill.style.width = (v * 100) + "%";
      // (the knob, 10 px, inside the bar at both ends)
      knob.style.left = "calc(" + (v * 100) + "% - " + (v * 10) + "px)";
    }
  };
}

// "1 fixture", "3 fixtures" (French and Portuguese: 0 is singular too, as the program says)
function fixturesText(n) {
  const one = LANG === "fr" || LANG === "pt" ? n <= 1 : n === 1;
  return tr(one ? "fixture1" : "fixtures").replace("%1", n);
}

/* A strip of the live mixer, as on the computer (MixerStrip): the dimmer
   with Full and Blackout, the colour (hue, saturation) and its reset, the
   strobe and its speed, Flash and Flash excl. (while a finger is on them)
   and Full excl. Its border lights while a flash or a full is on, and goes
   red on blackout */
function mixerStrip(m) {
  const strip = el("div", "mstrip");
  strip.appendChild(el("div", "name", m.name || ""));
  strip.appendChild(el("div", "count", fixturesText(m.fixtures || 0)));
  // what it shows: the settings of the computer, and what a finger moves
  const v = { dimmer: 1, colour: false, hue: 0, saturation: 1, strobeHz: 8 };
  // the controls a finger is on (and 400 ms after): the state doesn't move
  // them; then what the computer made of the last move (the state comes
  // only when it changes)
  const busy = {};
  const grab = (key) => { clearTimeout(busy[key]); busy[key] = true; };
  const letGo = (key) => {
    busy[key] = setTimeout(() => {
      delete busy[key];
      const settings = state && state.mixers ? state.mixers[String(m.id)] : null;
      if (settings && settings[key] !== undefined) v[key] = settings[key];
      paint();
    }, 400);
  };
  const set = (key, value) => sendSoon("mx" + m.id + key, { cmd: "mixer", mixer: m.id, key: key, value: value });
  // the buttons lit by their setting
  const lit = new Map();
  const toggle = (key, text, cls) => {
    const b = el("button", cls || "", text);
    // (on a tap: a swipe that starts on it scrolls the strips)
    b.addEventListener("click", () => send({ cmd: "mixer", mixer: m.id, key: key, toggle: true }));
    lit.set(key, b);
    return b;
  };
  const flash = (key, text) => {
    const b = el("button", "flash warn", text), held = "mixer/" + m.id + "/" + key;
    b.addEventListener("pointerdown", (e) => {
      capture(b, e.pointerId);
      holdDown(held, e.pointerId, { cmd: "mixerFlash", mixer: m.id, key: key, down: true },
               { cmd: "mixerFlash", mixer: m.id, key: key, down: false });
    });
    holdPainters.push(() => b.classList.toggle("pressed", holds.has(held)));
    lit.set(key, b);
    return b;
  };

  // the dimmer: its fader, the % and Full and Blackout beside it
  const dim = el("div", "dim"), track = el("div", "track"), fill = el("div", "fill"), knob = el("div", "knob");
  track.appendChild(fill);
  track.appendChild(knob);
  const side = el("div", "side"), percent = el("div", "value", "");
  side.appendChild(percent);
  side.appendChild(toggle("full", tr("full")));
  side.appendChild(toggle("blackout", tr("blackout"), "danger"));
  dim.appendChild(track);
  dim.appendChild(side);
  strip.appendChild(dim);
  drag(track, (e, first) => {
    if (first) grab("dimmer");
    const r = track.getBoundingClientRect();
    v.dimmer = Math.max(0, Math.min(1, 1 - (e.clientY - r.top) / r.height));
    paint();
    set("dimmer", v.dimmer);
  }, () => letGo("dimmer"));

  // the colour: it replaces the one of the scenes until it is reset
  strip.appendChild(el("div", "label", tr("hue")));
  const hue = hbar(true, (x, first) => {
    if (first) grab("hue");
    v.hue = x;
    v.colour = true;
    paint();
    set("hue", x);
  }, () => letGo("hue"));
  strip.appendChild(hue.bar);
  strip.appendChild(el("div", "label", tr("saturation")));
  const saturation = hbar(false, (x, first) => {
    if (first) grab("saturation");
    v.saturation = x;
    v.colour = true;
    paint();
    set("saturation", x);
  }, () => letGo("saturation"));
  strip.appendChild(saturation.bar);
  const reset = el("button", "", tr("resetColour"));
  reset.addEventListener("click", () => send({ cmd: "mixer", mixer: m.id, key: "resetColour" }));
  strip.appendChild(reset);

  // the strobe, 0.5 to 20 Hz by 0.5
  strip.appendChild(toggle("strobe", tr("strobe"), "warn"));
  const hz = hbar(false, (x, first) => {
    if (first) grab("strobeHz");
    v.strobeHz = 0.5 + Math.round(x * 39) * 0.5;
    paint();
    set("strobeHz", v.strobeHz);
  }, () => letGo("strobeHz"));
  strip.appendChild(hz.bar);
  const hzText = el("div", "value", "");
  strip.appendChild(hzText);

  // the flashes: on while a finger is on them
  strip.appendChild(flash("flash", tr("flash")));
  strip.appendChild(flash("flashExclusive", tr("flashExcl")));
  strip.appendChild(toggle("fullExclusive", tr("fullExcl"), "warn"));

  const paint = () => {
    const on = v.flash || v.flashExclusive || v.full || v.fullExclusive;
    strip.classList.toggle("lit", !!on);
    strip.classList.toggle("dark", !on && !!v.blackout);
    fill.style.height = (v.dimmer * 100) + "%";
    knob.style.bottom = (v.dimmer * 100) + "%";
    // (in the colour of the mixer, as on the computer)
    fill.style.background = v.colour ? hsvToHex(v.hue, v.saturation, 1) : "";
    percent.textContent = Math.round(v.dimmer * 100) + "%";
    hue.show(v.hue);
    saturation.show(v.saturation);
    saturation.fill.style.background = hsvToHex(v.hue, 1, 1);
    hz.show((v.strobeHz - 0.5) / 19.5);
    hzText.textContent = tr("hz").replace("%1", v.strobeHz.toLocaleString(LANG, { minimumFractionDigits: 1, maximumFractionDigits: 1 }));
    reset.disabled = !v.colour;
    lit.forEach((b, key) => b.classList.toggle("on", !!v[key]));
  };
  paint();
  updaters.push((s) => {
    const settings = (s.mixers || {})[String(m.id)];
    if (!settings) return;
    for (const k in settings)
      if (!busy[k]) v[k] = settings[k];
    paint();
  });
  return strip;
}

const BUILDERS = {
  buttons(content, node) {
    const cols = Math.max(1, node.cols || 1), rows = Math.max(1, node.rows || 1);
    const grid = el("div", "grid");
    grid.style.gridTemplateColumns = "repeat(" + cols + ", 1fr)";
    grid.style.gridTemplateRows = "repeat(" + rows + ", 1fr)";
    for (let c = 0; c < cols * rows; c++) {
      const cell = el("div", "cell");
      const item = node.items[String(c)];
      if (item) {
        const b = el("div", "btn" + (item.image ? " image" : ""));
        b.style.setProperty("--tint", item.color || "#3d8bfd");
        if (item.image)
          b.style.backgroundImage = "url('" + item.image + "')";
        if (item.icon && !item.image)
          b.appendChild(el("div", "glyph", item.icon));
        b.appendChild(el("div", "", item.label || ""));
        if (item.flash)
          b.appendChild(el("span", "flash", ""));
        if (item.functionId !== undefined)
          rhythmBar(b, item.functionId);
        const at = { page: pageIndex, section: node.id, cell: c };
        // pressed while a finger is on it: its release reaches the computer
        // wherever the finger is let go (see holdDown)
        const k = key(node, c), held = "show/" + k;
        b.addEventListener("pointerdown", (e) => {
          capture(b, e.pointerId);
          holdDown(held, e.pointerId, Object.assign({ cmd: "press" }, at), Object.assign({ cmd: "release" }, at));
        });
        holdPainters.push(() => b.classList.toggle("pressed", holds.has(held)));
        updaters.push((s) => b.classList.toggle("active", !!(s.items[k] && s.items[k].active)));
        cell.appendChild(b);
      }
      grid.appendChild(cell);
    }
    content.appendChild(grid);
  },

  faders(content, node) {
    const cols = Math.max(1, node.cols || 1);
    const grid = el("div", "grid");
    grid.style.gridTemplateColumns = "repeat(" + cols + ", 1fr)";
    for (let c = 0; c < cols; c++) {
      const cell = el("div", "cell");
      const item = node.items[String(c)];
      if (item) {
        const f = el("div", "fader");
        f.style.setProperty("--tint", item.color || "#3d8bfd");
        f.appendChild(el("div", "label", item.label || ""));
        const track = el("div", "track"), fill = el("div", "fill"), knob = el("div", "knob");
        track.appendChild(fill);
        track.appendChild(knob);
        f.appendChild(track);
        const value = el("div", "value", "");
        f.appendChild(value);
        const at = { page: pageIndex, section: node.id, cell: c };
        let level = 0, dragging = false;
        // the knob at the position, the text the value
        const show = (v, raw) => {
          fill.style.height = (v * 100) + "%";
          knob.style.bottom = (v * 100) + "%";
          setValueText(value, valueText(item, raw));
        };
        drag(track, (e) => {
          dragging = true;
          const r = track.getBoundingClientRect();
          level = Math.max(0, Math.min(1, 1 - (e.clientY - r.top) / r.height));
          show(level, rangeValue(item, level));
          sendSoon(key(node, c), Object.assign({ cmd: "level", value: level }, at));
        }, () => {
          dragging = false;
          // what the computer made of it (a scene stopped at the end), once
          // the last move has reached it: the state comes only when it changes
          setTimeout(() => {
            if (dragging || !state || !state.items || !state.items[k]) return;
            level = state.items[k].level;
            show(level, stateValue(item, state.items[k]));
          }, 400);
        });
        const k = key(node, c);
        if (item.flash) {
          // full while a finger is on it; the computer puts the level back,
          // also when the finger is let go somewhere else or the phone goes
          const flash = el("button", "", tr("flash")), held = "fader/" + k;
          flash.addEventListener("pointerdown", (e) => {
            capture(flash, e.pointerId);
            holdDown(held, e.pointerId, Object.assign({ cmd: "faderFlash", down: true }, at),
                     Object.assign({ cmd: "faderFlash", down: false }, at));
          });
          holdPainters.push(() => flash.classList.toggle("pressed", holds.has(held)));
          f.appendChild(flash);
        }
        updaters.push((s) => {
          if (dragging || !s.items[k]) return;
          level = s.items[k].level;
          show(level, stateValue(item, s.items[k]));
        });
        cell.appendChild(f);
      }
      grid.appendChild(cell);
    }
    content.appendChild(grid);
  },

  dials(content, node) {
    const cols = Math.max(1, node.cols || 1), rows = Math.max(1, node.rows || 1);
    const grid = el("div", "grid");
    grid.style.gridTemplateColumns = "repeat(" + cols + ", 1fr)";
    grid.style.gridTemplateRows = "repeat(" + rows + ", 1fr)";
    for (let c = 0; c < cols * rows; c++) {
      const cell = el("div", "cell");
      const item = node.items[String(c)];
      if (item) {
        const d = el("div", "dial");
        d.appendChild(el("div", "label", item.label || ""));
        const canvas = el("canvas");
        d.appendChild(canvas);
        const value = el("div", "value", "");
        d.appendChild(value);
        const at = { page: pageIndex, section: node.id, cell: c };
        const tint = item.color || "#3d8bfd";
        // level is the position (the arc), raw the value (the text)
        let level = 0, raw = 0, wheel = 0, dragging = false, lastY = 0, rest = 0;
        const draw = () => {
          const { ctx, w, h } = sizeCanvas(canvas);
          ctx.clearRect(0, 0, w, h);
          const r = Math.min(w, h) / 2 - 6, cx = w / 2, cy = h / 2;
          if (r < 4) return;
          const start = Math.PI * 0.75, span = Math.PI * 1.5;
          ctx.lineCap = "round";
          if (item.encoder) {
            ctx.fillStyle = "#0e1014";
            ctx.beginPath(); ctx.arc(cx, cy, r, 0, Math.PI * 2); ctx.fill();
            ctx.strokeStyle = "#3a4150"; ctx.lineWidth = 2;
            for (let i = 0; i < 24; i++) {
              const a = wheel + i * Math.PI / 12;
              ctx.beginPath();
              ctx.moveTo(cx + Math.cos(a) * r * 0.62, cy + Math.sin(a) * r * 0.62);
              ctx.lineTo(cx + Math.cos(a) * r * 0.86, cy + Math.sin(a) * r * 0.86);
              ctx.stroke();
            }
            ctx.strokeStyle = tint; ctx.lineWidth = 4;
            ctx.beginPath(); ctx.arc(cx, cy, r + 2, start, start + span * level); ctx.stroke();
          } else {
            ctx.lineWidth = 6;
            ctx.strokeStyle = "#0e1014";
            ctx.beginPath(); ctx.arc(cx, cy, r, start, start + span); ctx.stroke();
            ctx.strokeStyle = tint;
            ctx.beginPath(); ctx.arc(cx, cy, r, start, start + span * level); ctx.stroke();
            ctx.fillStyle = "#252a35";
            ctx.beginPath(); ctx.arc(cx, cy, r * 0.72, 0, Math.PI * 2); ctx.fill();
            const a = start + span * level;
            ctx.strokeStyle = "#e7e9ef"; ctx.lineWidth = 3;
            ctx.beginPath();
            ctx.moveTo(cx + Math.cos(a) * r * 0.25, cy + Math.sin(a) * r * 0.25);
            ctx.lineTo(cx + Math.cos(a) * r * 0.65, cy + Math.sin(a) * r * 0.65);
            ctx.stroke();
          }
          setValueText(value, valueText(item, raw));
        };
        // up and down with the finger: a dial moves its whole range in 200 px, a wheel a notch every 12 px
        drag(canvas, (e, first) => {
          if (first) { lastY = e.clientY; dragging = true; return; }
          const dy = lastY - e.clientY;
          lastY = e.clientY;
          if (item.encoder) {
            rest += dy;
            wheel -= dy / 40;
            const notches = Math.trunc(rest / 12);
            if (notches !== 0) {
              rest -= notches * 12;
              send(Object.assign({ cmd: "encoder", notches: notches }, at));
            }
          } else {
            level = Math.max(0, Math.min(1, level + dy / 200));
            raw = rangeValue(item, level);
            sendSoon(key(node, c), Object.assign({ cmd: "level", value: level }, at));
          }
          draw();
        }, () => {
          dragging = false;
          // (as the faders: what the computer made of the last move)
          setTimeout(() => {
            if (dragging || !state || !state.items || !state.items[k]) return;
            level = state.items[k].level;
            raw = stateValue(item, state.items[k]);
            draw();
          }, 400);
        });
        const k = key(node, c);
        updaters.push((s) => {
          if ((dragging && !item.encoder) || !s.items[k]) return;
          level = s.items[k].level;
          raw = stateValue(item, s.items[k]);
          draw();
        });
        cell.appendChild(d);
      }
      grid.appendChild(cell);
    }
    content.appendChild(grid);
    requestAnimationFrame(() => update());
  },

  // the playbacks (the playback bar): a strip per playback, of the page
  // the computer shows (a header changes it) or of a fixed page
  playbacks(content, node) {
    const first = Math.max(0, node.first || 0);
    const cols = Math.max(1, Math.min(node.cols || 8, 20 - first));
    const fixed = node.playbackPage !== undefined && node.playbackPage >= 0;
    // a press acts on the page the phone shows
    const pbPage = () => fixed ? node.playbackPage : (state ? state.playbackPage : 0);
    const box = el("div", "pbsection");
    if (!fixed) {
      const head = el("div", "pbhead");
      const prev = el("button", "icon", ""), name = el("div", "pbname", ""), next = el("button", "icon", "");
      prev.addEventListener("click", () => send({ cmd: "playback", action: "pagePrevious" }));
      next.addEventListener("click", () => send({ cmd: "playback", action: "pageNext" }));
      head.appendChild(prev);
      head.appendChild(name);
      head.appendChild(next);
      box.appendChild(head);
      updaters.push((s) => {
        const names = s.playbackPageNames || [];
        name.textContent = names[s.playbackPage] || (tr("page") + " " + (s.playbackPage + 1));
        prev.disabled = s.playbackPage <= 0;
        next.disabled = s.playbackPage >= (s.playbackPages || 1) - 1;
      });
    }
    const grid = el("div", "pbgrid");
    grid.style.gridTemplateColumns = "repeat(" + cols + ", minmax(78px, 1fr))";
    for (let c = 0; c < cols; c++)
      grid.appendChild(playbackStrip(node, first + c, pbPage));
    box.appendChild(grid);
    content.appendChild(box);
  },

  group(content, node) { BUILDERS.groups(content, node); },

  groups(content, node) {
    const box = el("div", "zones");
    (node.zones || []).forEach((z) => {
      const col = el("div", "zone");
      col.style.setProperty("--tint", z.color || "#3d8bfd");
      col.appendChild(el("div", "name", z.name));
      const list = el("div", "scenes");
      const buttons = [];
      z.scenes.forEach((sc) => {
        const b = el("button", "scene", sc.name);
        b.addEventListener("click", () => send({ cmd: "scene", zone: z.id, id: sc.id }));
        rhythmBar(b, sc.id);
        list.appendChild(b);
        buttons.push({ b: b, id: sc.id });
      });
      col.appendChild(list);
      const master = el("div", "master hbar"), fill = el("div", "fill"), label = el("span", "", tr("master"));
      master.appendChild(fill);
      master.appendChild(label);
      let dragging = false;
      drag(master, (e) => {
        dragging = true;
        const r = master.getBoundingClientRect();
        const v = Math.max(0, Math.min(1, (e.clientX - r.left) / r.width));
        fill.style.width = (v * 100) + "%";
        sendSoon("zone" + z.id, { cmd: "groupMaster", zone: z.id, value: v });
      }, () => { dragging = false; });
      col.appendChild(master);
      updaters.push((s) => {
        const zs = s.zones[String(z.id)];
        if (!zs) return;
        buttons.forEach((x) => x.b.classList.toggle("active", zs.active.indexOf(x.id) >= 0));
        if (!dragging) fill.style.width = (zs.master * 100) + "%";
      });
      box.appendChild(col);
    });
    if ((node.zones || []).length === 0)
      box.appendChild(el("div", "placeholder", tr("noGroup")));
    content.appendChild(box);
  },

  // the live mixer: a strip per mixer (all the fixtures, then each fixture
  // group), side by side as on the computer, sideways when they don't fit
  mixer(content, node) {
    const box = el("div", "mixers");
    (node.mixers || []).forEach((m) => box.appendChild(mixerStrip(m)));
    content.appendChild(box);
  },

  clock(content, node) {
    const box = el("div", "clock"), time = el("div", "time"), date = el("div", "date");
    box.appendChild(time);
    if (node.date !== false) box.appendChild(date);
    const tick = () => {
      const now = new Date();
      time.textContent = now.toLocaleTimeString(LANG, node.seconds === false ? { hour: "2-digit", minute: "2-digit" } : {});
      date.textContent = now.toLocaleDateString(LANG, { weekday: "long", day: "numeric", month: "long", year: "numeric" });
    };
    tick();
    const timer = setInterval(() => { if (!box.isConnected) clearInterval(timer); else tick(); }, 1000);
    content.appendChild(box);
  },

  // a stopwatch or a countdown: the one of the computer (started, paused
  // or reset here or there, it is the same timer)
  timer(content, node) {
    const countdown = node.mode === "countdown", total = (node.minutes || 5) * 60000;
    const box = el("div", "timer"), time = el("div", "time"), row = el("div", "row");
    const toggle = el("button"), reset = el("button");
    const at = { cmd: "timer", page: pageIndex, section: node.id };
    toggle.addEventListener("click", () => send(Object.assign({ action: "toggle" }, at)));
    reset.addEventListener("click", () => send(Object.assign({ action: "reset" }, at)));
    reset.appendChild(el("span", "icon", ""));
    reset.appendChild(document.createTextNode(" " + tr("reset")));
    row.appendChild(toggle);
    row.appendChild(reset);
    box.appendChild(time);
    box.appendChild(row);
    // Start or Pause, as it runs
    let running = null;
    const paint = (now) => {
      if (now === running) return;
      running = now;
      toggle.textContent = "";
      toggle.appendChild(el("span", "icon", running ? "" : ""));
      toggle.appendChild(document.createTextNode(" " + tr(running ? "pause" : "start")));
      toggle.classList.toggle("on", running);
    };
    const elapsed = () => {
      const t = state && state.timers ? state.timers[node.id] : null;
      if (!t) return 0;
      return t.running ? Math.max(0, serverNow() - t.origin) : t.elapsed;
    };
    // the time as on the computer: orange in the last minute of a
    // countdown, red at zero
    const tick = () => {
      const ms = elapsed(), shown = countdown ? Math.max(0, total - ms) : ms;
      time.textContent = timerText(shown, countdown);
      time.classList.toggle("over", countdown && shown <= 0);
      time.classList.toggle("soon", countdown && shown > 0 && shown < 60000);
    };
    paint(false);
    tick();
    updaters.push((s) => {
      paint(!!(s.timers && s.timers[node.id] && s.timers[node.id].running));
      tick();
    });
    const timer = setInterval(() => { if (!box.isConnected) clearInterval(timer); else tick(); }, 100);
    content.appendChild(box);
  },

  // the rhythm: GO (a press of a scene starts it and marks its rhythm) and
  // Stop (a press stops it), always one of the two, and BeatGO (each scene
  // button shows its own rhythm)
  bpm(content) {
    const box = el("div", "bpm"), modes = el("div", "row modes"), row = el("div", "row");
    const go = el("button", "go", "GO"), stop = el("button", "stop", "Stop"), beat = el("button", "", "BeatGO");
    go.addEventListener("pointerdown", () => send({ cmd: "global", action: "pressGo" }));
    stop.addEventListener("pointerdown", () => send({ cmd: "global", action: "pressStop" }));
    beat.addEventListener("pointerdown", () => send({ cmd: "global", action: "beatgo" }));
    modes.appendChild(go);
    modes.appendChild(stop);
    row.appendChild(beat);
    box.appendChild(modes);
    box.appendChild(row);
    updaters.push((s) => {
      go.classList.toggle("on", s.pressMode === "go");
      stop.classList.toggle("on", s.pressMode === "stop");
    });
    content.appendChild(box);
  },

  notes(content, node) {
    const n = el("div", "notes", node.text || "");
    n.style.fontSize = (node.fontSize || 20) + "px";
    content.appendChild(n);
  },

  running(content) {
    const box = el("div", "running");
    updaters.push((s) => {
      box.innerHTML = "";
      if (s.running.length === 0) box.appendChild(el("div", "placeholder", tr("nothing")));
      s.running.forEach((name) => box.appendChild(el("div", "", name)));
    });
    content.appendChild(box);
  },

  // the DMX levels of a universe, as on the computer: its 512 channels in
  // 32 × 16 cells of the accent colour, brighter with the value
  monitor(content, node) {
    const universe = Math.max(0, node.universe || 0);
    const box = el("div", "monitor"), canvas = el("canvas");
    box.appendChild(el("div", "universe", tr("universe").replace("%1", universe + 1)));
    box.appendChild(canvas);
    if (watched.indexOf(universe) < 0)
      watched.push(universe);
    const draw = () => {
      const { ctx, w, h } = sizeCanvas(canvas);
      ctx.clearRect(0, 0, w, h);
      const cw = (w - 31) / 32, ch = (h - 15) / 16;
      if (cw <= 0 || ch <= 0) return;
      const values = levels.get(universe);
      ctx.fillStyle = getComputedStyle(box).getPropertyValue("--accent").trim() || "#3d8bfd";
      for (let i = 0; i < 512; i++) {
        ctx.globalAlpha = 0.06 + 0.9 * (values ? values[i] || 0 : 0) / 255;
        ctx.fillRect((i % 32) * (cw + 1), Math.floor(i / 32) * (ch + 1), cw, ch);
      }
      ctx.globalAlpha = 1;
    };
    levelPainters.push({ universe: universe, paint: draw });
    content.appendChild(box);
    // (once it is laid out: the canvas has no size before)
    requestAnimationFrame(draw);
  },

  image(content, node) {
    if (!node.image) return;
    const img = el("img", "image" + (node.fit === "fill" ? " fill" : ""));
    img.src = node.image;
    content.appendChild(img);
  },

  colour(content, node) {
    const box = el("div", "pad"), canvas = el("canvas"), row = el("div", "row");
    box.appendChild(canvas);
    box.appendChild(row);
    const at = { page: pageIndex, section: node.id };
    let current = "";
    const draw = () => {
      const { ctx, w, h } = sizeCanvas(canvas);
      ctx.clearRect(0, 0, w, h);
      // centred in the canvas
      const r = Math.min(w, h) / 2 - 2, cx = w / 2, cy = h / 2;
      for (let i = 0; i < 360; i += 2) {
        ctx.fillStyle = hsvToHex(i / 360, 1, 1);
        ctx.beginPath();
        ctx.moveTo(cx, cy);
        ctx.arc(cx, cy, r, -(i + 2.5) * Math.PI / 180, -i * Math.PI / 180, false);
        ctx.closePath();
        ctx.fill();
      }
      const g = ctx.createRadialGradient(cx, cy, 0, cx, cy, r);
      g.addColorStop(0, "rgba(255,255,255,1)");
      g.addColorStop(1, "rgba(255,255,255,0)");
      ctx.fillStyle = g;
      ctx.beginPath(); ctx.arc(cx, cy, r, 0, Math.PI * 2); ctx.fill();
      if (current) {
        const hsv = hexToHsv(current);
        const x = cx + Math.cos(hsv.h * Math.PI * 2) * hsv.s * r, y = cy - Math.sin(hsv.h * Math.PI * 2) * hsv.s * r;
        ctx.strokeStyle = "white"; ctx.lineWidth = 3;
        ctx.beginPath(); ctx.arc(x, y, 8, 0, Math.PI * 2); ctx.stroke();
      }
    };
    drag(canvas, (e) => {
      const rect = canvas.getBoundingClientRect();
      const r = Math.min(rect.width, rect.height) / 2 - 2;
      const dx = e.clientX - rect.left - rect.width / 2, dy = rect.height / 2 - (e.clientY - rect.top);
      let hue = Math.atan2(dy, dx) / (Math.PI * 2);
      if (hue < 0) hue += 1;
      current = hsvToHex(hue, Math.min(1, Math.hypot(dx, dy) / r), 1);
      draw();
      sendSoon(key(node), Object.assign({ cmd: "colour", color: current }, at));
    });
    ["#ffffff", "#ff0000", "#ff7f00", "#ffff00", "#00ff00", "#00ffff", "#0000ff", "#ff00ff"].forEach((hex) => {
      const s = el("button", "swatch");
      s.style.background = hex;
      s.addEventListener("click", () => send(Object.assign({ cmd: "colour", color: hex }, at)));
      row.appendChild(s);
    });
    const release = el("button", "", tr("release"));
    release.addEventListener("click", () => send(Object.assign({ cmd: "releaseSection" }, at)));
    row.appendChild(release);
    const k = key(node);
    updaters.push((s) => { current = s.colours[k] || ""; draw(); });
    content.appendChild(box);
  },

  xy(content, node) {
    const box = el("div", "pad"), canvas = el("canvas"), row = el("div", "row");
    box.appendChild(canvas);
    box.appendChild(row);
    const at = { page: pageIndex, section: node.id };
    let pos = { x: 0.5, y: 0.5, available: false };
    const draw = () => {
      const { ctx, w, h } = sizeCanvas(canvas);
      ctx.clearRect(0, 0, w, h);
      // a square in the middle of the canvas
      const side = Math.min(w, h), ox = (w - side) / 2, oy = (h - side) / 2;
      ctx.translate(ox, oy);
      ctx.fillStyle = "#0e1014";
      ctx.fillRect(0, 0, side, side);
      ctx.strokeStyle = "#2b303c"; ctx.lineWidth = 1;
      for (let i = 1; i < 4; i++) {
        ctx.beginPath(); ctx.moveTo(side * i / 4, 0); ctx.lineTo(side * i / 4, side); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(0, side * i / 4); ctx.lineTo(side, side * i / 4); ctx.stroke();
      }
      ctx.fillStyle = pos.available ? "#ae3ec9" : "#5d6475";
      ctx.beginPath(); ctx.arc(pos.x * side, (1 - pos.y) * side, 10, 0, Math.PI * 2); ctx.fill();
      ctx.strokeStyle = "white"; ctx.lineWidth = 2; ctx.stroke();
    };
    drag(canvas, (e) => {
      const rect = canvas.getBoundingClientRect();
      const side = Math.min(rect.width, rect.height), ox = (rect.width - side) / 2, oy = (rect.height - side) / 2;
      pos = { x: Math.max(0, Math.min(1, (e.clientX - rect.left - ox) / side)),
              y: Math.max(0, Math.min(1, 1 - (e.clientY - rect.top - oy) / side)), available: true };
      draw();
      sendSoon(key(node), Object.assign({ cmd: "xy", x: pos.x, y: pos.y }, at));
    });
    const centre = el("button", "", tr("centre"));
    centre.addEventListener("click", () => send(Object.assign({ cmd: "xy", x: 0.5, y: 0.5 }, at)));
    const release = el("button", "", tr("release"));
    release.addEventListener("click", () => send(Object.assign({ cmd: "releaseSection" }, at)));
    row.appendChild(centre);
    row.appendChild(release);
    const k = key(node);
    updaters.push((s) => { if (s.xy[k]) pos = s.xy[k]; draw(); });
    content.appendChild(box);
  }
};

function update() {
  if (!state) return;
  $("blackout").classList.toggle("on", !!state.blackout);
  updaters.forEach((u) => u(state));
  // (a page that follows the computer: its flashes are another playback's now)
  paintHolds();
}

window.addEventListener("resize", () => { update(); paintLevels(); });
$("offlineText").textContent = tr("connecting");
connect();
