#include "infra/configreader.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pugixml.hpp>

namespace infra
{

struct ConfigReader::Pimpl
{
};

ConfigReader::ConfigReader(const std::string& filename)
: _pimpl(std::make_unique<Pimpl>())
{
}

ConfigReader::~ConfigReader() = default;

ConfigNode ConfigReader::root()
{
    return ConfigNode();
}

} // namespace infra
