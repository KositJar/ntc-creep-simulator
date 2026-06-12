// app.js — NTC Creep Simulator web UI logic

// ── Password gate ────────────────────────────────────────────────────────────
// SHA-256 hash of the lab password. Change this to update the password.
// To generate: run  sha256("yourpassword")  in browser console after page loads.
// Default password: ntclab2024
const PASSWORD_HASH = "061406b92feb02f5f0843b64f75e214a29e98341d8309718a7f7e68420e65ea1";

async function sha256(msg) {
  const buf = await crypto.subtle.digest("SHA-256",
    new TextEncoder().encode(msg));
  return Array.from(new Uint8Array(buf))
    .map(b => b.toString(16).padStart(2, "0")).join("");
}

async function checkPassword() {
  const input = document.getElementById("pw-input").value;
  const hash = await sha256(input);
  if (hash === PASSWORD_HASH) {
    sessionStorage.setItem("ntc_auth", "1");
    document.getElementById("gate").classList.add("hidden");
  } else {
    document.getElementById("gate-error").textContent = "Incorrect password.";
    document.getElementById("pw-input").value = "";
    document.getElementById("pw-input").focus();
  }
}

document.getElementById("pw-input").addEventListener("keydown", e => {
  if (e.key === "Enter") checkPassword();
});
document.getElementById("pw-btn").addEventListener("click", checkPassword);

if (sessionStorage.getItem("ntc_auth") === "1") {
  document.getElementById("gate").classList.add("hidden");
}

// ── Dark mode ────────────────────────────────────────────────────────────────
const themeToggle = document.getElementById("theme-toggle");
function applyTheme(dark) {
  document.body.classList.toggle("dark", dark);
  themeToggle.textContent = dark ? "☀ Light" : "☾ Dark";
}
applyTheme(localStorage.getItem("ntc_theme") === "dark");
themeToggle.addEventListener("click", () => {
  const dark = !document.body.classList.contains("dark");
  localStorage.setItem("ntc_theme", dark ? "dark" : "light");
  applyTheme(dark);
});

// ── Collapsible sections ─────────────────────────────────────────────────────
document.querySelectorAll(".section-toggle").forEach(btn => {
  btn.addEventListener("click", () => {
    btn.classList.toggle("open");
    btn.nextElementSibling.classList.toggle("open");
  });
});

