#include "licensemanager.h"

#include <rlm/license.h>

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace infra {

struct LicenseManager::Pimpl
{
    RLM_HANDLE handle = nullptr;
    RLM_LICENSE license = nullptr;
};

LicenseManager::LicenseManager(const std::string& licenseDir, const std::string& applicationName, const std::string& applicationVersion)
: _pimpl(std::make_unique<Pimpl>())
{
    char errstring[RLM_ERRSTRING_MAX];

    _pimpl->handle = rlm_init(".", licenseDir.c_str(), nullptr);
    auto stat = rlm_stat(_pimpl->handle);
    if (stat)
    {
        rlm_errstring(nullptr, _pimpl->handle, errstring);
        throw std::runtime_error("Error initializing license system" + std::string(errstring));
    }

    int count = 1;
    _pimpl->license = rlm_checkout(_pimpl->handle, applicationName.c_str(), applicationVersion.c_str(), count);

    stat = rlm_license_stat(_pimpl->license);
    if (stat == RLM_EL_INQUEUE)
    {

    }
    else if (stat != 0)
    {
        rlm_errstring_num(stat, errstring);
        throw std::runtime_error("Error initializing license system (" + std::string(errstring) + ")");
    }
}

LicenseManager::~LicenseManager() noexcept
{
    rlm_checkin(_pimpl->license);
    rlm_close(_pimpl->handle);
}

}
