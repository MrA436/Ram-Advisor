
#pragma once
#include <string>
#include <cstdint>

// Represents a single process/task in the system
struct Process {
    int pid;
    std::string name;
    double cpu_usage;
    double memory_usage;
    int base_priority;
    double ai_score;
    int age;

    // Higher ai_score + age bonus = higher effective priority
    double effective_priority() const {
        return ai_score + (age * 0.5);
    }
};

// Comparator for max-heap (highest effective priority on top)
struct ProcessComparator {
    bool operator()(const Process& a, const Process& b) const {
        return a.effective_priority() < b.effective_priority();
    }
};
