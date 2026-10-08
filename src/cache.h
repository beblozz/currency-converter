#pragma once

#include <string>

#include "rates.h"

std::string cachePath();
bool saveCache(const Rates& rates);
bool loadCache(Rates& rates);
