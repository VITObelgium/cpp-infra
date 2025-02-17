#include "infra/filesystem.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <fstream>
#include <istream>
#include <sstream>

#ifdef WIN32
#include <Windows.h>
#endif

namespace inf::file {
#ifdef INFRA_HAS_FILESYSTEM

static fs::path& forcePreferred(fs::path& p)
{
#ifndef _MSC_VER
    auto pStr = p.string();
    for (auto& c : pStr) {
        if (c == '\\') {
            c = '/';
        }
    }

    p = pStr;
    return p;
#else
    return p.make_preferred();
#endif
}

bool create_directory_if_not_exists(const fs::path& path)
{
    if (path.empty() || fs::exists(path)) {
        return true;
    }

    return fs::create_directories(path);
}

bool create_directory_for_file(const fs::path& path)
{
    if (path.has_parent_path()) {
        return fs::create_directories(path.parent_path());
    }

    return true;
}

fs::path combine_absolute_with_relative_path(const fs::path& base, const fs::path& file)
{
    assert(base.has_parent_path());
    auto preferredBase = base;
    return combine_path(forcePreferred(preferredBase).parent_path(), file);
}

fs::path absolute_to_relative_path(const fs::path& absPath, const fs::path& root)
{
#if defined(HAVE_FILESYSTEM_H) || defined(HAVE_BOOST_FILESYSTEM)
    return fs::relative(absPath, root);
#elif defined HAVE_EXP_FILESYSTEM_H
    // Start at the root path and while they are the same then do nothing then when they first
    // diverge take the entire from path, swap it with '..' segments, and then append the remainder of the to path.
    auto fromIter = root.begin();
    auto toIter   = absPath.begin();

    // Loop through both while they are the same to find nearest common directory
    while (fromIter != root.end() && toIter != absPath.end() && (*toIter) == (*fromIter)) {
        ++toIter;
        ++fromIter;
    }

    // Replace from path segments with '..' (from => nearest common directory)
    fs::path finalPath;
    while (fromIter != root.end()) {
        finalPath /= "..";
        ++fromIter;
    }

    // Append the remainder of the to path (nearest common directory => to)
    while (toIter != absPath.end()) {
        finalPath /= *toIter;
        ++toIter;
    }

    return finalPath;
#endif
}

fs::path relative_to_absolute_path(const fs::path& relPath, const fs::path& base)
{
#ifdef HAVE_FILESYSTEM_H
    if (base.empty()) {
        return fs::absolute(relPath);
    } else {
        return fs::absolute(base / relPath);
    }
#elif defined(HAVE_EXP_FILESYSTEM_H) || defined(HAVE_BOOST_FILESYSTEM)
    return fs::absolute(relPath, base);
#endif
}

fs::path combine_path(const fs::path& base, const fs::path& file)
{
#ifdef HAVE_FILESYSTEM_H
    if (base.empty()) {
        return fs::path(file).make_preferred();
    } else if (base.is_absolute()) {
        return (base / file).make_preferred();
    } else {
        return (fs::absolute(base) / file).make_preferred();
    }

#elif defined(HAVE_EXP_FILESYSTEM_H) || defined(HAVE_BOOST_FILESYSTEM)
    return fs::absolute(file, base).make_preferred();
#endif
}

std::vector<uint8_t> read(const fs::path& filename)
{
    std::ifstream fileStream(filename, std::ifstream::binary);
    if (!fileStream.is_open()) {
        throw RuntimeError("Failed to open file for reading: {}", filename);
    }

    fileStream.seekg(0, std::ios::end);
    const auto fileSize = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);

    std::vector<uint8_t> result(fileSize, 0);
    fileStream.read(reinterpret_cast<char*>(result.data()), result.size());
    return result;
}

std::string read_as_text(const fs::path& filename)
{
    std::ifstream fileStream(filename, std::ifstream::binary);
    if (!fileStream.is_open()) {
        throw RuntimeError("Failed to open file for reading: {}", filename);
    }

    return read_as_text(fileStream);
}

