#include "infra/filesystem.h"

#include <fstream>
#include <gtest/gtest.h>

namespace infra {

#ifdef INFRA_HAS_FILESYSTEM

TEST(FilesystemTest, getFullPath)
{
    auto pwd = fs::current_path();
    fs::create_directories("./test1/test");
    std::ofstream f1("test1/test/file.txt");
    ASSERT_TRUE(f1.is_open());
    f1.close();

    std::ofstream f2("./test1/test/file.db");
    ASSERT_TRUE(f2.is_open());
    f2.close();

    fs::path base(pwd / "test1/test/file.txt");
    fs::path rel("file.db");

    auto expected = (pwd / "test1/test/file.db").make_preferred();
    EXPECT_EQ(expected.string(), file::combineAbsoluteWithRelativePath(base, rel).string());
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

    fs::path base(pwd / "test1\\test\\file.txt");
    fs::path rel("file.db");

    rel.make_preferred();

    auto expected = (pwd / "test1/test/file.db").make_preferred();
    EXPECT_EQ(expected.string(), file::combineAbsoluteWithRelativePath(base, rel).string());
    fs::remove_all("./test1/test");
}

#endif

}