// ── Parameter definitions ────────────────────────────────────────────────────
// Order matches SetParam::setParam() exactly (56 tokens in .prm file)
const PARAMS = [
  // Section: Initial Conditions
  { id: "Initime",    label: "time₀ (s)",          section: "init",    default: 0 },
  { id: "Iniep",      label: "ε₀",                  section: "init",    default: 0 },
  { id: "Iniepd",     label: "ε̇₀ (strain rate)",   section: "init",    default: 0 },
  { id: "Iniep_ir",   label: "ε_ir,₀",              section: "init",    default: 0 },
  { id: "Iniepd_ir",  label: "ε̇_ir,₀",             section: "init",    default: 0 },
  { id: "IniSigma",   label: "R₀ (stress ratio)",   section: "init",    default: 0 },
  { id: "IniSigma_f", label: "R_f,₀",               section: "init",    default: 0 },
  { id: "Initau",     label: "τ₀ (damage)",          section: "init",    default: 0 },
  // Section: Elastic
  { id: "b",          label: "b",                    section: "elastic", default: 0.5 },
  { id: "E_ini",      label: "E_ini",                section: "elastic", default: 500 },
  { id: "Sigma_0",    label: "Σ₀ (ref. stress)",     section: "elastic", default: 1 },
  // Section: Viscous
  { id: "r1",         label: "r1 (decay)",           section: "viscous", default: 0.0001 },
  { id: "r2",         label: "r2 (decay)",           section: "viscous", default: 0.0001 },
  { id: "ep_ir_0",    label: "ε_ir,₀ (decay)",       section: "viscous", default: 0 },
  { id: "c",          label: "c (decay)",             section: "viscous", default: 0.3 },
  { id: "alpha",      label: "α (viscosity)",         section: "viscous", default: 0.36 },
  { id: "m",          label: "m (viscosity)",         section: "viscous", default: 0.03 },
  { id: "epd_ir_0",   label: "ε̇_ir,₀",              section: "viscous", default: 1e-7 },
  { id: "alpha2",     label: "α₂",                   section: "viscous", default: 0.4 },
  { id: "m2",         label: "m₂",                   section: "viscous", default: 0.55 },
  { id: "epd_ir_2",   label: "ε̇_ir,₂",             section: "viscous", default: 0.00012 },
  { id: "epd_ir_int", label: "ε̇_ir,int",            section: "viscous", default: 1.99245e-7 },
  { id: "gvtype",     label: "gvtype (1 or 2)",       section: "viscous", default: 2 },
  // Section: Solver
  { id: "DirectionX", label: "DirectionX (±1)",       section: "solver",  default: 1 },
  { id: "DirectionY", label: "DirectionY (±1)",       section: "solver",  default: 1 },
  { id: "IniDSigV",   label: "IniΔΣ_V (history)",    section: "solver",  default: 0 },
  { id: "Initau_h",   label: "Initau (history)",      section: "solver",  default: 0 },
  { id: "MaxIter",    label: "MaxIteration",          section: "solver",  default: 2000 },
  { id: "Precision",  label: "Precision (–log₁₀)",   section: "solver",  default: 7 },
  { id: "Integration",label: "Integration",           section: "solver",  default: 0 },
  { id: "lambda_V",   label: "λ_V",                   section: "solver",  default: 1 },
  { id: "PropX",      label: "ProportionX",           section: "solver",  default: 1 },
  { id: "PropY",      label: "ProportionY",           section: "solver",  default: 1 },
  // Section: Reference Curve
  { id: "RefP0",   label: "RefParam[0]",  section: "ref", default: 0 },
  { id: "RefP1",   label: "RefParam[1]",  section: "ref", default: 4.44277 },
  { id: "RefP2",   label: "RefParam[2]",  section: "ref", default: -0.07648 },
  { id: "RefP3",   label: "RefParam[3]",  section: "ref", default: 1.53056 },
  { id: "RefP4",   label: "RefParam[4]",  section: "ref", default: 0.68195 },
  { id: "RefP5",   label: "RefParam[5]",  section: "ref", default: -2.35359 },
  { id: "RefP6",   label: "RefParam[6]",  section: "ref", default: 0 },
  { id: "RefP7",   label: "RefParam[7]",  section: "ref", default: 0 },
  { id: "RefP8",   label: "RefParam[8]",  section: "ref", default: 0 },
  { id: "RefP9",   label: "RefParam[9]",  section: "ref", default: 0 },
  { id: "RefFunc", label: "RefFunction",  section: "ref", default: 2 },
  { id: "th1",     label: "θ₁",           section: "ref", default: -1.42 },
  { id: "th2",     label: "θ₂",           section: "ref", default: -0.75 },
  { id: "rf_strain", label: "rf_strain",  section: "ref", default: 7.66 },
  { id: "curve",   label: "curve",        section: "ref", default: 1 },
  // Section: Aging
  { id: "a_f",       label: "a_f",        section: "aging", default: 1 },
  { id: "b_f",       label: "b_f",        section: "aging", default: 0 },
  { id: "lambda_f",  label: "λ_f",        section: "aging", default: 0 },
  { id: "r1_f",      label: "r1_f",       section: "aging", default: 8.5 },
  { id: "r2_f",      label: "r2_f",       section: "aging", default: 1 },
  { id: "ep_ir_0_f", label: "ε_ir,₀_f",  section: "aging", default: 0 },
  { id: "c_f",       label: "c_f",        section: "aging", default: 0 },
  { id: "A_f",       label: "A_f (temp/aging factor)", section: "aging", default: 0.957 },
];

