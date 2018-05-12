#include "infra/string.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace std {

void PrintTo(const std::string_view& sv, ::std::ostream* os)
{
    *os << "'";
    os->write(sv.data(), sv.size());
    *os << "'";
}

} // namespace foo

namespace infra::test {

using std::string;
using std::string_view;
using std::vector;
using std::wstring;

using namespace testing;
using namespace std::string_literals;

TEST(StringTest, LowerCase)
{
    string testString = "TESTSTRING";
    str::lowercaseInPlace(testString);
    EXPECT_EQ("teststring", testString);

    testString = "teststring";
    str::lowercaseInPlace(testString);
    EXPECT_EQ("teststring", testString);

    testString = "~!@#$%^&*()_1234567890-";
    str::lowercaseInPlace(testString);
    EXPECT_EQ("~!@#$%^&*()_1234567890-", testString);

    testString = "H_ell_O";
    str::lowercaseInPlace(testString);
    EXPECT_EQ("h_ell_o", testString);

    EXPECT_EQ(std::string("hello"), str::lowercase("HeLLo"));
}

TEST(StringTest, UpperCase)
{
    string testString = "teststring";
    str::uppercaseInPlace(testString);
    EXPECT_EQ("TESTSTRING", testString);

    testString = "teststring";
    str::uppercaseInPlace(testString);
    EXPECT_EQ("TESTSTRING", testString);

    testString = "~!@#$%^&*()_1234567890-";
    str::uppercaseInPlace(testString);
    EXPECT_EQ("~!@#$%^&*()_1234567890-", testString);

    testString = "H_ell_O";
    str::uppercaseInPlace(testString);
    EXPECT_EQ("H_ELL_O", testString);

    EXPECT_EQ(std::string("HELLO"), str::uppercase("HeLLo"));
}

TEST(StringTest, Iequals)
{
    EXPECT_TRUE(str::iequals("TEST", "test"));
    EXPECT_TRUE(str::iequals("TEST", "TEST"));
    EXPECT_TRUE(str::iequals("test", "test"));
    EXPECT_TRUE(str::iequals("", ""));

    EXPECT_FALSE(str::iequals("TEST", "TOST"));
    EXPECT_FALSE(str::iequals("TEST", ""));
    EXPECT_FALSE(str::iequals("", "TEST"));
}

TEST(StringTest, ReplaceInPlace)
{
    string testString = "abcaabbabbab";
    str::replaceInPlace(testString, "ab", "a");
    EXPECT_EQ("acaababa", testString);
    str::replaceInPlace(testString, "a", "z");
    EXPECT_EQ("zczzbzbz", testString);

    testString = "stringstringstring";
    str::replaceInPlace(testString, "stringstring", "string");
    EXPECT_EQ("stringstring", testString);
}

TEST(StringTest, Replace)
{
    string testString = "abcaabbabbab";
    EXPECT_EQ("acaababa", str::replace(testString, "ab", "a"));
    EXPECT_EQ("zcazbzbz", str::replace(testString, "ab", "z"));

    testString = "stringstringstring";
    EXPECT_EQ("stringstring", str::replace(testString, "stringstring", "string"));
}

TEST(StringOperationsTest, Split)
{
    string testString = "A-B-C";
    vector<string> tokenized;
    tokenized = str::split(testString, "-");
    ASSERT_EQ(3u, tokenized.size());
    EXPECT_EQ("A"s, tokenized[0]);
    EXPECT_EQ("B"s, tokenized[1]);
    EXPECT_EQ("C"s, tokenized[2]);

    testString = ";";
    tokenized  = str::split(testString, ";");
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ(""s, tokenized[0]);
    EXPECT_EQ(""s, tokenized[1]);

    testString = ";";
    tokenized  = str::split(testString, ';');
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ(""s, tokenized[0]);
    EXPECT_EQ(""s, tokenized[1]);

    testString = "A_*_B_*_C";
    tokenized  = str::split(testString, "_*_");
    ASSERT_EQ(3u, tokenized.size());
    EXPECT_EQ("A"s, tokenized[0]);
    EXPECT_EQ("B"s, tokenized[1]);
    EXPECT_EQ("C"s, tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::split(testString, "_**_");
    ASSERT_EQ(1u, tokenized.size());
    EXPECT_EQ("A_*_B_*_C"s, tokenized[0]);

    testString = "A_*_B_*_C";
    tokenized  = str::split(testString, "_C");
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ("A_*_B_*"s, tokenized[0]);
    EXPECT_EQ(""s, tokenized[1]);

    testString = "";
    tokenized  = str::split(testString, ";");
    ASSERT_EQ(1u, tokenized.size());
    EXPECT_EQ(""s, tokenized[0]);

    testString = "string";
    tokenized  = str::split(testString, ";");
    ASSERT_EQ(1u, tokenized.size());
    EXPECT_EQ("string"s, tokenized[0]);

    testString = "";
    tokenized  = str::split(testString, ';');
    ASSERT_EQ(1u, tokenized.size());
    EXPECT_EQ(""s, tokenized[0]);

    testString = "A,,C";
    tokenized  = str::split(testString, ",");
    ASSERT_EQ(3u, tokenized.size());
    EXPECT_EQ("A"s, tokenized[0]);
    EXPECT_EQ(""s, tokenized[1]);
    EXPECT_EQ("C"s, tokenized[2]);
}

TEST(StringOperationsTest, SplitAndTrim)
{
    string testString = "A - B - C";
    vector<string> tokenized;
    tokenized = str::split(testString, "-", str::SplitOpt::Trim);
    ASSERT_EQ(3u, tokenized.size());
    EXPECT_EQ("A"s, tokenized[0]);
    EXPECT_EQ("B"s, tokenized[1]);
    EXPECT_EQ("C"s, tokenized[2]);
}

TEST(StringOperationsTest, SplitNoEmpty)
{
    string testString = "A,,C";
    vector<string> tokenized;
    tokenized = str::split(testString, ",", str::SplitOpt::NoEmpty);
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ("A"s, tokenized[0]);
    EXPECT_EQ("C"s, tokenized[1]);
}

TEST(StringOperationsTest, SplitAndTrimNoEmpty)
{
    string testString        = " A,  ,C  ";
    vector<string> tokenized = str::split(testString, ',', str::SplitOpt::Trim | str::SplitOpt::NoEmpty);
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ("A"s, tokenized[0]);
    EXPECT_EQ("C"s, tokenized[1]);

    testString = " A , , ,  C  ";
    tokenized  = str::split(testString, ", ", str::SplitOpt::Trim | str::SplitOpt::NoEmpty);
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ("A"s, tokenized[0]);
    EXPECT_EQ("C"s, tokenized[1]);
}

TEST(StringOperationsTest, SplittedView)
{
    string testString = "A-B-C";
    vector<string_view> tokenized;
    tokenized = str::splitView(testString, '-');
    EXPECT_EQ(3u, tokenized.size());
    EXPECT_EQ("A", tokenized[0]);
    EXPECT_EQ("B", tokenized[1]);
    EXPECT_EQ("C", tokenized[2]);

    testString = ";";
    tokenized  = str::splitView(testString, ';');
    EXPECT_EQ(2u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);
    EXPECT_EQ("", tokenized[1]);

    testString = ";";
    tokenized  = str::splitView(testString, ";");
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);
    EXPECT_EQ("", tokenized[1]);

    testString = ";;";
    tokenized  = str::splitView(testString, ';');
    EXPECT_EQ(3u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);
    EXPECT_EQ("", tokenized[1]);
    EXPECT_EQ("", tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::splitView(testString, "_*_");
    EXPECT_EQ(3u, tokenized.size());
    EXPECT_EQ("A", tokenized[0]);
    EXPECT_EQ("B", tokenized[1]);
    EXPECT_EQ("C", tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::splitView(testString, "_**_");
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("A_*_B_*_C", tokenized[0]);

    testString = "";
    tokenized  = str::splitView(testString, ',');
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);

    testString = "string";
    tokenized  = str::splitView(testString, ',');
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("string", tokenized[0]);

    testString = "";
    tokenized  = str::splitView(testString, ",");
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);
}

TEST(StringOperationsTest, SplittedViewDelimiterString)
{
    string testString = "A-B.C|D+E++";
    auto tokenized    = str::splitView(testString, "-.+", str::SplitOpt::DelimiterIsCharacterArray);
    EXPECT_THAT(tokenized, ContainerEq(std::vector<std::string_view>{"A", "B", "C|D", "E"}));

    testString = "- This, a sample string.";
    tokenized  = str::splitView(testString, " ,.-", str::SplitOpt::DelimiterIsCharacterArray);
    EXPECT_THAT(tokenized, ContainerEq(std::vector<std::string_view>{"This", "a", "sample", "string"}));
}

TEST(StringTest, Trim)
{
    EXPECT_EQ("astring"s, str::trim("astring"));
    EXPECT_EQ("a"s, str::trim("a "));
    EXPECT_EQ("a"s, str::trim(" a"));
    EXPECT_EQ("a a  a"s, str::trim("  a a  a "));
    EXPECT_EQ("a \r\t\n a  a"s, str::trim("  \r \n\t\r\n a \r\t\n a  a \t\t\t"));
    EXPECT_EQ("", str::trim(""));
    EXPECT_EQ("", str::trim(" \r\n\t"));
}

TEST(StringTest, TrimInPlace)
{
    std::string str(" please trim me    .  ");
    str::trimInPlace(str);
    EXPECT_EQ("please trim me    ."s, str);

    str = std::string("please trim me.");
    str::trimInPlace(str);
    EXPECT_EQ("please trim me."s, str);
}

TEST(StringTest, JoinStrings)
{
    EXPECT_EQ("one,two,three", str::join<std::vector<string>>({"one", "two", "three"}, ","));
    EXPECT_EQ("one", str::join<std::vector<string>>({"one"}, ","));
}

TEST(StringTest, JoinStringViews)
{
    struct StringViewable
    {
        operator std::string_view() const noexcept
        {
            return std::string_view(value);
        }

        std::string value;
    };

    static_assert(can_cast_to_string_view_v<StringViewable>, "StringViewable should be convertible to string_view");
    static_assert(!is_streamable_v<StringViewable>, "StringViewable should not be streamable");

    EXPECT_EQ("one,two,three", str::join<std::vector<StringViewable>>({{"one"}, {"two"}, {"three"}}, ","));
}

struct Streamable
{
    int value;
};

std::ostream& operator<<(std::ostream& os, const Streamable& s)
{
    return os << s.value;
}

TEST(StringTest, JoinStreamables)
{
    static_assert(is_streamable_v<Streamable>, "Streamable should be streamable");
    static_assert(!can_cast_to_string_view_v<Streamable>, "Streamable should not be convertible to string_view");

    EXPECT_EQ("1, 2, 3", str::join<std::vector<Streamable>>({{1}, {2}, {3}}, ", "));
    EXPECT_EQ("one", str::join<std::vector<string>>({"one"}, ","));
}

TEST(StringTest, StartsWith)
{
    EXPECT_TRUE(str::startsWith("TestOne", ""));
    EXPECT_TRUE(str::startsWith("TestOne", "T"));
    EXPECT_TRUE(str::startsWith("TestOne", "Te"));
    EXPECT_TRUE(str::startsWith("TestOne", "TestOn"));
    EXPECT_TRUE(str::startsWith("TestOne", "TestOne"));

    EXPECT_TRUE(str::startsWith("TestOne", std::string("TestOn")));
    EXPECT_TRUE(str::startsWith("TestOne", std::string("TestOne")));

    EXPECT_FALSE(str::startsWith("TestOne", "es"));
    EXPECT_FALSE(str::startsWith("TestOne", "t"));

    EXPECT_FALSE(str::startsWith("", "."));
}

TEST(StringTest, EndsWith)
{
    EXPECT_TRUE(str::endsWith("TestOne", ""));
    EXPECT_TRUE(str::endsWith("TestOne", "e"));
    EXPECT_TRUE(str::endsWith("TestOne", "ne"));
    EXPECT_TRUE(str::endsWith("TestOne", "One"));
    EXPECT_TRUE(str::endsWith("TestOne", "TestOne"));

    EXPECT_FALSE(str::endsWith("TestOne", "On"));
    EXPECT_FALSE(str::endsWith("TestOne", "TestOn"));
    EXPECT_FALSE(str::endsWith("TestOne", "TestOne."));

    EXPECT_FALSE(str::endsWith("", "."));
}

TEST(StringTest, SplitterTest)
{
    static const char* line = "Line 1:\t1\t2\t3\t4\t5\t10";

    std::vector<std::string_view> result;

    for (auto& value : str::Splitter(line, "\t")) {
        result.push_back(value);
    }

    // TODO: does not compile under msvc
    //auto splitter = str::Splitter(line, "\t");
    //std::copy(splitter.begin(), splitter.end(), std::back_inserter(result));

    EXPECT_THAT(result, ContainerEq(std::vector<std::string_view>{"Line 1:", "1", "2", "3", "4", "5", "10"}));

    auto splitter = str::Splitter(line, "\t");
    EXPECT_EQ("Line 1:", *splitter.begin());
    EXPECT_EQ("Line 1:", *splitter.begin());
    EXPECT_EQ(7, std::distance(splitter.begin(), splitter.end()));
}

TEST(StringTest, SplitterTestOnlyDelimiters)
{
    static const char* line = "-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    auto splitter           = str::Splitter(line, ".-");
    EXPECT_EQ(0, std::distance(splitter.begin(), splitter.end()));
}

TEST(StringTest, SplitterTestStartsWithDelimeters)
{
    static const char* line = "- This, a sample string.";

    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(line, " ,.-")) {
        result.push_back(value);
    }

    EXPECT_THAT(result, ContainerEq(std::vector<std::string_view>{"This", "a", "sample", "string"}));
}

TEST(StringTest, SplitterTestEmptyElements)
{
    static const char* line = "Line 2:\t\tv 1\tv 2\tv 3\tv 4\tv 5\tv 10";

    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(line, "\t")) {
        result.push_back(value);
    }

    EXPECT_THAT(result, ContainerEq(std::vector<std::string_view>{"Line 2:", "v 1", "v 2", "v 3", "v 4", "v 5", "v 10"}));
}
}
