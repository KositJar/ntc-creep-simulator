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

    const total = ntcModule.initSimulation(paramContent, ctrlContent);
    if (total < 0) {
        postMessage({ type: 'error', msg: ntcModule.getError() || 'Initialization failed' });
        return;
    }

    // Run in chunks — setTimeout(0) yields between chunks so postMessage
    // is delivered and the main thread can update the progress bar.
    function runChunk() {
        if (ntcModule.isSimulationDone()) {
            finalize();
            return;
        }

        const pct = ntcModule.runNSteps(150);

        if (pct === -2) {
            const err = ntcModule.getError() || 'Solver error';
            postMessage({ type: 'error', msg: err });
            return;
        }

        postMessage({ type: 'progress', value: Math.min(pct, 99) });

        if (ntcModule.isSimulationDone()) {
            finalize();
        } else {
            setTimeout(runChunk, 0);
        }
    }

    setTimeout(runChunk, 0);
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
