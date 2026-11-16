#pragma once
#include "process.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

using namespace std;

enum ActionType {
    ACTION_TRIM = 1,
    ACTION_SUSPEND = 2,
    ACTION_TERMINATE = 3
};

struct plan {
    ActionType action_type = ACTION_TRIM;
    string action_name = "";
    string rationale = "";
    double estimated_freed_mb = 0.0;
    string risk_level = "LOW";
    string risk_details = "";
};

struct ai_engine {
    static double calculate_score(const process& p) {
        if (p.essential || !p.active || p.ram <= 0.0) return 0.0;
        double score = (p.ram * 0.7);
        if (p.background) {
            score += 150.0;
        }
        return score;
    }

    plan make_plan(const process& p) const {
        plan pl;
        string low_name = p.name;
        for (char &c : low_name) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        bool is_helper = (low_name.find("helper") != string::npos ||
                          low_name.find("service") != string::npos ||
                          low_name.find("worker") != string::npos ||
                          low_name.find("daemon") != string::npos ||
                          low_name.find("updater") != string::npos);

        if (is_helper || p.ram < 300.0) {
            pl.action_type = ACTION_TRIM;
            pl.action_name = "Trim Working Set / Flush Cache";
            pl.rationale = "Background helper/daemon detected. Cache trim releases memory pages safely.";
            pl.estimated_freed_mb = p.ram * 0.45;
            pl.risk_level = "LOW";
            pl.risk_details = "Safe operation. Process remains alive and no user data is affected.";
        }
        else if (p.ram < 900.0) {
            pl.action_type = ACTION_SUSPEND;
            pl.action_name = "Suspend Process (Preserve Memory Pages)";
            pl.rationale = "Application has active session state. Suspending stops CPU paging while preserving unsaved work.";
            pl.estimated_freed_mb = p.ram * 0.70;
            pl.risk_level = "MODERATE";
            pl.risk_details = "Process is paused without termination. Can be resumed anytime without data loss.";
        }
        else {
            pl.action_type = ACTION_TERMINATE;
            pl.action_name = "Polite Process Termination";
            pl.rationale = "Heavy memory footprint (>900 MB). Terminating frees substantial system RAM.";
            pl.estimated_freed_mb = p.ram;
            pl.risk_level = "HIGH";
            pl.risk_details = "App will close politely. User should ensure open work is saved.";
        }

        return pl;
    }
};
