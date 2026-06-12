// app.js — NTC Creep Simulator v1.4

// ── Password gate ─────────────────────────────────────────────────────────────
const PASSWORD_HASH = "061406b92feb02f5f0843b64f75e214a29e98341d8309718a7f7e68420e65ea1"; // geotech13

async function sha256(msg) {
  const buf = await crypto.subtle.digest("SHA-256", new TextEncoder().encode(msg));
  return Array.from(new Uint8Array(buf)).map(b => b.toString(16).padStart(2,"0")).join("");
}

async function checkPassword() {
  const input = document.getElementById("pw-input").value;
  const hash  = await sha256(input);
  if (hash === PASSWORD_HASH) {
    sessionStorage.setItem("ntc_auth", "1");
    document.getElementById("gate").classList.add("hidden");
  } else {
    document.getElementById("gate-error").textContent = "Incorrect password.";
    document.getElementById("pw-input").value = "";
    document.getElementById("pw-input").focus();
  }
}

document.getElementById("pw-input").addEventListener("keydown", e => { if (e.key === "Enter") checkPassword(); });
document.getElementById("pw-btn").addEventListener("click", checkPassword);
if (sessionStorage.getItem("ntc_auth") === "1") document.getElementById("gate").classList.add("hidden");

// ── Theme (dark default) ──────────────────────────────────────────────────────
const themeToggle = document.getElementById("theme-toggle");
function applyTheme(dark) {
  document.body.classList.toggle("dark", dark);
  document.body.classList.toggle("light", !dark);
  themeToggle.textContent = dark ? "☀ Light" : "☾ Dark";
}
// Dark unless user explicitly chose light
applyTheme(localStorage.getItem("ntc_theme") !== "light");
themeToggle.addEventListener("click", () => {
  const dark = !document.body.classList.contains("dark");
  localStorage.setItem("ntc_theme", dark ? "dark" : "light");
  applyTheme(dark);
  if (parsedData) renderChart();
});

// ── Modals ────────────────────────────────────────────────────────────────────
function openModal(id)  { document.getElementById(id).classList.add("open"); }
function closeModal(id) { document.getElementById(id).classList.remove("open"); }
function closeModalOutside(e, overlay) { if (e.target === overlay) overlay.classList.remove("open"); }
document.addEventListener("keydown", e => {
  if (e.key === "Escape") document.querySelectorAll(".modal-overlay.open").forEach(m => m.classList.remove("open"));
});

// ── Collapsible sections ──────────────────────────────────────────────────────
document.querySelectorAll(".section-toggle").forEach(btn => {
  btn.addEventListener("click", () => {
    btn.classList.toggle("open");
    btn.nextElementSibling.classList.toggle("open");
  });
});

