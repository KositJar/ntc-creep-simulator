# CLAUDE.md — NTC Creep Simulator

Browser web app that runs the C++ **NTC (Nonlinear Three-Component) creep model**
compiled to WebAssembly. Deployed to GitHub Pages.

- **Live:** https://kositjar.github.io/ntc-creep-simulator/
- **Repo:** https://github.com/KositJar/ntc-creep-simulator
- **Developed by:** Kosit Jariyatatsakorn, Ph.D.

---

## ⚠️ Workflow rules (MUST follow on EVERY change)

For any code/asset change, in this order:

1. **Bump the version** — patch (z) for fixes/tweaks, minor (y) for new features.
   Update the version string in **all 7 locations** (see below).
2. **Add a changelog entry** in the "What's New" modal in `index.html`
   (newest at top: `<h3>vX.Y.Z — YYYY-MM-DD</h3>` + `<ul>` of changes).
3. **Commit** to git with a clear message.
4. **Push** to `main` (no need to ask first — push is always wanted).
   Pushing to `main` triggers GitHub Actions → rebuilds WASM → deploys to Pages.

### Version-string locations (keep all in sync)
- `src/web/app.js:1` — header comment
- `src/web/style.css:1` — header comment
- `src/web/index.html` — 5 spots: `<title>`, gate `.gate-ver`, navbar `.nav-version`,
  About modal `<h3>`, and the new changelog `<h3>` heading

---

## Architecture (read the file when you need detail — don't guess)

- **No framework / no build step for the frontend.** Plain HTML/CSS/JS + Plotly (CDN).
- `src/wasm/main_wasm.cpp` + `src/wasm/myfunc/*.h` — C++ engine (physics UNCHANGED from
  the desktop `3Comp_modern_v2`). Compiled with Emscripten via CI only.
- `src/web/worker.js` — runs WASM in a Web Worker; executes in **150-step chunks** with
  `setTimeout(0)` between chunks so the progress bar shows real %.
- `src/web/app.js` — all UI logic. Key pieces:
  - `PARAMS[]` — 56 entries, order MUST match `SetParam.h` reading order.
    Some entries use `type:"select"` (dropdown) or `widget:"af"` (custom widget).
  - `assemblePrm()` / `assembleCtrl()` — serialize form → `.prm` / `.ctr` text.
  - Password gate: SHA-256 hash at `src/web/app.js` const `PASSWORD_HASH`
    (read that line if you need to change the password — do NOT copy the hash here).
- `.github/workflows/deploy.yml` — Emscripten 3.1.64 build → `peaceiris/actions-gh-pages`.
  When adding a new web asset, remember to `cp` it into `dist/` here.

## File formats
- `.prm` — 56 whitespace-separated tokens (extra tokens ignored by `SetParam.h`).
- `.sctr` — 4 columns: `type start end value`.
- `.ctr` — 5 columns: `type start end dt value`. Auto-detected by column count.
- Event types: `1`=strain-rate, `2`=relaxation, `3`=strain-accel, `4`=stress-rate,
  `5`=creep, `6`=no-control; `0` terminates the file.

## Assets
- `src/web/icon.png` — square soil-layer app icon (transparent bg). Favicon + navbar.
- `src/web/icon-logo.png` — horizontal "3Comp" wordmark. Login gate + About modal.
- `src/web/icon.svg` — legacy, no longer referenced.
- `icon/` — original source PNGs (pre-processing), kept for regeneration.

## Examples
- `src/web/example/SC_T45.{prm,ctr}` — default (Hostun sand 45°C, 158 events).
- `src/web/example/SL1T60.{prm,sctr}` — older example.

## Environment note
Windows / PowerShell. Image processing (cropping, bg removal) done via inline
`System.Drawing` in PowerShell.
