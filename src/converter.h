#pragma once

#include <string>

#include "rates.h"

std::string toUpper(std::string text);
bool parseAmount(const std::string& text, double& amount);
bool hasCurrency(const Rates& rates, const std::string& code);
double convert(const Rates& rates, double amount, const std::string& from, const std::string& to);
std::string formatTime(long long seconds);
