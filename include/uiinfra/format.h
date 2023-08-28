#pragma once

#include <fmt/core.h>
#include <qstring.h>

template <>
struct fmt::formatter<QString>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const QString& qstr, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "{}", qstr.toStdString());
    }
};
