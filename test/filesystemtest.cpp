#include "infra/filesystem.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf {

#ifdef INFRA_HAS_FILESYSTEM

TEST_CASE("FilesystemTest.getFullPath")
{
    auto pwd = fs::current_path();
    fs::create_directories("./test1/test");
    std::ofstream f1("test1/test/file.txt");
    REQUIRE(f1.is_open());
    f1.close();

    std::ofstream f2("./test1/test/file.db");
    REQUIRE(f2.is_open());
    f2.close();

    fs::path base(pwd / "test1/test/file.txt");
    fs::path rel("file.db");

    auto expected = (pwd / "test1/test/file.db").make_preferred();
    CHECK(expected.string() == file::combine_absolute_with_relative_path(base, rel).string());
    fs::remove_all("./test1/test");
}

TEST_CASE("FilesystemTest.getFullPathBackslash")
{
    auto pwd = fs::current_path();
    fs::create_directories("./test1/test");
    std::ofstream f1("./test1/test/file.txt");
    REQUIRE(f1.is_open());
    f1.close();

    std::ofstream f2("./test1/test/file.db");
    REQUIRE(f2.is_open());
    f2.close();

    fs::path base(pwd / "test1\\test\\file.txt");
    fs::path rel("file.db");

    rel.make_preferred();

    auto expected = (pwd / "test1/test/file.db").make_preferred();
    CHECK_MESSAGE(expected.string() == file::combine_absolute_with_relative_path(base, rel).string(), fmt::format("{} {}", base, base.make_preferred()));
    fs::remove_all("./test1/test");
}

#endif

}
