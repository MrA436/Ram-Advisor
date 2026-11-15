#pragma once
#include "process.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
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
struct scanner {
    double total = 0.0;
    double used = 0.0;
    bool demo_mode = false;
    void read_memory() {
        if (demo_mode) {
            return;
        }
#ifdef _WIN32
        MEMORYSTATUSEX m;
        m.dwLength = sizeof(m);
        GlobalMemoryStatusEx(&m);
        total = m.ullTotalPhys / 1048576.0;
        used = total - (m.ullAvailPhys / 1048576.0);
#elif defined(__APPLE__)
        int mib[2] = {CTL_HW, HW_MEMSIZE};
        uint64_t mem = 0;
        size_t len = sizeof(mem);
        sysctl(mib, 2, &mem, &len, NULL, 0);
        total = mem / 1048576.0;
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
    }
    vector<process> get_demo_processes() {
        total = 16384.0;
        used = 12480.0;
        vector<process> list = {
            {5120, "chrome_render_helper", 1180.5, true, false, true, 0.0},
            {4280, "slack_desktop",        640.2,  true, false, true, 0.0},
            {3910, "spotify_music",        425.0,  true, false, true, 0.0},
            {6104, "discord_canary",       360.8,  true, false, true, 0.0},
            {7412, "steam_service",        275.4,  true, false, true, 0.0},
            {8291, "idle_python_worker",   195.0,  true, false, true, 0.0},
            {1102, "system_kernel",        1850.0, true, true,  true, 0.0},
            {9450, "code_editor",          850.0,  false, false, true, 0.0}
        };
        return list;
    }
    vector<process> scan() {
        if (demo_mode) {
            vector<process> list = get_demo_processes();
            cout << "\n[System Scanner] Running in PRESENTATION / DEMO MODE\n";
            cout << "Total System RAM: " << fixed << setprecision(1) << total / 1024.0 << " GB\n";
            cout << "Used RAM: " << used / 1024.0 << " GB (" << (used / total * 100.0) << "%)\n";
            cout << "Loaded simulated student workload (" << list.size() << " apps)\n";
            return list;
        }
        read_memory();
        map<string, process> byname;
        auto add_entry = [&](int pid, string name, double ram, bool bg, bool ess) {
            transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (ram < 40.0) return;
            auto it = byname.find(name);
            if (it == byname.end()) {
                byname[name] = {pid, name, ram, bg, ess, true, 0.0};
            } else {
                it->second.ram += ram;
                it->second.background = it->second.background && bg;
                it->second.essential = it->second.essential || ess;
            }
        };
        set<string> keep = {
            "system", "registry", "smss.exe", "csrss.exe", "wininit.exe",
            "winlogon.exe", "services.exe", "lsass.exe", "svchost.exe", "explorer.exe",
            "dwm.exe", "fontdrvhost.exe", "sihost.exe", "ctfmon.exe", "audiodg.exe",
            "msmpeng.exe", "memcompression", "systemd", "sshd", "dbus-daemon", "init",
            "launchd", "windowserver", "kernel_task", "finder", "dock", "loginwindow",
            "coreaudiod", "mds", "systemstats", "antigravity", "terminal", "zsh", "bash",
            "code", "iterm2", "controlcenter", "notificationcenter"
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
        PROCESSENTRY32 e;
        e.dwSize = sizeof(e);
        for (BOOL ok = Process32First(snap, &e); ok; ok = Process32Next(snap, &e)) {
            HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, e.th32ProcessID);
            if (!h) continue;
            PROCESS_MEMORY_COUNTERS c;
            if (GetProcessMemoryInfo(h, &c, sizeof(c))) {
                string n = e.szExeFile;
                string low = n;
                transform(low.begin(), low.end(), low.begin(), ::tolower);
                add_entry(e.th32ProcessID, n, c.WorkingSetSize / 1048576.0, !windowed.count(e.th32ProcessID),
                          keep.count(low) || e.th32ProcessID == me || e.th32ProcessID == e.th32ParentProcessID);
            }
            CloseHandle(h);
        }
        CloseHandle(snap);
#elif defined(__APPLE__)
        pid_t me = getpid();
        pid_t parent = getppid();
        int num_pids = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
        if (num_pids > 0) {
            vector<pid_t> pids(num_pids);
            int bytes_read = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), num_pids * sizeof(pid_t));
            int actual_count = bytes_read / (int)sizeof(pid_t);
            for (int i = 0; i < actual_count; i++) {
                pid_t pid = pids[i];
                if (pid <= 1) continue;
                struct proc_taskinfo pti;
                if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti)) <= 0) continue;
                char name_buf[PROC_PIDPATHINFO_MAXSIZE] = {0};
                if (proc_name(pid, name_buf, sizeof(name_buf)) <= 0) continue;
                string n = name_buf;
                string low = n;
                transform(low.begin(), low.end(), low.begin(), ::tolower);
                double ram_mb = pti.pti_resident_size / 1048576.0;
                bool is_ess = keep.count(low) || pid == me || pid == parent;
                add_entry(pid, n, ram_mb, true, is_ess);
            }
        }
#else
        DIR* d = opendir("/proc");
        if (d) {
            while (dirent* en = readdir(d)) {
                int pid = atoi(en->d_name);
                if (pid <= 0) continue;
                ifstream st("/proc/" + to_string(pid) + "/status");
                string line, name; double rss = 0; int uid = -1;
                while (getline(st, line)) {
                    if (line.rfind("Name:", 0) == 0) name = line.substr(6);
                    if (line.rfind("Uid:", 0) == 0) uid = atoi(line.c_str() + 5);
                    if (line.rfind("VmRSS:", 0) == 0) rss = atof(line.c_str() + 7) / 1024.0;
                }
                ifstream sf("/proc/" + to_string(pid) + "/stat");
                getline(sf, line);
                size_t rparen = line.rfind(')');
                if (rparen != string::npos && rparen + 2 < line.size()) {
                    istringstream ss(line.substr(rparen + 2));
                    string state; int ppid, grp, sess, tty = 0;
                    ss >> state >> ppid >> grp >> sess >> tty;
                    add_entry(pid, name, rss, tty == 0,
                              uid != (int)getuid() || keep.count(name) || pid == getpid() || pid == getppid() || pid <= 2);
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
        cout << "Used RAM : " << used / 1024.0 << " GB (" << (used / total * 100.0) << "%)\n";
        cout << "Discovered Applications: " << list.size() << " (grouped by name)\n";
        return list;
    }
};
