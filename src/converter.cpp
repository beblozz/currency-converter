#include "converter.h"

#include <cctype>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string toUpper(std::string text) {
    for (char& c : text) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return text;
}

bool parseAmount(const std::string& text, double& amount) {
    try {
        std::size_t used = 0;
        double value = std::stod(text, &used);

        if (used != text.size()) {
            return false;
        }
        if (!std::isfinite(value) || value < 0) {
            return false;
        }

        amount = value;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool hasCurrency(const Rates& rates, const std::string& code) {
    return rates.values.count(code) > 0;
}

double convert(const Rates& rates, double amount, const std::string& from, const std::string& to) {
    double inBase = amount / rates.values.at(from);
    return inBase * rates.values.at(to);
}

std::string formatTime(long long seconds) {
    std::time_t time = static_cast<std::time_t>(seconds);
    std::tm* local = std::localtime(&time);
    if (local == nullptr) {
        return "unknown";
    }

    std::ostringstream out;
    out << std::put_time(local, "%Y-%m-%d %H:%M");
    return out.str();
}
