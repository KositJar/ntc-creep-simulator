// main_wasm.cpp — Emscripten entry point for the NTC Creep Simulator web app.
//
// Exposes both a one-shot API (runSimulation) and a step-by-step API for
// real progress reporting from a Web Worker:
//   initSimulation(paramContent, ctrlContent) → int  (totalSteps estimate, -1 on error)
//   runNSteps(n)                              → int  (progress 0-100, -1 done, -2 error)
//   isSimulationDone()                        → bool
//   getOutput()                               → string (tab-separated .dat content)
//   getError()                                → string
//   runSimulation(paramContent, ctrlContent)  → string (one-shot, kept for compat)
//
// All constitutive model headers in myfunc/ are used completely unchanged.

#include <emscripten/bind.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <cmath>
#include <ctime>
#include <iostream>
#include <memory>

using namespace std;

#include "myfunc/SetParam.h"
#include "myfunc/FreezingStrainControl.h"
#include "myfunc/FreezingStressControl.h"
#include "myfunc/StrainControl.h"
#include "myfunc/StressControl.h"
#include "myfunc/StressStrainControl.h"
#include "myfunc/GenerateCtrl.h"

// ── Persistent simulation state ───────────────────────────────────────────────

struct SimState {
    Value   CurrValue;
    Control CurrCTR;
    vector<Control> controlQueue;
    size_t  ctrlIdx   = 0;
    double  totalTime = 1.0;   // max EndTime across all control records
    double  startTime = 0.0;   // initial time value
    bool    done      = false;
    bool    hasError  = false;
    string  errorMsg;
    ofstream* datFile = nullptr;
    ofstream* logFile = nullptr;
};

static SimState* gState = nullptr;

static void cleanupState() {
    if (gState) {
        if (gState->datFile) { gState->datFile->close(); delete gState->datFile; }
        if (gState->logFile) { gState->logFile->close(); delete gState->logFile; }
        delete gState;
        gState = nullptr;
    }
}

// ── Step helper ───────────────────────────────────────────────────────────────

