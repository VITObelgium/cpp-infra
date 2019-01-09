#include "gridmapper.hpp"

#include "infra/exception.h"
#include "infra/log.h"
#include "infra/string.h"

namespace rio {

using namespace inf;

std::unordered_map<int64_t, inf::Cell> read_mapping_file(const fs::path& path)
{
    std::unordered_map<int64_t, inf::Cell> result;

    // now import the griddef file and setup the mapping arrays
    auto mappingContents = file::read_as_text(path);
    auto lines           = str::split_view(mappingContents, '\n', str::SplitOpt::Trim);

    // remove the header
    std::swap(lines.front(), lines.back());
    lines.pop_back();

    for (auto& line : lines) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto items = str::split_view(line, " \t;,", str::StrTokFlags);
        if (items.size() != 3) {
            Log::warn("Invalid mapping line: {}", line);
            continue;
        }

        auto id  = str::to_int64(items[0]);
        auto row = str::to_int32(items[1]);
        auto col = str::to_int32(items[2]);

        if (id.has_value() && row.has_value() && col.has_value()) {
            result.emplace(id.value(), inf::Cell(row.value() - 1, col.value() - 1));
        } else {
            Log::warn("Invalid mapping line: {}", line);
        }
    }

    return result;
}
}
