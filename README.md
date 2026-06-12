# NTC Creep Simulator

A browser-based simulator for the **Nonlinear Three-Component (NTC) creep model** for geomaterials.

The constitutive model is compiled to WebAssembly (via Emscripten) and runs entirely in the browser — no installation or server required.

**Live app:** https://kositabn.github.io/ntc-creep-simulator/

## Scientific basis

- Di Benedetto, H., Tatsuoka, F., & Ishihara, M. (2002). Time-dependent shear deformation characteristics of sand and their constitutive modelling. *Soils and Foundations*, 42(2), 1–22.
- Tatsuoka, F., Ishihara, M., Di Benedetto, H., & Kuwano, R. (2002). Time-dependent shear deformation characteristics of geomaterials and their simulation. *Soils and Foundations*, 42(2), 103–129.

## Features

- Pre-loaded SL1T60 example — runs immediately on first visit
- Editable parameter form (collapsible sections: Elastic, Viscous, Reference Curve, Aging, Solver)
- Editable loading sequence table (`.sctr` format: event type, start/end time, control value)
- Upload custom `.prm` and `.sctr` files
- Interactive Plotly chart — default: R (stress ratio) vs. irreversible strain
- User-selectable X and Y axes from all output columns
- Download raw `.dat` output file
- Light/dark theme

## Output columns

| Column | Description |
|--------|-------------|
| `time (s)` | Time in seconds |
| `ep` | Total strain |
| `epd` | Strain rate |
| `ep_ir` | Irreversible strain |
| `epd_ir` | Irreversible strain rate |
| `Sigma` | Stress ratio R = σ₁'/σ₃' |
| `Sigma_f` | Inviscid stress component |
| `Sigma_fy` | Yield stress |
| `tau` | Creep damage variable |

## Repository structure

```
src/
  wasm/
    main_wasm.cpp       Emscripten wrapper (exposes runSimulation())
    myfunc/             Constitutive model headers (unchanged from 3Comp_modern_v2)
  web/
    index.html          Single-page app
    app.js              UI logic, WASM integration, Plotly chart
    style.css           Light/dark theme
    example/
      SL1T60.prm        Pre-loaded example parameters
      SL1T60.sctr       Pre-loaded example loading sequence
.github/workflows/
  deploy.yml            GitHub Actions: Emscripten build → GitHub Pages deploy
```

## Building locally

Install [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html), then:

```bash
mkdir dist
emcc src/wasm/main_wasm.cpp \
  -I src/wasm \
  -o dist/ntc_solver.js \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='NTCSolver' \
  -s FORCE_FILESYSTEM=1 \
  --bind -O2

cp src/web/index.html src/web/app.js src/web/style.css dist/
cp -r src/web/example dist/example

# Serve locally (required — file:// won't load WASM)
python -m http.server 8080 --directory dist
```

Then open http://localhost:8080

## Changing the password

The app uses a client-side SHA-256 password gate. To set a new password:

1. Open `src/web/app.js`
2. In a browser console, run: `await crypto.subtle.digest("SHA-256", new TextEncoder().encode("your-new-password")).then(b => Array.from(new Uint8Array(b)).map(x=>x.toString(16).padStart(2,"0")).join(""))`
3. Replace the `PASSWORD_HASH` constant at the top of `app.js` with the new hash
4. Push to `main` — CI will rebuild and deploy automatically
