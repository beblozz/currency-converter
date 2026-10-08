#include "api.h"

#include <ctime>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

const std::string API_URL = "https://open.er-api.com/v6/latest/USD";

bool downloadRates(Rates& rates, std::string& error) {
    cpr::Response response = cpr::Get(cpr::Url{API_URL}, cpr::Timeout{5000});

    if (response.error.code != cpr::ErrorCode::OK) {
        error = "network error: " + response.error.message;
        return false;
    }

    if (response.status_code != 200) {
        error = "server returned code " + std::to_string(response.status_code);
        return false;
    }

    try {
        nlohmann::json data = nlohmann::json::parse(response.text);

        if (data.at("result").get<std::string>() != "success") {
            error = "server did not return rates";
            return false;
        }

        Rates fresh;
        fresh.base = data.at("base_code").get<std::string>();
        fresh.updated = data.at("time_last_update_unix").get<long long>();
        fresh.saved = std::time(nullptr);
        fresh.values = data.at("rates").get<std::map<std::string, double>>();

        if (fresh.values.empty()) {
            error = "server returned empty list of rates";
            return false;
        }

        rates = fresh;
    } catch (const nlohmann::json::exception& e) {
        error = std::string("bad answer from server: ") + e.what();
        return false;
    }

    return true;
}
