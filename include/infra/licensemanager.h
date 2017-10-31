#pragma once

#include <string>
#include <memory>

namespace infra {

class LicenseManager {
public:
    LicenseManager (const std::string& licenseDir, const std::string& applicationName, const std::string& applicationVersion);
    ~LicenseManager () noexcept;

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

}