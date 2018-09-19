#include "infra/filesystem.h"
#include "infra/exception.h"

#include <fstream>
#include <istream>
#include <sstream>

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

bool createDirectoryIfNotExists(const fs::path& path)
{
    if (path.empty() || fs::exists(path)) {
        return true;
    }

    return fs::create_directories(path);
}

fs::path combineAbsoluteWithRelativePath(const fs::path& base, const fs::path& file)
{
    assert(base.has_parent_path());
    auto preferredBase = base;
    return combinePath(forcePreferred(preferredBase).parent_path(), file);
}

fs::path absoluteToRelativePath(const fs::path& absPath, const fs::path& root)
{
#if defined(HAVE_FILESYSTEM_H) || defined(HAVE_BOOST_FILESYSTEM)
    return fs::relative(absPath, root);
#elif defined HAVE_EXP_FILESYSTEM_H
    // Start at the root path and while they are the same then do nothing then when they first
    // diverge take the entire from path, swap it with '..' segments, and then append the remainder of the to path.
    auto fromIter = root.begin();
    auto toIter = absPath.begin();

    // Loop through both while they are the same to find nearest common directory
    while (fromIter != root.end() && toIter != absPath.end() && (*toIter) == (*fromIter))
    {
        ++toIter;
        ++fromIter;
    }

    // Replace from path segments with '..' (from => nearest common directory)
    fs::path finalPath;
    while (fromIter != root.end())
    {
        finalPath /= "..";
        ++fromIter;
    }

    // Append the remainder of the to path (nearest common directory => to)
    while (toIter != absPath.end())
    {
        finalPath /= *toIter;
        ++toIter;
    }

    return finalPath;
#endif
}

fs::path relativeToAbsolutePath(const fs::path& relPath, const fs::path& base)
{
#ifdef HAVE_FILESYSTEM_H
    return fs::absolute(base / relPath);
#elif defined(HAVE_EXP_FILESYSTEM_H) || defined(HAVE_BOOST_FILESYSTEM)
    return fs::absolute(relPath, base);
#endif
}

fs::path combinePath(const fs::path& base, const fs::path& file)
{
#ifdef HAVE_FILESYSTEM_H
    return (fs::absolute(base) / file).make_preferred();
#elif defined(HAVE_EXP_FILESYSTEM_H) || defined(HAVE_BOOST_FILESYSTEM)
    return fs::absolute(file, base).make_preferred();
#endif
}

std::string readAsText(const fs::path& filename)
{
    return readAsText(filename.string());
}

#endif

std::string readAsText(const std::string& filename)
{
    return readAsText(filename.c_str());
}

std::string readAsText(const char* filename)
{
    std::ifstream fileStream(filename, std::ifstream::binary);
    if (!fileStream.is_open()) {
        throw RuntimeError("Failed to open file for reading: {}", filename);
    }

    return readAsText(fileStream);
}

std::string readAsText(const std::istream& fileStream)
{
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

void writeAsText(const std::string& filename, std::string_view contents)
{
    std::ofstream fs(filename.c_str(), std::ios::trunc | std::ios::binary);
    if (!fs.is_open()) {
        throw RuntimeError("Failed to open file for writing: {}", filename);
    }

    fs.write(contents.data(), contents.size());
}
}