// Build parameter input fields into DOM
function buildParamEditor() {
  const sections = {
    init:    { title: "Initial Conditions",  el: document.getElementById("sec-init-body") },
    elastic: { title: "Elastic Component",   el: document.getElementById("sec-elastic-body") },
    viscous: { title: "Viscous Component",   el: document.getElementById("sec-viscous-body") },
    solver:  { title: "Solver & Control",    el: document.getElementById("sec-solver-body") },
    ref:     { title: "Reference Curve",     el: document.getElementById("sec-ref-body") },
    aging:   { title: "Aging",               el: document.getElementById("sec-aging-body") },
  };
  PARAMS.forEach(p => {
    const sec = sections[p.section];
    if (!sec || !sec.el) return;
    let grid = sec.el.querySelector(".param-grid");
    if (!grid) { grid = document.createElement("div"); grid.className = "param-grid"; sec.el.appendChild(grid); }
    const div = document.createElement("div"); div.className = "field";
    div.innerHTML = `<label for="p-${p.id}">${p.label}</label>
      <input type="text" id="p-${p.id}" value="${p.default}" autocomplete="off">`;
    grid.appendChild(div);
  });
}
buildParamEditor();

// ── Control sequence event types ─────────────────────────────────────────────
const EVENT_TYPES = [
  { value: 1, label: "1 – Strain Rate" },
  { value: 2, label: "2 – Relaxation" },
  { value: 3, label: "3 – Strain Accel." },
  { value: 4, label: "4 – Stress Rate" },
  { value: 5, label: "5 – Creep" },
  { value: 6, label: "6 – StressStrain" },
];

function addCtrlRow(tableBody, type = 1, tStart = 0, tEnd = 0, val = 0) {
  const tr = document.createElement("tr");
  const opts = EVENT_TYPES.map(e =>
    `<option value="${e.value}"${e.value == type ? " selected" : ""}>${e.label}</option>`
  ).join("");
  tr.innerHTML = `
    <td><select class="ctrl-row-type">${opts}</select></td>
    <td><input class="ctrl-row-num" type="text" value="${tStart}" placeholder="start time"></td>
    <td><input class="ctrl-row-num" type="text" value="${tEnd}" placeholder="end time"></td>
    <td><input class="ctrl-row-num" type="text" value="${val}" placeholder="value"></td>
    <td><button class="btn-icon" onclick="this.closest('tr').remove()" title="Remove row">✕</button></td>`;
  tableBody.appendChild(tr);
}

function initCtrlTable(rows) {
  const tbody = document.getElementById("ctrl-tbody");
  tbody.innerHTML = "";
  rows.forEach(r => addCtrlRow(tbody, r[0], r[1], r[2], r[3]));
}

document.getElementById("btn-add-row").addEventListener("click", () => {
  const tbody = document.getElementById("ctrl-tbody");
  const rows = tbody.querySelectorAll("tr");
  let lastEnd = 0;
  if (rows.length > 0) {
    const lastRow = rows[rows.length - 1];
    lastEnd = parseFloat(lastRow.querySelectorAll("input")[1].value) || 0;
  }
  addCtrlRow(tbody, 1, lastEnd, lastEnd, 0);
});

// ── Load example data ────────────────────────────────────────────────────────
// Example SL1T60 control rows (parsed from SL1T60.sctr):
const EXAMPLE_CTRL = [
  [1, 0,     191,   0.0008206],
  [5, 191,   11627, 0],
  [1, 11627, 11765, 0.000754],
  [5, 11765, 23114, 0],
  [1, 23114, 23255, 0.00118],
  [5, 23255, 34588, 0],
  [1, 34588, 46138, 0.00125],
];

async function loadExample() {
  // Load prm file and populate fields
  try {
    const res = await fetch("example/SL1T60.prm");
    if (res.ok) {
      const text = await res.text();
      loadPrmText(text);
    }
  } catch (e) { /* fall back to PARAMS defaults */ }

  initCtrlTable(EXAMPLE_CTRL);
}

loadExample();

// ── Parse and load .prm file text ────────────────────────────────────────────
function loadPrmText(text) {
  const tokens = text.trim().split(/\s+/).filter(t => t.length > 0);
  PARAMS.forEach((p, i) => {
    const el = document.getElementById(`p-${p.id}`);
    if (el && tokens[i] !== undefined) el.value = tokens[i];
  });
}

