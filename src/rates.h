#pragma once

#include <map>
#include <string>

struct Rates {
    std::string base;
    long long updated = 0;
    long long saved = 0;
    std::map<std::string, double> values;
};