// ── Parameter definitions ─────────────────────────────────────────────────────
// Order MUST match SetParam::setParam() reading order exactly (56 tokens).
// dispSec controls which HTML section each field renders into.
const PARAMS = [
  // 1–8: Initial conditions
  { id:"Initime",    label:"time",               dispSec:"init",       default:0 },
  { id:"Iniep",      label:"ep",                 dispSec:"init",       default:0 },
  { id:"Iniepd",     label:"epd",                dispSec:"init",       default:0 },
  { id:"Iniep_ir",   label:"ep_ir",              dispSec:"init",       default:0 },
  { id:"Iniepd_ir",  label:"epd_ir",             dispSec:"init",       default:0 },
  { id:"IniSigma",   label:"Sigma",              dispSec:"init",       default:0 },
  { id:"IniSigma_f", label:"Sigma_r",            dispSec:"init",       default:0 },
  { id:"Initau",     label:"tau",                dispSec:"init",       default:0 },
  // 9–11: Hypo-Elastic
  { id:"b",          label:"b",                  dispSec:"elastic",    default:0.5 },
  { id:"E_ini",      label:"E_ini",              dispSec:"elastic",    default:500 },
  { id:"Sigma_0",    label:"Sigma_0",            dispSec:"elastic",    default:1 },
  // 12–15: Decay function
  { id:"r1",         label:"r1",                 dispSec:"decay",      default:0.0001 },
  { id:"r2",         label:"r2",                 dispSec:"decay",      default:0.0001 },
  { id:"ep_ir_0",    label:"ep_ir_0",            dispSec:"decay",      default:0 },
  { id:"c",          label:"c",                  dispSec:"decay",      default:0.3 },
  // 16–23: Viscosity function
  { id:"alpha",      label:"Alpha",              dispSec:"viscous",    default:0.36 },
  { id:"m",          label:"m",                  dispSec:"viscous",    default:0.03 },
  { id:"epd_ir_0",   label:"epd_ir_0",           dispSec:"viscous",    default:1e-7 },
  { id:"alpha2",     label:"Alpha*",             dispSec:"viscous",    default:0.4 },
  { id:"m2",         label:"1+b",                dispSec:"viscous",    default:0.55 },
  { id:"epd_ir_2",   label:"epd_ir_0 (2nd)",     dispSec:"viscous",    default:0.00012 },
  { id:"epd_ir_int", label:"epd_ir_int",         dispSec:"viscous",    default:1.99245e-7 },
  { id:"gvtype",     label:"Type  0:gv1 1:gv2 2:mix", dispSec:"viscous", default:2 },
  // 24–25: Direction
  { id:"DirectionX", label:"Dx",                 dispSec:"direction",  default:1 },
  { id:"DirectionY", label:"Dy",                 dispSec:"direction",  default:1 },
  // 26–27: Initial viscous history
  { id:"IniDSigV",   label:"Last DSigma_V",      dispSec:"history",    default:0 },
  { id:"Initau_h",   label:"Last_tau",           dispSec:"history",    default:0 },
  // 28–30: Iteration / solver
  { id:"MaxIter",    label:"Max Iteration",      dispSec:"solver",     default:2000 },
  { id:"Precision",  label:"Precision (−log₁₀)", dispSec:"solver",     default:7 },
  { id:"Integration",label:"Integration",        dispSec:"solver",     default:0 },
  // 31: Lambda
  { id:"lambda_V",   label:"Lambda_V",           dispSec:"lambda",     default:1 },
  // 32–33: Proportion
  { id:"PropX",      label:"Px",                 dispSec:"proportion", default:1 },
  { id:"PropY",      label:"Py",                 dispSec:"proportion", default:1 },
  // 34–43: Reference curve parameters
  { id:"RefP0",  label:"0",  dispSec:"ref", default:0 },
  { id:"RefP1",  label:"1",  dispSec:"ref", default:4.44277 },
  { id:"RefP2",  label:"2",  dispSec:"ref", default:-0.07648 },
  { id:"RefP3",  label:"3",  dispSec:"ref", default:1.53056 },
  { id:"RefP4",  label:"4",  dispSec:"ref", default:0.68195 },
  { id:"RefP5",  label:"5",  dispSec:"ref", default:-2.35359 },
  { id:"RefP6",  label:"6",  dispSec:"ref", default:0 },
  { id:"RefP7",  label:"7",  dispSec:"ref", default:0 },
  { id:"RefP8",  label:"8",  dispSec:"ref", default:0 },
  { id:"RefP9",  label:"9",  dispSec:"ref", default:0 },
  // 44: Reference function type
  { id:"RefFunc", label:"Type  0:Poly 1:Exp 2:EPS 3–4:DST", dispSec:"ref", default:2 },
  // 45–48: Theta
  { id:"th1",       label:"theta1",  dispSec:"theta", default:-1.42 },
  { id:"th2",       label:"theta2",  dispSec:"theta", default:-0.75 },
  { id:"rf_strain", label:"ep_ir_0", dispSec:"theta", default:7.66 },
  { id:"curve",     label:"c",       dispSec:"theta", default:1 },
  // 49–56: Ageing
  { id:"a_f",       label:"a_f",       dispSec:"aging", default:1 },
  { id:"b_f",       label:"b_f",       dispSec:"aging", default:0 },
  { id:"lambda_f",  label:"lambda_f",  dispSec:"aging", default:1 },
  { id:"r1_f",      label:"r1_f",      dispSec:"aging", default:1 },
  { id:"r2_f",      label:"r2_f",      dispSec:"aging", default:1 },
  { id:"ep_ir_0_f", label:"ep_ir_0_f", dispSec:"aging", default:0 },
  { id:"c_f",       label:"c_f",       dispSec:"aging", default:1 },
  { id:"A_f",       label:"A_f",       dispSec:"aging", default:0.957 },
];

