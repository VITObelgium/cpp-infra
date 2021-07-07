#include "infra/gdal-private.h"

#include "infra/exception.h"

namespace inf::gdal {

using namespace inf;

void throw_last_error(std::string_view msg)
{
    auto* errorMsg = CPLGetLastErrorMsg();
    if (errorMsg != nullptr && strlen(errorMsg) > 0) {
        throw RuntimeError("{}: {}", msg, errorMsg);
    } else {
        throw RuntimeError(msg);
    }
}

void check_error(CPLErr err, std::string_view msg)
{
    if (err != CE_None) {
        throw_last_error(msg);
    }
}

void check_error(OGRErr err, std::string_view msg)
{
    if (err != OGRERR_NONE) {
        throw_last_error(msg);
    }
}

CPLStringList create_string_list(std::span<const std::string> options)
{
    CPLStringList result;
    for (auto& opt : options) {
        result.AddString(opt.c_str());
    }

    return result;
}

}
