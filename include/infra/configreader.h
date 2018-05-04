#pragma once

#include <memory>
#include <vector>

namespace infra
{

class ConfigNode
{
};

class ConfigReader
{
public:
    ConfigReader(const std::string& filename);
    ConfigReader(const ConfigReader&) = delete;
    ConfigReader& operator=(const ConfigReader&) = delete;
    ~ConfigReader();

    ConfigNode root();

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

} // namespace infra