static void advanceOneStep() {
    Value PrevValue = gState->CurrValue;

    if (gState->CurrValue.Param.flg_freeze) {
        if (gState->CurrCTR.TypeofEvent == StrainRateConstant ||
            gState->CurrCTR.TypeofEvent == Relaxation ||
            gState->CurrCTR.TypeofEvent == AccControl) {
            FreezingStrainControl ctrl;
            gState->CurrValue = ctrl.NextStep(gState->CurrValue, gState->CurrCTR, *gState->logFile);
        } else if (gState->CurrCTR.TypeofEvent == StressRateConstant ||
                   gState->CurrCTR.TypeofEvent == Creep) {
            FreezingStressControl ctrl;
            gState->CurrValue = ctrl.NextStep(gState->CurrValue, gState->CurrCTR, *gState->logFile);
        }
    } else if (gState->CurrCTR.TypeofEvent == StrainRateConstant ||
               gState->CurrCTR.TypeofEvent == Relaxation ||
               gState->CurrCTR.TypeofEvent == AccControl) {
        StrainControl ctrl;
        gState->CurrValue = ctrl.NextStep(gState->CurrValue, gState->CurrCTR, *gState->logFile);
    } else if (gState->CurrCTR.TypeofEvent == StressRateConstant ||
               gState->CurrCTR.TypeofEvent == Creep) {
        StressControl ctrl;
        gState->CurrValue = ctrl.NextStep(gState->CurrValue, gState->CurrCTR, *gState->logFile);
    }
    if (gState->CurrCTR.TypeofEvent == StressStrainControl) {
        class StressStrainControl ctrl;
        gState->CurrValue = ctrl.NextStep(gState->CurrValue, gState->CurrCTR, *gState->logFile);
    }

    if (!gState->CurrValue.result) {
        gState->hasError = true;
        gState->errorMsg = "ERROR: Solver could not converge — check parameters or control sequence";
        gState->datFile->close();
        gState->logFile->close();
        return;
    }

    gState->CurrValue.Output_Value(*gState->datFile);

    // Advance time / fetch next control record
    if ((gState->CurrValue.time + gState->CurrCTR.dt) > gState->CurrCTR.EndTime) {
        if (gState->CurrValue.time < gState->CurrCTR.EndTime - 0.01) {
            gState->CurrCTR.dt = gState->CurrCTR.EndTime - gState->CurrValue.time;
        } else {
            if (gState->ctrlIdx >= gState->controlQueue.size()) {
                gState->done = true;
                gState->datFile->close();
                gState->logFile->close();
                return;
            }
            gState->CurrCTR = gState->controlQueue[gState->ctrlIdx++];
            if (!gState->CurrCTR.TypeofEvent) {
                gState->done = true;
                gState->datFile->close();
                gState->logFile->close();
                return;
            }
        }
    }

    {
        SetParam sp;
        gState->CurrValue = sp.ChkDir(gState->CurrValue, PrevValue, gState->CurrCTR);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────

// Initialize simulation. Returns estimated total step count (>0), or -1 on error.
int initSimulation(std::string paramContent, std::string ctrlContent) {
    cleanupState();
    gState = new SimState();

    // Write inputs to Emscripten MEMFS
    { std::ofstream pf("/tmp/param.prm"); if (!pf) { gState->hasError=true; gState->errorMsg="ERROR: MEMFS param"; return -1; } pf << paramContent; }
    { std::ofstream cf("/tmp/ctrl.sctr"); if (!cf) { gState->hasError=true; gState->errorMsg="ERROR: MEMFS ctrl";  return -1; } cf << ctrlContent; }

    // Open persistent output files
    gState->logFile = new ofstream("/tmp/output.log");
    gState->datFile = new ofstream("/tmp/output.dat");
    if (!*gState->datFile) { gState->hasError=true; gState->errorMsg="ERROR: Cannot open output file"; return -1; }

    // Load and expand control sequence
    {
        std::ifstream sctrFile("/tmp/ctrl.sctr");
        auto simpleEvents = readSimplifiedControl(sctrFile);
        if (simpleEvents.empty()) { gState->hasError=true; gState->errorMsg="ERROR: No control events found"; return -1; }
        gState->controlQueue = expandSimpleControl(simpleEvents);
        if (gState->controlQueue.empty()) { gState->hasError=true; gState->errorMsg="ERROR: Empty control queue"; return -1; }
    }

    // Determine total simulation time for progress calculation
    for (const auto& c : gState->controlQueue) {
        if (c.TypeofEvent && c.EndTime > gState->totalTime)
            gState->totalTime = c.EndTime;
    }

    // Read parameters and initialise state
    {
        std::ifstream PRMFILE("/tmp/param.prm");
        if (!PRMFILE) { gState->hasError=true; gState->errorMsg="ERROR: Cannot open param file"; return -1; }
        SetParam sp;
        gState->CurrValue = sp.setParam(PRMFILE);
        sp.RecParam(*gState->logFile);
    }

    gState->startTime = gState->CurrValue.time;
    if (gState->totalTime <= gState->startTime) gState->totalTime = gState->startTime + 1.0;

    // Load first control record
    gState->CurrCTR = gState->controlQueue[gState->ctrlIdx++];

    // Write header + initial state
    gState->CurrValue.Output_Header(*gState->datFile);
    gState->CurrValue.Output_Value(*gState->datFile);

    // Estimate total steps (useful for caller; not used for % internally)
    int estSteps = 0;
    for (const auto& c : gState->controlQueue) {
        if (c.TypeofEvent && c.dt > 0)
            estSteps += (int)((c.EndTime - c.StartTime) / c.dt + 1);
    }
    return (estSteps > 0) ? estSteps : 1000;
}

// Run up to n steps. Returns progress % (0-99 while running, 100 when done, -2 on error).
int runNSteps(int n) {
    if (!gState || gState->done)  return 100;
    if (gState->hasError)         return -2;

    for (int i = 0; i < n && !gState->done && !gState->hasError; i++) {
        advanceOneStep();
    }

    if (gState->hasError) return -2;
    if (gState->done)     return 100;

    double span = gState->totalTime - gState->startTime;
    double elapsed = gState->CurrValue.time - gState->startTime;
    int pct = (span > 0) ? (int)(elapsed / span * 100.0) : 50;
    if (pct >= 100) pct = 99;
    if (pct < 0)    pct = 0;
    return pct;
}

bool isSimulationDone() {
    return !gState || gState->done || gState->hasError;
}

std::string getOutput() {
    if (!gState)           return "ERROR: No simulation state";
    if (gState->hasError)  return gState->errorMsg;
    if (!gState->done)     return "ERROR: Simulation not finished";

    std::ifstream result("/tmp/output.dat");
    if (!result) return "ERROR: Cannot read output file";
    return std::string((std::istreambuf_iterator<char>(result)),
                        std::istreambuf_iterator<char>());
}

std::string getError() {
    if (!gState || !gState->hasError) return "";
    return gState->errorMsg;
}

// One-shot synchronous wrapper (kept for compatibility / testing)
std::string runSimulation(std::string paramContent, std::string ctrlContent) {
    int n = initSimulation(paramContent, ctrlContent);
    if (n < 0) return getError();
    while (!isSimulationDone()) {
        int r = runNSteps(500);
        if (r == -2) return getError();
    }
    return getOutput();
}

// ── Emscripten bindings ───────────────────────────────────────────────────────
EMSCRIPTEN_BINDINGS(ntc_module) {
    emscripten::function("initSimulation",   &initSimulation);
    emscripten::function("runNSteps",        &runNSteps);
    emscripten::function("isSimulationDone", &isSimulationDone);
    emscripten::function("getOutput",        &getOutput);
    emscripten::function("getError",         &getError);
    emscripten::function("runSimulation",    &runSimulation);
}