// ── Parse and load .sctr file text ───────────────────────────────────────────
function loadSctrText(text) {
  const rows = [];
  text.trim().split("\n").forEach(line => {
    const parts = line.trim().split(/\s+/);
    if (parts.length >= 1) {
      const type = parseInt(parts[0]);
      if (isNaN(type) || type === 0) return;
      rows.push([
        type,
        parseFloat(parts[1] || 0),
        parseFloat(parts[2] || 0),
        parseFloat(parts[3] || 0),
      ]);
    }
  });
  if (rows.length > 0) initCtrlTable(rows);
}

// ── File upload handlers ─────────────────────────────────────────────────────
document.getElementById("upload-prm").addEventListener("change", e => {
  const file = e.target.files[0]; if (!file) return;
  const reader = new FileReader();
  reader.onload = ev => loadPrmText(ev.target.result);
  reader.readAsText(file);
  e.target.value = "";
});

document.getElementById("upload-sctr").addEventListener("change", e => {
  const file = e.target.files[0]; if (!file) return;
  const reader = new FileReader();
  reader.onload = ev => loadSctrText(ev.target.result);
  reader.readAsText(file);
  e.target.value = "";
});

// ── Assemble .prm text from form ─────────────────────────────────────────────
function assemblePrm() {
  return PARAMS.map(p => {
    const el = document.getElementById(`p-${p.id}`);
    return el ? el.value.trim() : String(p.default);
  }).join("\n") + "\n";
}

// ── Assemble .sctr text from control table ────────────────────────────────────
function assembleSctr() {
  const rows = document.getElementById("ctrl-tbody").querySelectorAll("tr");
  let lines = "";
  rows.forEach(tr => {
    const sel = tr.querySelector("select").value;
    const inputs = tr.querySelectorAll("input");
    lines += `${sel}\t${inputs[0].value}\t${inputs[1].value}\t${inputs[2].value}\n`;
  });
  lines += "0\n";
  return lines;
}

// ── WASM loading ──────────────────────────────────────────────────────────────
let ntcModule = null;

function loadWasm() {
  if (typeof NTCSolver === "undefined") {
    setStatus("WASM module not loaded. Check that ntc_solver.js is present.", "error");
    return;
  }
  NTCSolver().then(m => {
    ntcModule = m;
    setStatus("Ready — click Run to simulate.", "");
    document.getElementById("btn-run").disabled = false;
  }).catch(err => {
    setStatus("Failed to initialize WASM module: " + err, "error");
  });
}

window.addEventListener("load", () => {
  // Defer WASM load slightly so Plotly and DOM settle
  setTimeout(loadWasm, 100);
});

// ── Status messages ───────────────────────────────────────────────────────────
function setStatus(msg, type = "") {
  const el = document.getElementById("status-msg");
  el.textContent = msg;
  el.className = type;
}

// ── Run simulation ────────────────────────────────────────────────────────────
let lastOutputData = null;

document.getElementById("btn-run").addEventListener("click", () => {
  if (!ntcModule) { setStatus("WASM module not ready yet.", "error"); return; }

  const paramContent = assemblePrm();
  const ctrlContent  = assembleSctr();

  setStatus("⟳ Running simulation…", "");
  document.getElementById("btn-run").disabled = true;

  // Run async so the browser can repaint the status message first
  setTimeout(() => {
    try {
      const result = ntcModule.runSimulation(paramContent, ctrlContent);
      document.getElementById("btn-run").disabled = false;

      if (result.startsWith("ERROR:")) {
        setStatus(result, "error");
        return;
      }

      lastOutputData = result;
      setStatus(`Done — ${countRows(result)} data points.`, "success");
      document.getElementById("btn-download").disabled = false;
      plotResult(result);
    } catch (e) {
      document.getElementById("btn-run").disabled = false;
      setStatus("Runtime error: " + e.message, "error");
    }
  }, 30);
});

