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
    auto par = base.parent_path() / fileStr;

    return fs::canonical(par);
#else
    return fs::canonical((base.parent_path() / file).make_preferred());
#endif
}
}
