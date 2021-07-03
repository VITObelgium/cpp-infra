#include "infra/test/tempdir.h"

namespace inf {

TempDir::TempDir(std::string_view name)
{
    const auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    _path                = fs::temp_directory_path() / std::to_string(timestamp) / name;

    if (fs::exists(_path)) {
        fs::remove_all(_path);
    }

    fs::create_directories(_path);
}

TempDir::~TempDir() noexcept
{
    std::error_code ec;
    fs::remove_all(_path, ec);
}

const fs::path& TempDir::path() const noexcept
{
    return _path;
}
}
