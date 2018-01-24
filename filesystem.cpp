#include "infra/filesystem.h"

namespace infra {

bool createDirectoryIfNotExists(const fs::path& path)
{
    if (fs::exists(path)) {
        return true;
    }

    return fs::create_directories(path);
}

fs::path combineAbsoluteWithRelativePath(const fs::path& base, const fs::path& file)
{
    assert(base.has_parent_path());
    return combinePath(base.parent_path(), file);
}

fs::path combinePath(const fs::path& base, const fs::path& file)
{
#ifndef WIN32
    auto fileStr = file.string();
    for (auto& c : fileStr) {
        if (c == '\\') {
            c = '/';
        }
    }

    //return fs::canonical(fs::absolute(file, base.parent_path()));
    return fs::absolute(fileStr, base);
#else
    return fs::canonical(fs::absolute(file, base)).make_preferred();
#endif
}
}
