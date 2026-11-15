#pragma once
#include <string>

struct process {
    int pid;
    std::string name;
    double ram;
    bool background;
    bool essential;
    bool active;
    double score;
};