// worker.js — Web Worker that loads the NTC WASM module and runs the simulation
// in chunks, posting real progress updates back to the main thread.

importScripts('ntc_solver.js');

let ntcModule = null;

NTCSolver().then(m => {
    ntcModule = m;
    postMessage({ type: 'ready' });
}).catch(err => {
    postMessage({ type: 'error', msg: 'Failed to load WASM module: ' + err });
});

onmessage = function(e) {
    if (e.data.type === 'run') {
        startSimulation(e.data.paramContent, e.data.ctrlContent);
    }
};

function startSimulation(paramContent, ctrlContent) {
    if (!ntcModule) {
        postMessage({ type: 'error', msg: 'WASM module not ready' });
        return;
    }

    postMessage({ type: 'progress', value: 0 });

    // If the step-by-step API is available, use it for real progress %.
    // Otherwise fall back to the one-shot API (old cached WASM).
    if (typeof ntcModule.initSimulation === 'function') {
        runChunked(paramContent, ctrlContent);
    } else {
        runOneShot(paramContent, ctrlContent);
    }
}

function runChunked(paramContent, ctrlContent) {
    const total = ntcModule.initSimulation(paramContent, ctrlContent);
    if (total < 0) {
        postMessage({ type: 'error', msg: ntcModule.getError() || 'Initialization failed' });
        return;
    }

    function runChunk() {
        if (ntcModule.isSimulationDone()) { finalize(); return; }

        const pct = ntcModule.runNSteps(150);
        if (pct === -2) { postMessage({ type: 'error', msg: ntcModule.getError() || 'Solver error' }); return; }

        postMessage({ type: 'progress', value: Math.min(pct, 99) });

        if (ntcModule.isSimulationDone()) finalize();
        else setTimeout(runChunk, 0);
    }
    setTimeout(runChunk, 0);
}

function runOneShot(paramContent, ctrlContent) {
    // Fallback for old cached WASM that only has runSimulation()
    postMessage({ type: 'progress', value: 10 });
    try {
        const result = ntcModule.runSimulation(paramContent, ctrlContent);
        postMessage({ type: 'progress', value: 100 });
        if (result.startsWith('ERROR:')) postMessage({ type: 'error', msg: result });
        else postMessage({ type: 'result', data: result });
    } catch (e) {
        postMessage({ type: 'error', msg: 'Runtime error: ' + e.message });
    }
}

function finalize() {
    postMessage({ type: 'progress', value: 100 });
    const output = ntcModule.getOutput();
    if (output.startsWith('ERROR:')) {
        postMessage({ type: 'error', msg: output });
    } else {
        postMessage({ type: 'result', data: output });
    }
}
