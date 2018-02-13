#include "infra/filesystem.h"
#include "infra/exception.h"

#include <fstream>
#include <sstream>

namespace infra {

fs::path make_preferred(const fs::path& p)
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

std::string readTextFile(const fs::path& filename)
{
    std::ifstream fileStream(filename.c_str(), std::ifstream::binary);

    if (!fileStream.is_open()) {
        throw RuntimeError("Failed to open file for reading: {}", filename.string());
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}
}
