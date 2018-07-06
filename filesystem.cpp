#include "infra/filesystem.h"
#include "infra/exception.h"

#include <fstream>
#include <istream>
#include <sstream>

namespace infra::file {
#ifdef INFRA_HAS_FILESYSTEM

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
    return combinePath(base.parent_path(), file);
}

fs::path absoluteToRelativePath(const fs::path& absPath, const fs::path& root)
{
#ifdef HAVE_FILESYSTEM_H
    return fs::relative(absPath, root);
#else
    throw RuntimeError("absoluteToRelativePath not implemented");
#endif
}

fs::path relativeToAbsolutePath(const fs::path& relPath, const fs::path& base)
{
#ifdef HAVE_FILESYSTEM_H
    return fs::absolute(base / relPath);
#elif defined HAVE_EXP_FILESYSTEM_H
    return fs::absolute(relPath, base);
#else
    throw RuntimeError("relativeToAbsolutePath not implemented");
#endif
}

fs::path combinePath(const fs::path& base, const fs::path& file)
{
#ifdef HAVE_FILESYSTEM_H
    return (fs::absolute(base) / file).make_preferred();
#elif defined HAVE_EXP_FILESYSTEM_H
    return fs::absolute(file, base).make_preferred();
#else
    throw RuntimeError("combinePath not supported");
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
