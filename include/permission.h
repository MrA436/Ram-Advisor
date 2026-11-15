#pragma once
#include "process.h"
#include "ai_engine.h"
#include <iostream>
#include <iomanip>
#include <string>
using namespace std;
enum UserDecision {
    DECISION_APPROVE,
    DECISION_FORCE_TRIM,
    DECISION_FORCE_SUSPEND,
    DECISION_SKIP,
    DECISION_QUIT
};
struct permission {
    UserDecision ask(const process& p, const plan& pl, ActionType& final_action) const {
        cout << "\n------------------------------------------------------------\n";
        cout << "[User Approval] Proposed Optimization Action\n";
        cout << "------------------------------------------------------------\n";
        cout << "Target Application : " << p.name << " (PID: " << p.pid << ")\n";
        cout << "Current RAM Usage  : " << fixed << setprecision(1) << p.ram << " MB\n";
        cout << "Proposed Action    : " << pl.action_name << "\n";
        cout << "Rationale          : " << pl.rationale << "\n";
        cout << "Estimated Benefit  : ~" << fixed << setprecision(1) << pl.estimated_freed_mb << " MB\n";
        cout << "Assessed Risk      : [" << pl.risk_level << "] - " << pl.risk_details << "\n";
        cout << "------------------------------------------------------------\n";
        cout << "Options:\n";
        cout << "  [y] Approve proposed action (" << pl.action_name << ")\n";
        if (pl.action_type != ACTION_TRIM) {
            cout << "  [t] Non-destructive Working Set Trim (0% risk, keeps app running)\n";
        }
        if (pl.action_type == ACTION_TERMINATE) {
            cout << "  [s] Suspend instead (stops CPU/RAM paging, preserves unsaved work)\n";
        }
        cout << "  [n] Skip this application (add to hash table blacklist)\n";
        cout << "  [q] Finish optimization and view summary\n";
        char input;
        while (true) {
            cout << "\nEnter choice (y / "
                 << (pl.action_type != ACTION_TRIM ? "t / " : "")
                 << (pl.action_type == ACTION_TERMINATE ? "s / " : "")
                 << "n / q): ";
            if (!(cin >> input)) {
                return DECISION_QUIT;
            }
            input = tolower(input);
            if (input == 'y') {
                final_action = pl.action_type;
                return DECISION_APPROVE;
            } else if (input == 't' && pl.action_type != ACTION_TRIM) {
                final_action = ACTION_TRIM;
                return DECISION_FORCE_TRIM;
            } else if (input == 's' && pl.action_type == ACTION_TERMINATE) {
                final_action = ACTION_SUSPEND;
                return DECISION_FORCE_SUSPEND;
            } else if (input == 'n') {
                return DECISION_SKIP;
            } else if (input == 'q') {
                return DECISION_QUIT;
            } else {
                cout << "Invalid choice. Please enter one of the valid options.\n";
            }
        }
    }
};
