#pragma once

#include "infra/filesystem.h"

#include <doctest/doctest.h>

namespace inf::test {

inline void check_strings_equal_ignore_line_endings(std::string expected, std::string actual)
{
    str::replace_in_place(expected, "\r\n", "\n");
    str::replace_in_place(actual, "\r\n", "\n");

    CHECK(expected == actual);
}

inline void check_text_files_equal_ignore_line_endings(const fs::path& expected, const fs::path& actual)
{
    auto expectedContents = file::read_as_text(expected);
    auto actualContents   = file::read_as_text(actual);

    check_strings_equal_ignore_line_endings(expectedContents, actualContents);
}

}
