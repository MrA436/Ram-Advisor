#pragma once
#include "process.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <libproc.h>
#include <unistd.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#endif

using namespace std;

// ============================================================================
// SYSTEM SCANNER
// RAM Advisor: Intelligent Memory Governor
// Responsibilities:
// - Cross-platform memory inspection (Windows / macOS / Linux)
// - Process enumeration and memory footprint calculation
// - Safe demo mode for fail-safe viva presentations on any OS
// ============================================================================
struct scanner {
    double total = 0.0;     // Total RAM in MB
    double used = 0.0;      // Used RAM in MB
    bool demo_mode = false; // Safe simulated mode for presentations

    void read_memory() {
        if (demo_mode) {
            return;
        }

#ifdef _WIN32
        MEMORYSTATUSEX m;
        m.dwLength = sizeof(m);
        if (GlobalMemoryStatusEx(&m)) {
            total = m.ullTotalPhys / 1048576.0;
            used = total - (m.ullAvailPhys / 1048576.0);
        }
#elif defined(__APPLE__)
        int mib[2] = {CTL_HW, HW_MEMSIZE};
        uint64_t mem = 0;
        size_t len = sizeof(mem);
        if (sysctl(mib, 2, &mem, &len, NULL, 0) == 0) {
            total = mem / 1048576.0;
        }

        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        vm_statistics64_data_t vm_stat;
        if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
            used = ((uint64_t)vm_stat.active_count + vm_stat.wire_count + vm_stat.compressor_page_count) * (uint64_t)vm_page_size / 1048576.0;
        }
#else
        ifstream f("/proc/meminfo");
        string key, unit;
        double v, avail = 0;
        while (f >> key >> v >> unit) {
            if (key == "MemTotal:") total = v / 1024.0;
            if (key == "MemAvailable:") avail = v / 1024.0;
        }
        used = total - avail;
#endif
        if (used < 0.0) used = 0.0;
    }

    // Generate safe mock workload for viva demo
    vector<process> get_demo_processes() {
        total = 16384.0; // 16 GB
        used = 12480.0;  // ~76% used
        vector<process> list = {
            {5120, "chrome_render_helper", 1180.5, true, false, true, 0.0},
            {4280, "slack_desktop",        640.2,  true, false, true, 0.0},
            {3910, "spotify_music",        425.0,  true, false, true, 0.0},
            {6104, "discord_canary",       360.8,  true, false, true, 0.0},
            {7412, "steam_service",        275.4,  true, false, true, 0.0},
            {8291, "idle_python_worker",   195.0,  true, false, true, 0.0},
            {1102, "system_kernel",        1850.0, true, true,  true, 0.0}, // essential
            {9450, "code_editor",          850.0,  false, false, true, 0.0} // active foreground
        };
        return list;
    }

    vector<process> scan() {
        if (demo_mode) {
            vector<process> list = get_demo_processes();
            cout << "\n[System Scanner] Running in PRESENTATION / DEMO MODE\n";
            cout << "Total System RAM: " << fixed << setprecision(1) << total / 1024.0 << " GB\n";
            double pct = (total > 0.0) ? (used / total * 100.0) : 0.0;
            cout << "Used RAM: " << used / 1024.0 << " GB (" << pct << "%)\n";
            cout << "Loaded simulated student workload (" << list.size() << " apps)\n";
            return list;
        }

        read_memory();
        map<string, process> byname; // Aggregate multiple instances by app name

        auto add_entry = [&](int pid, string name, double ram, bool bg, bool ess) {
            for (char &c : name) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            if (ram < 40.0) return; // Skip tiny background threads

            auto it = byname.find(name);
            if (it == byname.end()) {
                byname[name] = {pid, name, ram, bg, ess, true, 0.0};
            } else {
                it->second.ram += ram;
                it->second.background = it->second.background && bg;
                it->second.essential = it->second.essential || ess;
            }
        };

        // System processes to protect across all platforms (normalized to lowercase)
        set<string> keep = {
            "system", "registry", "smss.exe", "csrss.exe", "wininit.exe",
            "winlogon.exe", "services.exe", "lsass.exe", "svchost.exe", "explorer.exe",
            "dwm.exe", "fontdrvhost.exe", "sihost.exe", "ctfmon.exe", "audiodg.exe",
            "msmpeng.exe", "memcompression", "systemd", "sshd", "dbus-daemon", "init",
            "launchd", "windowserver", "kernel_task", "finder", "dock", "loginwindow",
            "coreaudiod", "mds", "systemstats", "antigravity", "terminal", "zsh", "bash",
            "code", "iterm2", "controlcenter", "notificationcenter"
        };

        auto is_essential_name = [&](const string& low_name) -> bool {
            if (keep.count(low_name)) return true;
            if (low_name.size() > 4 && low_name.substr(low_name.size() - 4) == ".exe") {
                string without_exe = low_name.substr(0, low_name.size() - 4);
                if (keep.count(without_exe)) return true;
            }
            return false;
        };

#ifdef _WIN32
        set<DWORD> windowed;
        EnumWindows([](HWND h, LPARAM l) -> BOOL {
            if (IsWindowVisible(h) && GetWindowTextLengthA(h) > 0 && !GetWindow(h, GW_OWNER)) {
                DWORD pid;
                GetWindowThreadProcessId(h, &pid);
                ((set<DWORD>*)l)->insert(pid);
            }
            return TRUE;
        }, (LPARAM)&windowed);

        DWORD me = GetCurrentProcessId();
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 e;
            e.dwSize = sizeof(e);

            for (BOOL ok = Process32First(snap, &e); ok; ok = Process32Next(snap, &e)) {
                HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, e.th32ProcessID);
                if (!h) continue;
                PROCESS_MEMORY_COUNTERS c;
                if (GetProcessMemoryInfo(h, &c, sizeof(c))) {
                    string n = e.szExeFile;
                    string low = n;
                    for (char &ch : low) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    bool is_ess = is_essential_name(low) || e.th32ProcessID == me || e.th32ProcessID == e.th32ParentProcessID;
                    add_entry(e.th32ProcessID, n, c.WorkingSetSize / 1048576.0, !windowed.count(e.th32ProcessID), is_ess);
                }
                CloseHandle(h);
            }
            CloseHandle(snap);
        }
