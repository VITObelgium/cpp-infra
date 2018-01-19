#include "infra/filesystem.h"

namespace infra {

bool createDirectoryIfNotExists(const fs::path& path)
{
    if (fs::exists(path)) {
        return true;
    }

    return fs::create_directories(path);
}

fs::path getFullPath(const fs::path& base, const fs::path& file)
{
    assert(base.has_parent_path());

#ifndef WIN32
    auto fileStr = file.string();
    for (auto& c : fileStr) {
        if (c == '\\') {
            c = '/';
        }
    }

    return fs::canonical(fs::absolute(file, base.parent_path()));
#else
    return fs::canonical(fs::absolute(file, base.parent_path())).make_preferred();
#endif
}
}
