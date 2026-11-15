#pragma once
#include "process.h"
#include "dsa.h"
#include "ai_engine.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>

using namespace std;

// ============================================================================
// ANALYZER & RANKING ENGINE
// RAM Advisor: Intelligent Memory Governor
// Responsibilities:
// - Maintain RAM reading history using CustomCircularQueue
// - Filter skipped processes using CustomHashTable (O(1) lookup)
// - Sort candidate processes using CustomMergeSort (O(N log N))
// - Select optimal recommendation using CustomMaxHeap (O(log N))
// ============================================================================

struct analyzer {
    CustomCircularQueue ram_history;
    CustomHashTable skipped_table;
    CustomMaxHeap priority_heap;

    // Filters eligible candidates, sorts with MergeSort, and builds MaxHeap
    vector<process> rank_list(vector<process>& list) {
        vector<process> eligible;

        // 1. Calculate scores and filter candidates
        for (process& p : list) {
            p.score = ai_engine::calculate_score(p);
            if (p.active && !p.essential && p.score > 0.0) {
                // Check if already skipped or handled using CustomHashTable
                if (!skipped_table.contains(p.pid)) {
                    eligible.push_back(p);
                }
            }
        }

        // 2. Sort eligible candidates using custom MergeSort
        if (!eligible.empty()) {
            custom_merge_sort(eligible.data(), 0, static_cast<int>(eligible.size()) - 1);
        }

        // 3. Build CustomMaxHeap for priority retrieval
        priority_heap.clear();
        for (const process& p : eligible) {
            priority_heap.insert(p);
        }

        // 4. Display ranked candidates table
        cout << "\n------------------------------------------------------------\n";
        cout << "[Analyzer] Process Priority Ranking (Custom MergeSort & MaxHeap)\n";
        cout << "------------------------------------------------------------\n";
        if (eligible.empty()) {
            cout << "No safe candidates available for optimization.\n";
            return eligible;
        }

        cout << left << setw(4) << "No." 
             << setw(8) << "PID" 
             << setw(26) << "Process Name" 
             << setw(12) << "RAM (MB)" 
             << setw(10) << "Score" << "\n";
        cout << string(60, '-') << "\n";

        size_t show_count = min(static_cast<size_t>(5), eligible.size());
        for (size_t i = 0; i < show_count; i++) {
            cout << left << setw(4) << (i + 1)
                 << setw(8) << eligible[i].pid
                 << setw(26) << (eligible[i].name.size() > 24 ? eligible[i].name.substr(0, 22) + ".." : eligible[i].name)
                 << setw(12) << fixed << setprecision(1) << eligible[i].ram
                 << setw(10) << fixed << setprecision(1) << eligible[i].score << "\n";
        }

        return eligible;
    }

    void show_track(double total) const {
        cout << "\n------------------------------------------------------------\n";
        cout << "[Analyzer] RAM Usage History (Custom Circular Queue Audit)\n";
        cout << "------------------------------------------------------------\n";
        int n = ram_history.size();
        if (n == 0) {
            cout << "No historical readings recorded.\n";
            return;
        }

        for (int i = 0; i < n; i++) {
            double reading = ram_history.get_at(i);
            double pct = (total > 0.0) ? (reading / total * 100.0) : 0.0;
            cout << "Snapshot #" << (i + 1) << ": " 
                 << fixed << setprecision(1) << reading << " MB (" 
                 << fixed << setprecision(1) << pct << "% system RAM)\n";
        }
    }
};