// ── Build parameter form ──────────────────────────────────────────────────────
const SECTION_BODIES = {
  init:       "sec-init-body",
  elastic:    "sec-elastic-body",
  viscous:    "sec-viscous-body",
  decay:      "sec-decay-body",
  ref:        "sec-ref-body",
  theta:      "sec-theta-body",
  aging:      "sec-aging-body",
  history:    "sec-history-body",
  direction:  "sec-direction-body",
  proportion: "sec-proportion-body",
  lambda:     "sec-lambda-body",
  solver:     "sec-solver-body",
};

function buildParamEditor() {
  const grids = {};
  PARAMS.forEach(p => {
    const bodyId = SECTION_BODIES[p.dispSec];
    if (!bodyId) return;
    if (!grids[p.dispSec]) {
      const el = document.getElementById(bodyId);
      if (!el) return;
      grids[p.dispSec] = document.createElement("div");
      grids[p.dispSec].className = "param-grid";
      el.appendChild(grids[p.dispSec]);
    }
    const div = document.createElement("div");
    div.className = "field";
    div.innerHTML = `<label for="p-${p.id}">${p.label}</label>
      <input type="text" id="p-${p.id}" value="${p.default}" autocomplete="off">`;
    grids[p.dispSec].appendChild(div);
  });
}
buildParamEditor();

// ── Assemble .prm text (serialization order = PARAMS array order) ─────────────
function assemblePrm() {
  return PARAMS.map(p => {
    const el = document.getElementById(`p-${p.id}`);
    return (el ? el.value.trim() : String(p.default));
  }).join("\n") + "\n";
}

// ── Load .prm text into form ──────────────────────────────────────────────────
function loadPrmText(text) {
  const tokens = text.trim().split(/\s+/).filter(t => t.length > 0);
  PARAMS.forEach((p, i) => {
    const el = document.getElementById(`p-${p.id}`);
    if (el && tokens[i] !== undefined) el.value = tokens[i];
  });
}

// ── Control sequence ──────────────────────────────────────────────────────────
const EVENT_TYPES = [
  { value:1, label:"1 – Strain Rate" },
  { value:2, label:"2 – Relaxation" },
  { value:3, label:"3 – Strain Accel." },
  { value:4, label:"4 – Stress Rate" },
  { value:5, label:"5 – Creep" },
  { value:6, label:"6 – StressStrain" },
];

// ── Control table mode (.sctr = 4-col, .ctr = 5-col) ─────────────────────────
let ctrlMode = "sctr"; // "sctr" | "ctr"

function setCtrlMode(mode) {
  ctrlMode = mode;
  // Update toggle button styles
  document.getElementById("btn-mode-sctr").classList.toggle("active", mode === "sctr");
  document.getElementById("btn-mode-ctr").classList.toggle("active", mode === "ctr");
  // Show/hide the dt column header
  document.querySelectorAll(".col-dt").forEach(el => {
    el.style.display = mode === "ctr" ? "" : "none";
  });
  // Rebuild existing rows to show/hide dt cell
  const tbody = document.getElementById("ctrl-tbody");
  const trs = [...tbody.querySelectorAll("tr")];
  trs.forEach(tr => {
    const dtCell = tr.querySelector(".cell-dt");
    if (dtCell) dtCell.style.display = mode === "ctr" ? "" : "none";
  });
}

document.getElementById("btn-mode-sctr").addEventListener("click", () => setCtrlMode("sctr"));
document.getElementById("btn-mode-ctr").addEventListener("click",  () => setCtrlMode("ctr"));

function addCtrlRow(tbody, type=1, tStart=0, tEnd=0, dt=1, val=0) {
  const tr = document.createElement("tr");
  const opts = EVENT_TYPES.map(e =>
    `<option value="${e.value}"${e.value==type?" selected":""}>${e.label}</option>`).join("");
  const dtDisplay = ctrlMode === "ctr" ? "" : "display:none";
  tr.innerHTML = `
    <td><select class="ctrl-row-type">${opts}</select></td>
    <td><input class="ctrl-row-num" type="text" value="${tStart}" placeholder="start"></td>
    <td><input class="ctrl-row-num" type="text" value="${tEnd}" placeholder="end"></td>
    <td class="cell-dt col-dt" style="${dtDisplay}"><input class="ctrl-row-num" type="text" value="${dt}" placeholder="dt"></td>
    <td><input class="ctrl-row-num" type="text" value="${val}" placeholder="value"></td>
    <td><button class="btn-icon" onclick="this.closest('tr').remove()">✕</button></td>`;
  tbody.appendChild(tr);
}

