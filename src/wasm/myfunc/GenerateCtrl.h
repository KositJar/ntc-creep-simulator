#if !defined GENERATE_CTRL_H
#define GENERATE_CTRL_H

// GenerateCtrl.h — translates the MATLAB "Generate_ctr_file_V3" pre-processing step into C++.
//
// Reads a simplified 4-column .sctr file:
//   event_type  t_start  t_end  ctrl_value
//   ...
//   0
//
// and expands it into a full sequence of Control records using the same quasi-logarithmic
// (1-2-5 decade) time-step pattern as the MATLAB script, guaranteeing that
// (EndTime - StartTime) / dt is a strict integer for every record.

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

#include "Control.h"

// ------------------------------------------------------------------
// SimpleControl: one row of the simplified 4-column .sctr input file
// ------------------------------------------------------------------
struct SimpleControl {
    int    event;       // TypeofEvent code (0-6)
    double t_start;     // phase start time (seconds)
    double t_end;       // phase end time (seconds)
    double ctrlValue;   // event-specific parameter (strain rate, stress rate, alpha, etc.)
};

// ------------------------------------------------------------------
// Quasi-logarithmic increment table (MATLAB's "Increment" array).
// del_t = 0.01 * CTR_INCREMENTS[i]
// Sequence follows a 1-2-5 repeating pattern across decades.
// ------------------------------------------------------------------
static const double CTR_INCREMENTS[19] = {
    1, 2, 5, 10, 20, 50, 100, 200, 500, 1000,
    2000, 5000, 10000, 20000, 50000, 100000, 500000, 1000000, 5000000
};

// ------------------------------------------------------------------
// generateEventRecords
// Expands one SimpleControl event into full Control records and
// appends them to `out`.
//
// Algorithm (mirrors MATLAB Generate_ctr_file_V3 logic):
//   Primary pass: walk through quasi-log windows of width del_t*100.
//     - Each full window always contains exactly 100 steps.
//     - When the last window overshoots t_end:
//         a) If steps is an integer -> truncate cleanly.
//         b) If steps is non-integer:
//              emit floor(steps) record from primary pass,
//              then run secondary pass from (t_end - frac*del_t).
//   Secondary pass (when needed): re-run quasi-log sequence from the
//     adjusted start point. Because the offset is an exact multiple
//     of 0.01 s, the final truncated window always lands on an integer.
// ------------------------------------------------------------------
static void generateEventRecords(const SimpleControl& sc, std::vector<Control>& out) {
    double t_ini   = sc.t_start;
    double t_end   = sc.t_end;
    // Creep events (type 5) always carry CTRValue = 0 per model convention
    double ctrlVal = (sc.event == (int)Creep) ? 0.0 : sc.ctrlValue;

    bool   need_c2  = false;
    double c2_start = sc.t_start;

    // ---- Primary pass ----
    int i = 0;
    while (t_ini < t_end && i < 19) {
        double del_t      = 0.01 * CTR_INCREMENTS[i];
        double window_end = t_ini + del_t * 100.0;

        if (window_end > t_end) {
            // Last segment: must truncate to t_end
            double steps = (t_end - t_ini) / del_t;
            double frac  = steps - std::floor(steps);

            if (frac > 1e-9) {
                // Non-integer steps: emit up to floor(steps), defer remainder to secondary pass
                long long int_steps = (long long)std::floor(steps);
                if (int_steps > 0) {
                    Control rec;
                    rec.TypeofEvent = (TypeofEvent)sc.event;
                    rec.StartTime   = t_ini;
                    rec.EndTime     = t_ini + (double)int_steps * del_t;
                    rec.dt          = del_t;
                    rec.CTRValue    = ctrlVal;
                    out.push_back(rec);
                }
                // b3 = frac * del_t  (fractional time remainder, multiple of 0.01 s)
                c2_start = t_end - frac * del_t;
                need_c2  = true;
            } else {
                // Integer steps: clean truncation, no secondary pass needed
                Control rec;
                rec.TypeofEvent = (TypeofEvent)sc.event;
                rec.StartTime   = t_ini;
                rec.EndTime     = t_end;
                rec.dt          = del_t;
                rec.CTRValue    = ctrlVal;
                out.push_back(rec);
            }
            break;

        } else {
            // Full window: always 100 integer steps; advance increment index
            Control rec;
            rec.TypeofEvent = (TypeofEvent)sc.event;
            rec.StartTime   = t_ini;
            rec.EndTime     = window_end;
            rec.dt          = del_t;
            rec.CTRValue    = ctrlVal;
            out.push_back(rec);
            t_ini = window_end;
            i++;
        }
    }

    if (!need_c2) return;

    // ---- Secondary pass: fills [c2_start, t_end] using fresh quasi-log sequence ----
    // Starting from t_end - b3, which is an exact multiple of 0.01 s, guarantees
    // the final truncated window has integer steps.
    t_ini = c2_start;
    i = 0;
    while (t_ini < t_end && i < 19) {
        double del_t      = 0.01 * CTR_INCREMENTS[i];
        double window_end = t_ini + del_t * 100.0;

        Control rec;
        rec.TypeofEvent = (TypeofEvent)sc.event;
        rec.StartTime   = t_ini;
        rec.dt          = del_t;
        rec.CTRValue    = ctrlVal;

        if (window_end > t_end) {
            rec.EndTime = t_end;
            out.push_back(rec);
            break;
        } else {
            rec.EndTime = window_end;
            out.push_back(rec);
            t_ini = window_end;
            i++;
        }
    }
}

// ------------------------------------------------------------------
// readSimplifiedControl
// Reads a .sctr file into a vector of SimpleControl.
// Lines starting with '#' (or containing '#') are treated as comments.
// The file is terminated by a line whose first token is 0.
// ------------------------------------------------------------------
inline std::vector<SimpleControl> readSimplifiedControl(std::ifstream& file) {
    std::vector<SimpleControl> events;
    std::string line;
    while (std::getline(file, line)) {
        // Strip inline comments
        auto hash_pos = line.find('#');
        if (hash_pos != std::string::npos)
            line = line.substr(0, hash_pos);
        // Skip blank lines
        if (line.find_first_not_of(" \t\r\n") == std::string::npos)
            continue;

        std::istringstream ss(line);
        int ev = -1;
        if (!(ss >> ev)) continue;
        if (ev == 0) break;

        SimpleControl sc;
        sc.event = ev;
        if (!(ss >> sc.t_start >> sc.t_end >> sc.ctrlValue))
            continue;   // skip malformed line
        events.push_back(sc);
    }
    return events;
}

// ------------------------------------------------------------------
// expandSimpleControl
// Expands all simplified events into a complete Control queue
// (terminated by an End record) ready for the NTC simulation loop.
// ------------------------------------------------------------------
inline std::vector<Control> expandSimpleControl(const std::vector<SimpleControl>& simpleEvents) {
    std::vector<Control> controls;
    for (const auto& sc : simpleEvents)
        generateEventRecords(sc, controls);

    // Append End sentinel
    Control endRec;
    endRec.TypeofEvent = End;
    endRec.StartTime = endRec.EndTime = endRec.dt = endRec.CTRValue = 0.0;
    controls.push_back(endRec);
    return controls;
}

#endif // GENERATE_CTRL_H
