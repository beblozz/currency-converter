#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>

#include "api.h"
#include "cache.h"
#include "converter.h"
#include "rates.h"

const long long CACHE_LIFETIME = 12 * 60 * 60;

void printHelp() {
    std::cout << "Currency converter with offline cache\n\n";
    std::cout << "Usage:\n";
    std::cout << "  currency convert <amount> <from> <to>   convert money\n";
    std::cout << "  currency update                         download fresh rates\n";
    std::cout << "  currency list                           show all currencies\n";
    std::cout << "  currency status                         show info about cache\n";
    std::cout << "  currency help                           show this text\n\n";
    std::cout << "Example:\n";
    std::cout << "  currency convert 100 EUR USD\n";
}

void printDate(const Rates& rates, bool offline) {
    std::cout << "Rates updated: " << formatTime(rates.updated);
    if (offline) {
        std::cout << " (offline, from cache)";
    }
    std::cout << "\n";
}

bool getRates(Rates& rates, bool& offline) {
    offline = false;

    Rates cached;
    bool hasCache = loadCache(cached);

    if (hasCache) {
        long long age = std::time(nullptr) - cached.saved;
        if (age >= 0 && age < CACHE_LIFETIME) {
            rates = cached;
            return true;
        }
    }

    std::string error;
    if (downloadRates(rates, error)) {
        if (!saveCache(rates)) {
            std::cerr << "Warning: could not save cache to " << cachePath() << "\n";
        }
        return true;
    }

    if (hasCache) {
        std::cerr << "Warning: " << error << "\n";
        rates = cached;
        offline = true;
        return true;
    }

    std::cerr << "Error: " << error << "\n";
    std::cerr << "There is no saved cache to use, check your internet connection and try again\n";
    return false;
}

int runConvert(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Error: wrong number of arguments\n";
        std::cerr << "Usage: currency convert <amount> <from> <to>\n";
        return 1;
    }

    double amount = 0;
    if (!parseAmount(argv[2], amount)) {
        std::cerr << "Error: '" << argv[2] << "' is not a correct amount\n";
        return 1;
    }

    std::string from = toUpper(argv[3]);
    std::string to = toUpper(argv[4]);

    Rates rates;
    bool offline = false;
    if (!getRates(rates, offline)) {
        return 1;
    }

    if (!hasCurrency(rates, from)) {
        std::cerr << "Error: unknown currency '" << from << "'\n";
        return 1;
    }
    if (!hasCurrency(rates, to)) {
        std::cerr << "Error: unknown currency '" << to << "'\n";
        return 1;
    }

    double result = convert(rates, amount, from, to);
    double one = convert(rates, 1, from, to);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << amount << " " << from << " = " << result << " " << to << "\n";
    std::cout << std::setprecision(4);
    std::cout << "Rate: 1 " << from << " = " << one << " " << to << "\n";
    printDate(rates, offline);
    return 0;
}

int runUpdate() {
    Rates rates;
    std::string error;

    if (!downloadRates(rates, error)) {
        std::cerr << "Error: " << error << "\n";
        return 1;
    }

    if (!saveCache(rates)) {
        std::cerr << "Error: could not save cache to " << cachePath() << "\n";
        return 1;
    }

    std::cout << "Downloaded " << rates.values.size() << " currencies\n";
    printDate(rates, false);
    return 0;
}

int runList() {
    Rates rates;
    bool offline = false;
    if (!getRates(rates, offline)) {
        return 1;
    }

    std::cout << std::fixed << std::setprecision(4);
    for (const auto& item : rates.values) {
        std::cout << item.first << "  " << item.second << "\n";
    }

    std::cout << "\nBase currency: " << rates.base << "\n";
    printDate(rates, offline);
    return 0;
}

int runStatus() {
    Rates rates;
    if (!loadCache(rates)) {
        std::cout << "Cache is empty, run: currency update\n";
        return 0;
    }

    long long age = std::time(nullptr) - rates.saved;

    std::cout << "Cache file: " << cachePath() << "\n";
    std::cout << "Currencies: " << rates.values.size() << "\n";
    std::cout << "Rates updated: " << formatTime(rates.updated) << "\n";
    std::cout << "Downloaded: " << formatTime(rates.saved) << " (" << age / 3600 << " h ago)\n";

    if (age < CACHE_LIFETIME) {
        std::cout << "Cache is fresh\n";
    } else {
        std::cout << "Cache is old, it will be updated on next run\n";
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string command = argv[1];

    if (command == "convert") {
        return runConvert(argc, argv);
    }
    if (command == "update") {
        return runUpdate();
    }
    if (command == "list") {
        return runList();
    }
    if (command == "status") {
        return runStatus();
    }
    if (command == "help") {
        printHelp();
        return 0;
    }

    std::cerr << "Error: unknown command '" << command << "'\n\n";
    printHelp();
    return 1;
}
