#pragma once
#include <string>
#include <cstdint>

// ============================================================================
// SHARED PROCESS DATA STRUCTURE
// RAM Advisor: Intelligent Memory Governor
// ============================================================================

struct process {
    int pid = 0;
    std::string name = "";
    double ram = 0.0;          // Memory usage in MB
    bool background = false;   // True if process has no visible active GUI window
    bool essential = false;    // True if protected OS/system service
    bool active = true;        // True if currently running
    double score = 0.0;        // Priority score calculated by AI Decision Engine

    // Backwards-compatibility members
    double cpu_usage = 0.0;
    int base_priority = 0;
    int age = 0;

    double memory_usage() const { return ram; }
    double effective_priority() const { return score; }
};

// Aliases for compatibility
using Process = process;

// Comparator for priority queue (highest score first; tiebreak by higher RAM)
struct ProcessComparator {
    bool operator()(const process& a, const process& b) const {
        if (a.score != b.score) return a.score < b.score;
        return a.ram < b.ram;
    }
};
