#include "cache.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

std::string cachePath() {
    const char* home = std::getenv("USERPROFILE");
    if (home == nullptr) {
        home = std::getenv("HOME");
    }

    std::filesystem::path folder;
    if (home == nullptr) {
        folder = std::filesystem::current_path();
    } else {
        folder = home;
    }

    return (folder / ".currency_cache.json").string();
}

bool saveCache(const Rates& rates) {
    nlohmann::json data;
    data["base"] = rates.base;
    data["updated"] = rates.updated;
    data["saved"] = rates.saved;
    data["rates"] = rates.values;

    std::ofstream file(cachePath());
    if (!file.is_open()) {
        return false;
    }

    file << data.dump(2);
    return file.good();
}

bool loadCache(Rates& rates) {
    std::ifstream file(cachePath());
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json data = nlohmann::json::parse(file);

        Rates loaded;
        loaded.base = data.at("base").get<std::string>();
        loaded.updated = data.at("updated").get<long long>();
        loaded.saved = data.at("saved").get<long long>();
        loaded.values = data.at("rates").get<std::map<std::string, double>>();

        if (loaded.values.empty()) {
            return false;
        }

        rates = loaded;
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    return true;
}