function initCtrlTable(rows) {
  // rows: [type, start, end, (dt), val] — dt optional (index 3 if 5-col, val at [3] if 4-col)
  const tbody = document.getElementById("ctrl-tbody");
  tbody.innerHTML = "";
  rows.forEach(r => {
    if (r.length >= 5) addCtrlRow(tbody, r[0], r[1], r[2], r[3], r[4]);
    else               addCtrlRow(tbody, r[0], r[1], r[2], 1,    r[3]);
  });
}

document.getElementById("btn-add-row").addEventListener("click", () => {
  const trs = document.getElementById("ctrl-tbody").querySelectorAll("tr");
  let lastEnd = 0;
  if (trs.length) {
    const inps = trs[trs.length-1].querySelectorAll("input");
    lastEnd = parseFloat(inps[1].value) || 0;
  }
  addCtrlRow(document.getElementById("ctrl-tbody"), 1, lastEnd, lastEnd, 1, 0);
});

document.getElementById("btn-clear-ctrl").addEventListener("click", () => {
  if (confirm("Clear all loading sequence rows?"))
    document.getElementById("ctrl-tbody").innerHTML = "";
});

// Assemble control file text in the active format
function assembleCtrl() {
  let lines = "";
  document.getElementById("ctrl-tbody").querySelectorAll("tr").forEach(tr => {
    const sel  = tr.querySelector("select").value;
    const inps = tr.querySelectorAll("input");
    if (ctrlMode === "ctr") {
      // 5-col: type  startTime  endTime  dt  value
      lines += `${sel}\t${inps[0].value}\t${inps[1].value}\t${inps[2].value}\t${inps[3].value}\n`;
    } else {
      // 4-col: type  startTime  endTime  value  (inps[2] is hidden dt, skip it)
      lines += `${sel}\t${inps[0].value}\t${inps[1].value}\t${inps[3].value}\n`;
    }
  });
  return lines + "0\n";
}

function loadCtrlText(text) {
  const rows = [];
  text.trim().split("\n").forEach(line => {
    const p = line.trim().split(/\s+/);
    const t = parseInt(p[0]);
    if (isNaN(t) || t === 0) return;
    if (p.length >= 5) {
      // 5-col .ctr
      rows.push([t, parseFloat(p[1]||0), parseFloat(p[2]||0), parseFloat(p[3]||1), parseFloat(p[4]||0)]);
      if (ctrlMode !== "ctr") setCtrlMode("ctr"); // auto-switch mode
    } else {
      // 4-col .sctr
      rows.push([t, parseFloat(p[1]||0), parseFloat(p[2]||0), parseFloat(p[3]||0)]);
      if (ctrlMode !== "sctr") setCtrlMode("sctr");
    }
  });
  if (rows.length) initCtrlTable(rows);
}

// ── Example data (SL1T60) ─────────────────────────────────────────────────────
const EXAMPLE_CTRL = [
  [1,0,191,0.0008206],[5,191,11627,0],[1,11627,11765,0.000754],
  [5,11765,23114,0],[1,23114,23255,0.00118],[5,23255,34588,0],[1,34588,46138,0.00125],
];

async function loadExample() {
  try {
    const res = await fetch("example/SL1T60.prm");
    if (res.ok) loadPrmText(await res.text());
  } catch (_) {}
  initCtrlTable(EXAMPLE_CTRL);
}
loadExample();

// ── File uploads ──────────────────────────────────────────────────────────────
document.getElementById("upload-prm").addEventListener("change", e => {
  const f = e.target.files[0]; if (!f) return;
  const r = new FileReader(); r.onload = ev => loadPrmText(ev.target.result); r.readAsText(f);
  e.target.value = "";
});
document.getElementById("upload-sctr").addEventListener("change", e => {
  const f = e.target.files[0]; if (!f) return;
  const r = new FileReader(); r.onload = ev => loadCtrlText(ev.target.result); r.readAsText(f);
  e.target.value = "";
});

