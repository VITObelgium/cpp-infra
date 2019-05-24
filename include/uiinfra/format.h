#pragma once

#include <fmt/core.h>
#include <qstring.h>

namespace fmt {
template <>
struct formatter<QString>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const QString& qstr, FormatContext& ctx)
    {
        return format_to(ctx.begin(), qstr.toStdString());
    }
};
}