fs::path replace_illegal_path_characters(const fs::path& filename, char replacementChar)
{
    auto pathStr = filename.u8string();
    str::replace_in_place(pathStr, ':', replacementChar);
    str::replace_in_place(pathStr, '?', replacementChar);
    str::replace_in_place(pathStr, '*', replacementChar);
    str::replace_in_place(pathStr, '"', replacementChar);
    str::replace_in_place(pathStr, '<', replacementChar);
    str::replace_in_place(pathStr, '>', replacementChar);
    str::replace_in_place(pathStr, '/', replacementChar);
    str::replace_in_place(pathStr, '|', replacementChar);
    str::replace_in_place(pathStr, ':', replacementChar);
#if __cplusplus > 201703L
    return fs::path(pathStr);
#else
    return file::u8path(pathStr);
#endif
}

fs::path path_from_argv(char** const argv, int index, std::optional<size_t> offset)
{
#ifdef WIN32
    (void)argv;
    // On windows argv0 is not utf-8 encoded, obtain the unicode command line arguments
    int argc;
    auto arglist = CommandLineToArgvW(GetCommandLineW(), &argc);
    return fs::path(&arglist[index][offset.value_or(0)]);
#else
    return file::u8path(&argv[index][offset.value_or(0)]);
#endif
}

fs::path application_dir_from_argv0(char** const argv)
{
    return path_from_argv(argv, 0).parent_path();
}

#endif

std::string read_as_text(const std::string& filename)
{
    return read_as_text(filename.c_str());
}

std::string read_as_text(const char* filename)
{
    std::ifstream fileStream(filename, std::ifstream::binary);
    if (!fileStream.is_open()) {
        throw RuntimeError("Failed to open file for reading: {}", filename);
    }

    return read_as_text(fileStream);
}

std::string read_as_text(const std::istream& fileStream)
{
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

std::vector<std::string> read_lines(const fs::path& filename)
{
    return str::split(read_as_text(filename), "\r\n", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
}

void write(const fs::path& filename, std::span<const uint8_t> contents)
{
    if (filename.has_parent_path()) {
        fs::create_directories(filename.parent_path());
    }

    std::ofstream fs(filename, std::ios::trunc | std::ios::binary);
    if (!fs.is_open()) {
        throw RuntimeError("Failed to open file for writing: {}", filename);
    }

    fs.write(reinterpret_cast<const char*>(contents.data()), contents.size());
    if (fs.bad()) {
        throw RuntimeError("IO error writing file: {}", filename);
    } else if (fs.fail()) {
        throw RuntimeError("Error writing file: {}", filename);
    }
}

void write_as_text(const fs::path& filename, std::string_view contents)
{
    if (filename.has_parent_path()) {
        fs::create_directories(filename.parent_path());
    }

    std::ofstream fs(filename, std::ios::trunc | std::ios::binary);
    if (!fs.is_open()) {
        throw RuntimeError("Failed to open file for writing: {}", filename);
    }

    fs.write(contents.data(), contents.size());
    if (fs.bad()) {
        throw RuntimeError("IO error writing text file: {}", filename);
    } else if (fs.fail()) {
        throw RuntimeError("Error writing text file: {}", filename);
    }
}

void append_text_to_file(const fs::path& file, std::string_view contents)
{
    std::ofstream fs(file, std::ios::out | std::ios::app | std::ios::binary);
    if (!fs.is_open()) {
        throw RuntimeError("Failed to open file for appending: {}", file);
    }

    fs.write(contents.data(), contents.size());
    if (fs.bad()) {
        throw RuntimeError("IO error appending text to file: {}", file);
    } else if (fs.fail()) {
        throw RuntimeError("Error appending text to file: {}", file);
    }
}

void touch(const fs::path& filename)
{
    std::ofstream fs(filename, std::ios::app);
    if (!fs.is_open()) {
        throw RuntimeError("Failed to touch file: {}", filename);
    }
}

ScopedCurrentWorkingDirectory::ScopedCurrentWorkingDirectory(const fs::path& cwd)
: _prevCwd(fs::current_path())
{
    fs::current_path(cwd);
}

ScopedCurrentWorkingDirectory::~ScopedCurrentWorkingDirectory() noexcept
{
    if (!_prevCwd.empty()) {
        std::error_code ec;
        fs::current_path(_prevCwd, ec);
    }
}
}