// ── Project name & timestamp ──────────────────────────────────────────────────
function makeTimestamp() {
  const d = new Date();
  const pad = n => String(n).padStart(2,"0");
  return `ntc_${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())}_${pad(d.getHours())}${pad(d.getMinutes())}${pad(d.getSeconds())}`;
}

function getProjectName() {
  const el = document.getElementById("project-name");
  return (el && el.value.trim()) ? el.value.trim() : makeTimestamp();
}

function setProjectTimestamp() {
  const el = document.getElementById("project-name");
  if (el) el.value = makeTimestamp();
}

// Set initial timestamp on load
setProjectTimestamp();

// ── Export buttons ────────────────────────────────────────────────────────────
function downloadText(content, filename) {
  const a = document.createElement("a");
  a.href = URL.createObjectURL(new Blob([content], {type:"text/plain"}));
  a.download = filename; a.click(); URL.revokeObjectURL(a.href);
}
document.getElementById("btn-export-prm").addEventListener("click",
  () => downloadText(assemblePrm(), getProjectName() + ".prm"));
document.getElementById("btn-export-sctr").addEventListener("click",
  () => downloadText(assembleCtrl(), getProjectName() + ".sctr"));

// ── Status & progress bar ─────────────────────────────────────────────────────
function setStatus(msg, type="") {
  const el = document.getElementById("status-msg");
  el.textContent = msg; el.className = type;
}

function showProgress(state, pct=0) {
  const wrap = document.getElementById("progress-wrap");
  const bar  = document.getElementById("progress-bar");
  const txt  = document.getElementById("progress-pct");
  if (state === "hidden") { wrap.style.display="none"; bar.className="progress-bar"; return; }
  wrap.style.display = "flex";
  bar.className = "progress-bar";
  if (state === "running") {
    bar.style.width = pct + "%";
    txt.textContent = pct + "%";
  } else if (state === "done") {
    bar.classList.add("complete"); bar.style.width="100%"; txt.textContent="100%";
  } else if (state === "error") {
    bar.classList.add("error-bar"); bar.style.width="100%"; txt.textContent="Error";
  }
}

// ── Web Worker ────────────────────────────────────────────────────────────────
let simWorker = null;
let lastOutputData = null;

function initWorker() {
  simWorker = new Worker("worker.js");
  simWorker.onmessage = function(e) {
    const { type, value, data, msg } = e.data;
    if (type === "ready") {
      setStatus("Ready — click Run to simulate.", "");
      document.getElementById("btn-run").disabled = false;
    } else if (type === "progress") {
      showProgress("running", value);
      if (value < 100) setStatus(`Running… ${value}%`, "");
    } else if (type === "result") {
      document.getElementById("btn-run").disabled = false;
      lastOutputData = data;
      const n = data.trim().split("\n").length - 1;
      setStatus(`Done — ${n} data points.`, "success");
      showProgress("done");
      setTimeout(() => showProgress("hidden"), 1800);
      document.getElementById("btn-download").disabled = false;
      setProjectTimestamp();   // refresh timestamp for next download
      plotResult(data);
    } else if (type === "error") {
      document.getElementById("btn-run").disabled = false;
      setStatus(msg, "error");
      showProgress("error");
      setTimeout(() => showProgress("hidden"), 2500);
    }
  };
  simWorker.onerror = function(e) {
    setStatus("Worker error: " + e.message, "error");
    document.getElementById("btn-run").disabled = false;
  };
}

window.addEventListener("load", () => { setTimeout(initWorker, 100); });

// ── Run button ────────────────────────────────────────────────────────────────
document.getElementById("btn-run").addEventListener("click", () => {
  if (!simWorker) { setStatus("Worker not ready.", "error"); return; }
  document.getElementById("btn-run").disabled = true;
  setStatus("Starting…", "");
  showProgress("running", 0);
  simWorker.postMessage({
    type: "run",
    paramContent: assemblePrm(),
    ctrlContent:  assembleCtrl(),
  });
});

// ── Parse output data ─────────────────────────────────────────────────────────
let parsedData = null;

