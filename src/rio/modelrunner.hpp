#pragma once

#include <functional>

namespace rio {
class config;
class output;

void run_model(const config& cf, std::function<bool(int)> progressCb = nullptr);
void run_model(const config& cf, output& output, std::function<bool(int)> progressCb = nullptr);
}
