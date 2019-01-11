#pragma once

#include <functional>

namespace rio {
class config;
class output;
class obshandler;

struct modelcomponents
{
    obshandler* obsHandler = nullptr;
    output* outputHandler  = nullptr;
};

/*! Run the model as configured in the rio config
 * \param cf the rio configuration
 * \param progresscb callback that gets called with a value between 0 and 100 to indicate progress
 *                   if the callback returns false the calculation is interrupted
 */
void run_model(const config& cf, std::function<bool(int)> progressCb = nullptr);

/*! Run the model as configured in the rio config with te possibility to override configured components
 * \param cf the rio configuration
 * \param components structure with component overrides, all none nullptr components will be used instead of the configured ones
 * \param progresscb callback that gets called with a value between 0 and 100 to indicate progress
 *                   if the callback returns false the calculation is interrupted
 */
void run_model(const config& cf, modelcomponents components, std::function<bool(int)> progressCb = nullptr);
}