function countRows(text) {
  return text.trim().split("\n").length - 1; // subtract header
}

// ── Parse output data ─────────────────────────────────────────────────────────
function parseOutput(text) {
  const lines = text.trim().split("\n");
  if (lines.length < 2) return { headers: [], columns: {} };
  const headers = lines[0].split("\t").map(h => h.trim());
  const columns = {};
  headers.forEach(h => columns[h] = []);
  for (let i = 1; i < lines.length; i++) {
    const vals = lines[i].split("\t");
    headers.forEach((h, j) => {
      const v = parseFloat(vals[j]);
      columns[h].push(isNaN(v) ? 0 : v);
    });
  }
  return { headers, columns };
}

// ── Axis selector population ──────────────────────────────────────────────────
function populateAxisSelectors(headers) {
  ["axis-x", "axis-y"].forEach(id => {
    const sel = document.getElementById(id);
    const prev = sel.value;
    sel.innerHTML = headers.map(h => `<option value="${h}">${h}</option>`).join("");
    if (headers.includes(prev)) sel.value = prev;
  });
  // Default: X = ep_ir, Y = Sigma
  const xSel = document.getElementById("axis-x");
  const ySel = document.getElementById("axis-y");
  if (!xSel.value || !document.querySelector("#axis-x option[value='ep_ir']") === false) {
    if (headers.includes("ep_ir")) xSel.value = "ep_ir";
  }
  if (!ySel.value || headers.includes("Sigma")) {
    if (headers.includes("Sigma")) ySel.value = "Sigma";
  }
}

// ── Plotly chart ──────────────────────────────────────────────────────────────
let parsedData = null;

function plotResult(text) {
  parsedData = parseOutput(text);
  if (!parsedData.headers.length) return;

  populateAxisSelectors(parsedData.headers);
  renderChart();

  // Show axis controls
  document.getElementById("output-controls").style.display = "flex";
}

function renderChart() {
  if (!parsedData) return;
  const xKey = document.getElementById("axis-x").value;
  const yKey = document.getElementById("axis-y").value;
  const xData = parsedData.columns[xKey] || [];
  const yData = parsedData.columns[yKey] || [];

  const isDark = document.body.classList.contains("dark");
  const textColor   = isDark ? "#e8eaf0" : "#1a1a2e";
  const gridColor   = isDark ? "#2d3348" : "#e8ecf0";
  const paperColor  = isDark ? "#1e2130" : "#f5f7fa";
  const plotBg      = isDark ? "#1e2130" : "#ffffff";

  const trace = {
    x: xData, y: yData,
    mode: "lines",
    line: { color: "#2563eb", width: 1.5 },
    type: "scatter",
    name: `${yKey} vs ${xKey}`,
  };

  const layout = {
    xaxis: { title: xKey, color: textColor, gridcolor: gridColor, zerolinecolor: gridColor },
    yaxis: { title: yKey, color: textColor, gridcolor: gridColor, zerolinecolor: gridColor },
    paper_bgcolor: paperColor,
    plot_bgcolor: plotBg,
    font: { color: textColor, size: 12 },
    margin: { l: 60, r: 20, t: 20, b: 50 },
    showlegend: false,
  };

  const config = { responsive: true, displayModeBar: true, displaylogo: false };
  Plotly.newPlot("chart-container", [trace], layout, config);
  document.getElementById("chart-placeholder").style.display = "none";
}

document.getElementById("axis-x").addEventListener("change", renderChart);
document.getElementById("axis-y").addEventListener("change", renderChart);

// Re-render chart on theme change to update colors
themeToggle.addEventListener("click", () => { if (parsedData) renderChart(); });

// ── Download output ───────────────────────────────────────────────────────────
document.getElementById("btn-download").addEventListener("click", () => {
  if (!lastOutputData) return;
  const blob = new Blob([lastOutputData], { type: "text/plain" });
  const a = document.createElement("a");
  a.href = URL.createObjectURL(blob);
  a.download = "ntc_output.dat";
  a.click();
  URL.revokeObjectURL(a.href);
});
