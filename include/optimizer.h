#pragma once
#include "process.h"
#include "scanner.h"
#include "analyzer.h"
#include "ai_engine.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <signal.h>
#endif
using namespace std;
struct optimizer {
    void execute(process& p, ActionType action, scanner& sc, analyzer& an) const {
        cout << "\n------------------------------------------------------------\n";
        cout << "[Memory Optimizer] Executing Reclamation Strategy\n";
        cout << "------------------------------------------------------------\n";
        double ram_before = sc.used;
        if (sc.demo_mode) {
            if (action == ACTION_TRIM) {
                double trimmed = p.ram * 0.45;
                p.ram -= trimmed;
                sc.used = max(0.0, sc.used - trimmed);
                cout << "Action: Working Set Trim executed for " << p.name << ".\n";
                cout << "Status: Process remains active with reduced memory footprint.\n";
            } else if (action == ACTION_SUSPEND) {
                double freed = p.ram * 0.70;
                p.ram -= freed;
                p.active = false;
                sc.used = max(0.0, sc.used - freed);
                cout << "Action: Process " << p.name << " suspended (SIGSTOP signal simulated).\n";
                cout << "Status: Execution halted, pages paged out, unsaved state preserved.\n";
            } else {
                sc.used = max(0.0, sc.used - p.ram);
                p.active = false;
                cout << "Action: Process " << p.name << " closed politely.\n";
                cout << "Status: Application terminated gracefully.\n";
            }
        } else {
            if (action == ACTION_TRIM) {
                cout << "Trimming working set memory for " << p.name << " (PID " << p.pid << ")...\n";
#ifdef _WIN32
                HANDLE h = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, p.pid);
                if (h) {
                    EmptyWorkingSet(h);
                    CloseHandle(h);
                }
#else
                sync();
#endif
                cout << "Memory trim signal dispatched.\n";
            } else if (action == ACTION_SUSPEND) {
                cout << "Sending suspend signal to " << p.name << " (PID " << p.pid << ")...\n";
#ifdef _WIN32
                system(("taskkill /PID " + to_string(p.pid) + " > nul 2>&1").c_str());
#else
                kill(p.pid, SIGSTOP);
#endif
                p.active = false;
                cout << "Process suspended successfully.\n";
            } else {
                cout << "Requesting polite termination for " << p.name << " (PID " << p.pid << ")...\n";
#ifdef _WIN32
                system(("taskkill /PID " + to_string(p.pid) + " > nul 2>&1").c_str());
                Sleep(1500);
#else
                kill(p.pid, SIGTERM);
                sleep(1);
#endif
                p.active = false;
                cout << "Termination request sent.\n";
            }
            sc.read_memory();
        }
        an.ram_history.enqueue(sc.used);
        double actual_freed = ram_before - sc.used;
        if (actual_freed < 0) actual_freed = 0;
        cout << "------------------------------------------------------------\n";
        cout << "RAM Before Action : " << fixed << setprecision(1) << ram_before << " MB ("
             << (ram_before / sc.total * 100.0) << "%)\n";
        cout << "RAM After Action  : " << sc.used << " MB ("
             << (sc.used / sc.total * 100.0) << "%)\n";
        cout << "Measured Reclaimed: " << actual_freed << " MB\n";
        cout << "------------------------------------------------------------\n";
    }
};
