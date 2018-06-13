#include "infra/filesystem.h"
#include "infra/exception.h"

#include <fstream>
#include <istream>
#include <sstream>

namespace infra::file {
#ifdef INFRA_HAS_FILESYSTEM

static fs::path make_preferred(const fs::path& p)
{
    auto pStr = p.string();
    for (auto& c : pStr) {
        if (c == '\\') {
            c = '/';
        }
    }

    return fs::path(pStr);
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
    return combinePath(make_preferred(base).parent_path(), file);
}

fs::path combinePath(const fs::path& base, const fs::path& file)
{
#ifndef WIN32
    auto basePref = base;
    auto filePref = file;

    //return fs::canonical(fs::absolute(file, base.parent_path()));
    return fs::absolute(make_preferred(file), make_preferred(basePref));
#else
    return fs::canonical(fs::absolute(file, base)).make_preferred();
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
    std::ofstream fs(filename.c_str(), std::ios::trunc);
    if (!fs.is_open()) {
        throw RuntimeError("Failed to open file for writing: {}", filename);
    }

    fs << contents;
}
}
