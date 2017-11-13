#include "infra/filesystem.h"

#include <fstream>
#include <gtest/gtest.h>

namespace infra {

TEST(FilesystemTest, getFullPath)
{
    auto pwd = fs::current_path();
    fs::create_directories("./test1/test");
    std::ofstream f1("./test1/test/file.txt");
    ASSERT_TRUE(f1.is_open());
    f1.close();

    std::ofstream f2("./test1/test/file.db");
    ASSERT_TRUE(f2.is_open());
    f2.close();

    fs::path base(pwd / "test1/test/file.txt");
    fs::path rel("./file.db");

    EXPECT_EQ(pwd.string() + "/test1/test/file.db", getFullPath(base, rel).string());
    fs::remove_all("./test1/test");
}

TEST(FilesystemTest, getFullPathBackslash)
{
    auto pwd = fs::current_path();
    fs::create_directories("./test1/test");
    std::ofstream f1("./test1/test/file.txt");
    ASSERT_TRUE(f1.is_open());
    f1.close();

    std::ofstream f2("./test1/test/file.db");
    ASSERT_TRUE(f2.is_open());
    f2.close();

    fs::path base(pwd / "test1/test/file.txt");
    fs::path rel(".\\file.db");

    EXPECT_EQ(pwd.string() + "/test1/test/file.db", getFullPath(base, rel).string());
    fs::remove_all("./test1/test");
}
}
