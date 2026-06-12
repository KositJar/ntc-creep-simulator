// main_wasm.cpp — Emscripten entry point for the NTC Creep Simulator web app.
//
// Exposes a single function:
//   runSimulation(paramContent: string, ctrlContent: string) -> string
//
// Inputs are the raw text of a .prm and .sctr file.
// Returns the tab-separated output data (same format as .dat), or "ERROR: ..." on failure.
//
// The constitutive model headers in myfunc/ are used completely unchanged.

#include <emscripten/bind.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdexcept>

using namespace std;

#include "myfunc/SetParam.h"
#include "myfunc/FreezingStrainControl.h"
#include "myfunc/FreezingStressControl.h"
#include "myfunc/StrainControl.h"
#include "myfunc/StressControl.h"
#include "myfunc/StressStrainControl.h"
#include "myfunc/GenerateCtrl.h"

std::string runSimulation(std::string paramContent, std::string ctrlContent) {
    // Write inputs to Emscripten MEMFS virtual files
    {
        std::ofstream pf("/tmp/param.prm", std::ios::out);
        if (!pf) return "ERROR: Cannot create param file in MEMFS";
        pf << paramContent;
    }
    {
        std::ofstream cf("/tmp/ctrl.sctr", std::ios::out);
        if (!cf) return "ERROR: Cannot create control file in MEMFS";
        cf << ctrlContent;
    }

    Value   CurrValue;
    Control CurrCTR;

    std::ifstream PRMFILE("/tmp/param.prm", std::ios::in);
    if (!PRMFILE) return "ERROR: Cannot open param file";

    std::ofstream LOGFILE("/tmp/output.log", std::ios::out);
    std::ofstream DATFILE("/tmp/output.dat", std::ios::out);
    if (!DATFILE) return "ERROR: Cannot create output file in MEMFS";

    // Load control records from .sctr format
    vector<Control> controlQueue;
    size_t ctrlIdx = 0;
    {
        std::ifstream sctrFile("/tmp/ctrl.sctr", std::ios::in);
        if (!sctrFile) return "ERROR: Cannot open control file";

        auto simpleEvents = readSimplifiedControl(sctrFile);
        if (simpleEvents.empty()) return "ERROR: No control events found in control file";
        controlQueue = expandSimpleControl(simpleEvents);
        if (controlQueue.empty()) return "ERROR: Failed to expand control sequence";
    }

    // Read parameters and initialize state
    {
        SetParam setParam;
        CurrValue = setParam.setParam(PRMFILE);
        setParam.RecParam(LOGFILE);
    }

    if (!PRMFILE) return "ERROR: Parameter file is malformed or incomplete";

    // Load first control record
    CurrCTR = controlQueue[ctrlIdx++];
    LOGFILE << "Event:" << (int)CurrCTR.TypeofEvent << "\n"
            << "Time:" << CurrCTR.StartTime << "->" << CurrCTR.EndTime << "\n"
            << "dt:" << CurrCTR.dt << "\n"
            << "Value:" << CurrCTR.CTRValue << "\n";

    // Write header and initial state
    CurrValue.Output_Header(DATFILE);
    CurrValue.Output_Value(DATFILE);

    // Main simulation loop
    while (true) {
        Value PrevValue = CurrValue;

        if (CurrValue.Param.flg_freeze) {
            if (CurrCTR.TypeofEvent == StrainRateConstant ||
                CurrCTR.TypeofEvent == Relaxation ||
                CurrCTR.TypeofEvent == AccControl) {
                FreezingStrainControl control;
                CurrValue = control.NextStep(CurrValue, CurrCTR, LOGFILE);
            } else if (CurrCTR.TypeofEvent == StressRateConstant ||
                       CurrCTR.TypeofEvent == Creep) {
                FreezingStressControl control;
                CurrValue = control.NextStep(CurrValue, CurrCTR, LOGFILE);
            }
        } else if (CurrCTR.TypeofEvent == StrainRateConstant ||
                   CurrCTR.TypeofEvent == Relaxation ||
                   CurrCTR.TypeofEvent == AccControl) {
            StrainControl control;
            CurrValue = control.NextStep(CurrValue, CurrCTR, LOGFILE);
        } else if (CurrCTR.TypeofEvent == StressRateConstant ||
                   CurrCTR.TypeofEvent == Creep) {
            StressControl control;
            CurrValue = control.NextStep(CurrValue, CurrCTR, LOGFILE);
        }
        if (CurrCTR.TypeofEvent == StressStrainControl) {
            class StressStrainControl control;
            CurrValue = control.NextStep(CurrValue, CurrCTR, LOGFILE);
        }

        if (!CurrValue.result) {
            DATFILE.close();
            LOGFILE.close();
            return "ERROR: Solver could not converge — check parameters or control sequence";
        }

        CurrValue.Output_Value(DATFILE);

        // Advance time / fetch next control record
        if ((CurrValue.time + CurrCTR.dt) > CurrCTR.EndTime) {
            if (CurrValue.time < CurrCTR.EndTime - 0.01) {
                CurrCTR.dt = CurrCTR.EndTime - CurrValue.time;
            } else {
                if (ctrlIdx >= controlQueue.size()) break;
                CurrCTR = controlQueue[ctrlIdx++];
                if (!CurrCTR.TypeofEvent) break;  // End record
                LOGFILE << "Event:" << (int)CurrCTR.TypeofEvent << "\n"
                        << "Time:" << CurrCTR.StartTime << "->" << CurrCTR.EndTime << "\n"
                        << "dt:" << CurrCTR.dt << "\n"
                        << "Value:" << CurrCTR.CTRValue << "\n";
            }
        }

        {
            SetParam setParam;
            CurrValue = setParam.ChkDir(CurrValue, PrevValue, CurrCTR);
        }
    }

    DATFILE.close();
    LOGFILE.close();

    // Read and return the output data
    std::ifstream result("/tmp/output.dat", std::ios::in);
    if (!result) return "ERROR: Cannot read output file";
    std::string content((std::istreambuf_iterator<char>(result)),
                         std::istreambuf_iterator<char>());
    return content;
}

EMSCRIPTEN_BINDINGS(ntc_module) {
    emscripten::function("runSimulation", &runSimulation);
}