function parseOutput(text) {
  const lines = text.trim().split("\n");
  if (lines.length < 2) return { headers:[], columns:{} };
  const headers = lines[0].split("\t").map(h => h.trim());
  const columns = {};
  headers.forEach(h => columns[h] = []);
  for (let i = 1; i < lines.length; i++) {
    const vals = lines[i].split("\t");
    headers.forEach((h, j) => { const v = parseFloat(vals[j]); columns[h].push(isNaN(v)?0:v); });
  }
  return { headers, columns };
}

// ── Axis selector population ──────────────────────────────────────────────────
function populateAxisSelectors(headers) {
  ["axis-x","axis-y"].forEach(id => {
    const sel = document.getElementById(id);
    const prev = sel.value;
    sel.innerHTML = headers.map(h => `<option value="${h}">${h}</option>`).join("");
    if (headers.includes(prev)) sel.value = prev;
  });
  const xSel = document.getElementById("axis-x");
  const ySel = document.getElementById("axis-y");
  if (headers.includes("ep_ir")) xSel.value = "ep_ir";
  if (headers.includes("Sigma")) ySel.value = "Sigma";
}

// ── Plot (two-line default chart) ─────────────────────────────────────────────
// Sigma and Sigma_f are offset +1 for display (R starts at 1, not 0)
const STRESS_COLS = new Set(["Sigma","Sigma_f","Sigma_fy"]);

function applyOffset(key, arr) {
  return STRESS_COLS.has(key) ? arr.map(v => v + 1) : arr;
}

function plotResult(text) {
  parsedData = parseOutput(text);
  if (!parsedData.headers.length) return;
  populateAxisSelectors(parsedData.headers);
  renderChart();
  document.getElementById("output-controls").style.display = "flex";
}

function renderChart() {
  if (!parsedData) return;
  const xKey = document.getElementById("axis-x").value;
  const yKey = document.getElementById("axis-y").value;
  const xData = parsedData.columns[xKey] || [];

  const isDark     = document.body.classList.contains("dark");
  const textColor  = isDark ? "#e8eaf0" : "#1a1a2e";
  const gridColor  = isDark ? "#2d3348" : "#e8ecf0";
  const paperColor = isDark ? "#1e2130" : "#f5f7fa";
  const plotBg     = isDark ? "#1e2130" : "#ffffff";

  const traces = [];

  // Primary trace
  traces.push({
    x: xData,
    y: applyOffset(yKey, parsedData.columns[yKey] || []),
    mode: "lines",
    line: { color:"#3b82f6", width:1.8 },
    name: STRESS_COLS.has(yKey) ? yKey + "+1" : yKey,
  });

  // Second trace: Sigma_f dashed, only when primary Y is Sigma
  if (yKey === "Sigma" && parsedData.columns["Sigma_f"]) {
    traces.push({
      x: xData,
      y: applyOffset("Sigma_f", parsedData.columns["Sigma_f"]),
      mode: "lines",
      line: { color:"#f97316", width:1.5, dash:"dash" },
      name: "Sigma_f+1",
    });
  }

  const layout = {
    xaxis: { title:xKey, color:textColor, gridcolor:gridColor, zerolinecolor:gridColor },
    yaxis: {
      title: STRESS_COLS.has(yKey) ? yKey+" (+1 offset)" : yKey,
      color:textColor, gridcolor:gridColor, zerolinecolor:gridColor
    },
    paper_bgcolor: paperColor,
    plot_bgcolor:  plotBg,
    font: { color:textColor, size:12 },
    margin: { l:60, r:20, t:20, b:50 },
    legend: { orientation:"h", y:-0.18, font:{size:11} },
    showlegend: traces.length > 1,
  };

  Plotly.newPlot("chart-container", traces, layout,
    { responsive:true, displayModeBar:true, displaylogo:false });
  document.getElementById("chart-placeholder").style.display = "none";
}

document.getElementById("axis-x").addEventListener("change", renderChart);
document.getElementById("axis-y").addEventListener("change", renderChart);

// ── Download .dat ─────────────────────────────────────────────────────────────
document.getElementById("btn-download").addEventListener("click", () => {
  if (lastOutputData) downloadText(lastOutputData, getProjectName() + ".dat");
});
