#pragma once

#include "infra/span.h"
#include "infra/string.h"
#include <fmt/core.h>
#include <fmt/std.h>
#include <iosfwd>
#include <string_view>
#include <vector>

#include <cassert>
#ifdef HAVE_FILESYSTEM_H
#include <filesystem>
namespace fs = std::filesystem;
#elif defined HAVE_EXP_FILESYSTEM_H
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif defined HAVE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

#if defined HAVE_FILESYSTEM_H || defined HAVE_EXP_FILESYSTEM_H || defined HAVE_BOOST_FILESYSTEM
#define INFRA_HAS_FILESYSTEM
#endif

namespace inf::file {
#ifdef INFRA_HAS_FILESYSTEM
/*! Create the specified directory and all parent directories if it does
 *  not exist.
 * /param path should specify a directory, not a file path
 * /return true if the directory was created, false otherwise
 */
bool create_directory_if_not_exists(const fs::path& path);

/*! Given a filename, make sure the directory exists
 * /param path should specify a file path, not a directory
 * /return true if the directory was created, false otherwise
 */
bool create_directory_for_file(const fs::path& path);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a file on disk (not a directory)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path combine_absolute_with_relative_path(const fs::path& baseFile, const fs::path& file);

/*!Create a relative path from an absolute path given a root dir
 * /param absPath the absolute path that will be converted to a relative path
 * /param root absolute path of the root directory
 * /return the relative path relative to the root path
 */
fs::path absolute_to_relative_path(const fs::path& absPath, const fs::path& root);

/*!Create an absolute path from a relative path
 * /param relPath the relative path that will be converted to an absolute path
 * /param base base directory for the relative reference
 * /return the absolute path
 */
fs::path relative_to_absolute_path(const fs::path& relPath, const fs::path& base);

/*!Create a full absolute path based on the base path and a relative file path
 * /param base the full path of a directory on disk (not a file)
 * /param file relative filepath that will be combined with the base
 * /return the full absolute path of the relative file argument
 */
fs::path combine_path(const fs::path& baseDir, const fs::path& file);

std::vector<uint8_t> read(const fs::path& filename);
std::string read_as_text(const fs::path& filename);

/*! Check the provided path for illegal characters on the running platform */
fs::path replace_illegal_path_characters(const fs::path& filename, char replacementChar);

inline fs::path u8path(std::string_view pathStr)
{
#ifdef HAVE_CPP20_U8STRING
    return fs::path(str::to_u8(pathStr));
#else
    return fs::u8path(pathStr);
#endif
}

inline std::string u8string(const fs::path& path)
{
#ifdef HAVE_CPP20_U8STRING
    return str::from_u8(path.u8string());
#else
    return path.u8string();
#endif
}

inline std::string generic_u8string(const fs::path& path)
{
#ifdef HAVE_CPP20_U8STRING
    return str::from_u8(path.generic_u8string());
#else
    return path.generic_u8string();
#endif
}

#endif

std::string read_as_text(const char* filename);
std::string read_as_text(const std::string& filename);
std::string read_as_text(const std::istream& filestream);
std::vector<std::string> read_lines(const fs::path& filename);
void write(const fs::path& filename, std::span<const uint8_t> contents);
void write_as_text(const fs::path& filename, std::string_view contents);
void append_text_to_file(const fs::path& file, std::string_view contents);
void touch(const fs::path& file);

class Handle
{
public:
    Handle() = default;
    Handle(const fs::path& p, const char* mode) noexcept
    {
        open(p, mode);
    }
    Handle(Handle&& other) noexcept
    {
        _ptr       = other._ptr;
        other._ptr = nullptr;
    }

    ~Handle() noexcept
    {
        close();
    }

    Handle& operator=(Handle&& other) noexcept
    {
        close();
        _ptr       = other._ptr;
        other._ptr = nullptr;
        return *this;
    }

    void open(const fs::path& p, const char* mode) noexcept
    {
        close();
#ifdef _WIN32
        std::string_view modeStr(mode);
        _wfopen_s(&_ptr, p.c_str(), std::wstring(modeStr.begin(), modeStr.end()).c_str());
#else
        _ptr = std::fopen(file::u8string(p).c_str(), mode);
#endif
    }

    void close() noexcept
    {
        if (_ptr) {
            std::fclose(_ptr);
            _ptr = nullptr;
        }
    }

    bool is_open() const noexcept
    {
        return _ptr != nullptr;
    }

    std::FILE* get() noexcept
    {
        return _ptr;
    }

    operator std::FILE*() noexcept
    {
        return _ptr;
    }

private:
    std::FILE* _ptr = nullptr;
};

class ScopedCurrentWorkingDirectory
{
public:
    ScopedCurrentWorkingDirectory(const fs::path& cwd);
    ~ScopedCurrentWorkingDirectory() noexcept;

private:
    fs::path _prevCwd;
};

}