#elif defined(__APPLE__)
        pid_t me = getpid();
        pid_t parent = getppid();
        int bytes_needed = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
        if (bytes_needed > 0) {
            int num_pids = bytes_needed / (int)sizeof(pid_t) + 32; // Extra margin for new processes
            vector<pid_t> pids(num_pids);
            int bytes_read = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), (int)(pids.size() * sizeof(pid_t)));
            int actual_count = (bytes_read > 0) ? (bytes_read / (int)sizeof(pid_t)) : 0;
            for (int i = 0; i < actual_count; i++) {
                pid_t pid = pids[i];
                if (pid <= 1) continue;
                struct proc_taskinfo pti;
                if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti)) <= 0) continue;
                char name_buf[PROC_PIDPATHINFO_MAXSIZE] = {0};
                if (proc_name(pid, name_buf, sizeof(name_buf)) <= 0) continue;
                string n = name_buf;
                string low = n;
                for (char &ch : low) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                double ram_mb = pti.pti_resident_size / 1048576.0;
                bool is_ess = is_essential_name(low) || pid == me || pid == parent;
                add_entry(pid, n, ram_mb, true, is_ess);
            }
        }
#else
        DIR* d = opendir("/proc");
        if (d) {
            while (struct dirent* en = readdir(d)) {
                int pid = atoi(en->d_name);
                if (pid <= 0) continue;
                ifstream st("/proc/" + to_string(pid) + "/status");
                string line, name; double rss = 0; int uid = -1;
                while (getline(st, line)) {
                    if (line.rfind("Name:", 0) == 0) {
                        size_t start = line.find_first_not_of(" \t", 5);
                        name = (start != string::npos) ? line.substr(start) : "";
                    }
                    if (line.rfind("Uid:", 0) == 0) {
                        size_t start = line.find_first_not_of(" \t", 4);
                        if (start != string::npos) uid = atoi(line.c_str() + start);
                    }
                    if (line.rfind("VmRSS:", 0) == 0) {
                        size_t start = line.find_first_not_of(" \t", 6);
                        if (start != string::npos) rss = atof(line.c_str() + start) / 1024.0;
                    }
                }
                ifstream sf("/proc/" + to_string(pid) + "/stat");
                if (getline(sf, line)) {
                    size_t rparen = line.rfind(')');
                    if (rparen != string::npos && rparen + 2 < line.size()) {
                        istringstream ss(line.substr(rparen + 2));
                        string state; int ppid = 0, grp = 0, sess = 0, tty = 0;
                        ss >> state >> ppid >> grp >> sess >> tty;
                        string low = name;
                        for (char &ch : low) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                        bool is_ess = (uid != (int)getuid()) || is_essential_name(low) || 
                                      pid == getpid() || pid == getppid() || pid <= 2;
                        add_entry(pid, name, rss, tty == 0, is_ess);
                    }
                }
            }
            closedir(d);
        }
#endif

        vector<process> list;
        for (auto& kv : byname) {
            list.push_back(kv.second);
        }

        cout << "\n[System Scanner] Live Scan Complete\n";
        cout << "Total RAM: " << fixed << setprecision(1) << total / 1024.0 << " GB\n";
        double pct = (total > 0.0) ? (used / total * 100.0) : 0.0;
        cout << "Used RAM : " << used / 1024.0 << " GB (" << pct << "%)\n";
        cout << "Discovered Applications: " << list.size() << " (grouped by name)\n";
        return list;
    }
};
