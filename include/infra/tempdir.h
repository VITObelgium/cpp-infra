#pragma once

#include "infra/filesystem.h"

#include <string_view>

namespace inf {

class TempDir
{
public:
    TempDir();
    TempDir(std::string_view name);
    ~TempDir() noexcept;

    const fs::path& path() const noexcept;

private:
    fs::path _path;
};

}
