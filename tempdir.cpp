#include "infra/tempdir.h"

namespace inf {

TempDir::TempDir()
: TempDir(std::string_view())
{
}

TempDir::TempDir(std::string_view name)
{
    const auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    try {
        _path = fs::temp_directory_path() / std::to_string(timestamp);
    } catch (const fs::filesystem_error& e) {
        if (e.code() == std::errc::not_a_directory && !fs::exists(e.path1())) {
            // The temp directory does not exist, so we need to create it.
            fs::create_directories(e.path1());
            _path = fs::temp_directory_path() / std::to_string(timestamp);
        } else {
            throw e;
        }
    }

    if (!name.empty()) {
        _path /= file::u8path(name);
    }

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
