#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "process.h"
#include "dsa.h"
#include "scanner.h"
#include "analyzer.h"
#include "ai_engine.h"
#include "permission.h"
#include "optimizer.h"
using namespace std;
void print_banner() {
    cout << "============================================================\n";
    cout << "        RAM ADVISOR - INTELLIGENT MEMORY GOVERNOR          \n";
    cout << "      Semester 3 Data Structures & Algorithms Project       \n";
    cout << "============================================================\n";
}
int main() {
    print_banner();
    scanner sc;
    analyzer an;
    ai_engine ai;
    permission user;
    optimizer opt;
    cout << "Select Run Mode:\n";
    cout << "  [1] Live System Scan (Inspect actual processes on this computer)\n";
    cout << "  [2] Presentation Demo Mode (Simulated workload - safe for viva)\n";
    cout << "Choice [1/2, default 1]: ";
    string mode_input;
    if (getline(cin, mode_input)) {
        if (!mode_input.empty() && mode_input[0] == '2') {
            sc.demo_mode = true;
        }
    }
    vector<process> list = sc.scan();
    double initial_ram_used = sc.used;
    an.ram_history.enqueue(sc.used);
    while (true) {
        vector<process> ranked = an.rank_list(list);
        if (ranked.empty() || an.priority_heap.is_empty()) {
            cout << "\nOptimization complete. No further candidates available.\n";
            break;
        }
        process chosen;
        if (!an.priority_heap.extract_max(chosen)) {
            break;
        }
        plan pl = ai.make_plan(chosen);
        ActionType final_action = pl.action_type;
        UserDecision decision = user.ask(chosen, pl, final_action);
        if (decision == DECISION_QUIT) {
            cout << "\nExiting optimization cycle by user request.\n";
            break;
        } else if (decision == DECISION_SKIP) {
            cout << "Skipping " << chosen.name << ". Adding to hash table blacklist.\n";
            an.skipped_table.insert(chosen.pid);
        } else {
            for (process& p : list) {
                if (p.pid == chosen.pid) {
                    opt.execute(p, final_action, sc, an);
                    break;
                }
            }
            an.skipped_table.insert(chosen.pid);
        }
    }
    cout << "\n============================================================\n";
    cout << "                     FINAL SUMMARY                          \n";
    cout << "============================================================\n";
    cout << "Total System RAM  : " << fixed << setprecision(1) << sc.total / 1024.0 << " GB\n";
    cout << "RAM Before        : " << initial_ram_used << " MB ("
         << (initial_ram_used / sc.total * 100.0) << "%)\n";
    cout << "RAM After         : " << sc.used << " MB ("
         << (sc.used / sc.total * 100.0) << "%)\n";
    double total_reclaimed = initial_ram_used - sc.used;
    if (total_reclaimed < 0) total_reclaimed = 0;
    cout << "Total RAM Freed   : " << total_reclaimed << " MB\n";
    an.show_track(sc.total);
    cout << "\nThank you for using RAM Advisor.\n";
    return 0;
}
