#include "infra/string.h"
#include "infra/cast.h"

#include <gtest/gtest.h>

namespace std {

void PrintTo(const std::string_view& sv, ::std::ostream* os)
{
    *os << "'";
    os->write(sv.data(), sv.size());
    *os << "'";
}

} // namespace foo

namespace inf::test {

using std::string;
using std::string_view;
using std::vector;
using std::wstring;

using namespace testing;
using namespace std::string_literals;

TEST(StringTest, LowerCase)
{
    string testString = "TESTSTRING";
    str::lowercase_in_place(testString);
    EXPECT_EQ("teststring", testString);

    testString = "teststring";
    str::lowercase_in_place(testString);
    EXPECT_EQ("teststring", testString);

    testString = "~!@#$%^&*()_1234567890-";
    str::lowercase_in_place(testString);
    EXPECT_EQ("~!@#$%^&*()_1234567890-", testString);

    testString = "H_ell_O";
    str::lowercase_in_place(testString);
    EXPECT_EQ("h_ell_o", testString);

    EXPECT_EQ(std::string("hello"), str::lowercase("HeLLo"));
}

TEST(StringTest, UpperCase)
{
    string testString = "teststring";
    str::uppercase_in_place(testString);
    EXPECT_EQ("TESTSTRING", testString);

    testString = "teststring";
    str::uppercase_in_place(testString);
    EXPECT_EQ("TESTSTRING", testString);

    testString = "~!@#$%^&*()_1234567890-";
    str::uppercase_in_place(testString);
    EXPECT_EQ("~!@#$%^&*()_1234567890-", testString);

    testString = "H_ell_O";
    str::uppercase_in_place(testString);
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

TEST(StringTest, Replace_in_place)
{
    string testString = "abcaabbabbab";
    str::replace_in_place(testString, "ab", "a");
    EXPECT_EQ("acaababa", testString);
    str::replace_in_place(testString, "a", "z");
    EXPECT_EQ("zczzbzbz", testString);

    testString = "stringstringstring";
    str::replace_in_place(testString, "stringstring", "string");
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
    tokenized = str::split_view(testString, '-');
    EXPECT_EQ(3u, tokenized.size());
    EXPECT_EQ("A", tokenized[0]);
    EXPECT_EQ("B", tokenized[1]);
    EXPECT_EQ("C", tokenized[2]);

    testString = ";";
    tokenized  = str::split_view(testString, ';');
    EXPECT_EQ(2u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);
    EXPECT_EQ("", tokenized[1]);

    testString = ";";
    tokenized  = str::split_view(testString, ";");
    ASSERT_EQ(2u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);
    EXPECT_EQ("", tokenized[1]);

    testString = ";;";
    tokenized  = str::split_view(testString, ';');
    EXPECT_EQ(3u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);
    EXPECT_EQ("", tokenized[1]);
    EXPECT_EQ("", tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::split_view(testString, "_*_");
    EXPECT_EQ(3u, tokenized.size());
    EXPECT_EQ("A", tokenized[0]);
    EXPECT_EQ("B", tokenized[1]);
    EXPECT_EQ("C", tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::split_view(testString, "_**_");
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("A_*_B_*_C", tokenized[0]);

    testString = "";
    tokenized  = str::split_view(testString, ',');
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);

    testString = "string";
    tokenized  = str::split_view(testString, ',');
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("string", tokenized[0]);

    testString = "";
    tokenized  = str::split_view(testString, ",");
    EXPECT_EQ(1u, tokenized.size());
    EXPECT_EQ("", tokenized[0]);

    testString = "Line1\nLine2";
    tokenized  = str::split_view(testString, "\r\n", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    EXPECT_EQ(2u, tokenized.size());
    EXPECT_EQ("Line1", tokenized[0]);
    EXPECT_EQ("Line2", tokenized[1]);
}

TEST(StringOperationsTest, SplittedViewDelimiterString)
{
    string testString = "A-B.C|D+E++";
    auto tokenized    = str::split_view(testString, "-.+", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    EXPECT_TRUE(tokenized == std::vector<std::string_view>({"A", "B", "C|D", "E", ""}));
    tokenized = str::split_view(testString, "-.+", str::SplitOpt::DelimiterIsCharacterArray);
    EXPECT_TRUE(tokenized == std::vector<std::string_view>({"A", "B", "C|D", "E", "", ""}));

    testString = "- This, a sample string.";
    tokenized  = str::split_view(testString, " ,.-", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    EXPECT_TRUE(tokenized == std::vector<std::string_view>({"", "This", "a", "sample", "string", ""}));
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

TEST(StringTest, Trim_in_place)
{
    std::string str(" please trim me    .  ");
    str::trim_in_place(str);
    EXPECT_EQ("please trim me    ."s, str);

    str = std::string("please trim me.");
    str::trim_in_place(str);
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

TEST(StringTest, starts_with)
{
    EXPECT_TRUE(str::starts_with("TestOne", ""));
    EXPECT_TRUE(str::starts_with("TestOne", "T"));
    EXPECT_TRUE(str::starts_with("TestOne", "Te"));
    EXPECT_TRUE(str::starts_with("TestOne", "TestOn"));
    EXPECT_TRUE(str::starts_with("TestOne", "TestOne"));

    EXPECT_TRUE(str::starts_with("TestOne", std::string("TestOn")));
    EXPECT_TRUE(str::starts_with("TestOne", std::string("TestOne")));

    EXPECT_FALSE(str::starts_with("TestOne", "es"));
    EXPECT_FALSE(str::starts_with("TestOne", "t"));

    EXPECT_FALSE(str::starts_with("", "."));
}

TEST(StringTest, starts_with_ignore_case)
{
    EXPECT_TRUE(str::starts_with_ignore_case("TestOne", ""));
    EXPECT_TRUE(str::starts_with_ignore_case("TestOne", "t"));
    EXPECT_TRUE(str::starts_with_ignore_case("TestOne", "TE"));
    EXPECT_TRUE(str::starts_with_ignore_case("TestOne", "TeStOn"));
    EXPECT_TRUE(str::starts_with_ignore_case("TestOne", "tEsToNe"));

    EXPECT_TRUE(str::starts_with_ignore_case("TestOne", std::string("teston")));
    EXPECT_TRUE(str::starts_with_ignore_case("TestOne", std::string("TESTONE")));

    EXPECT_FALSE(str::starts_with_ignore_case("TestOne", "es"));
    EXPECT_FALSE(str::starts_with_ignore_case("", "."));
}

TEST(StringTest, ends_with)
{
    EXPECT_TRUE(str::ends_with("TestOne", ""));
    EXPECT_TRUE(str::ends_with("TestOne", "e"));
    EXPECT_TRUE(str::ends_with("TestOne", "ne"));
    EXPECT_TRUE(str::ends_with("TestOne", "One"));
    EXPECT_TRUE(str::ends_with("TestOne", "TestOne"));

    EXPECT_FALSE(str::ends_with("TestOne", "On"));
    EXPECT_FALSE(str::ends_with("TestOne", "TestOn"));
    EXPECT_FALSE(str::ends_with("TestOne", "TestOne."));

    EXPECT_FALSE(str::ends_with("", "."));
}

TEST(StringTest, ends_with_ignore_case)
{
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", ""));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "e"));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "E"));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "ne"));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "NE"));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "One"));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "oNe"));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "TestOne"));
    EXPECT_TRUE(str::ends_with_ignore_case("TestOne", "testone"));

    EXPECT_FALSE(str::ends_with_ignore_case("TestOne", "On"));
    EXPECT_FALSE(str::ends_with_ignore_case("TestOne", "oN"));
    EXPECT_FALSE(str::ends_with_ignore_case("TestOne", "TESTON"));
    EXPECT_FALSE(str::ends_with_ignore_case("TestOne", "TestOne."));

    EXPECT_FALSE(str::ends_with_ignore_case("", "."));
}

TEST(StringTest, SplitterTest)
{
    static const char* line = "Line 1:\t1\t2\t3\t4\t5\t10";

    std::vector<std::string_view> result;
    auto splitter = str::Splitter(line, "\t");
    std::copy(splitter.begin(), splitter.end(), std::back_inserter(result));

    EXPECT_TRUE(result == std::vector<std::string_view>({"Line 1:", "1", "2", "3", "4", "5", "10"}));

    splitter = str::Splitter(line, "\t");
    EXPECT_EQ("Line 1:", *splitter.begin());
    EXPECT_EQ("Line 1:", *splitter.begin());
    EXPECT_NE(splitter.begin(), ++splitter.begin());
    EXPECT_EQ(7, std::distance(splitter.begin(), splitter.end()));
}

TEST(StringTest, SplitterTestOnlyDelimiters)
{
    static const char* line = "-.-.-";
    auto splitter           = str::Splitter(line, ".-", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    EXPECT_EQ(2, std::distance(splitter.begin(), splitter.end()));
}

TEST(StringTest, SplitterTestNoDelimiters)
{
    static const char* line = "Blablablablablabla";
    auto splitter           = str::Splitter(line, ".-", str::SplitOpt::DelimiterIsCharacterArray);
    EXPECT_EQ(1, std::distance(splitter.begin(), splitter.end()));
    EXPECT_EQ(line, *splitter.begin());
}

TEST(StringTest, SplitterTeststarts_withDelimeters)
{
    static const char* line = "- This, a sample string.";

    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(line, " ,.-", str::StrTokFlags)) {
        result.push_back(value);
    }

    EXPECT_TRUE(result == std::vector<std::string_view>({"This", "a", "sample", "string"}));
}

TEST(StringTest, SplitterTestEmptyElements)
{
    static const char* line = "Line 2:\t\tv 1\tv 2\tv 3\tv 4\tv 5\tv 10";

    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(line, "\t")) {
        result.push_back(value);
    }

    EXPECT_TRUE(result == std::vector<std::string_view>({"Line 2:", "", "v 1", "v 2", "v 3", "v 4", "v 5", "v 10"}));
}

std::vector<std::string_view> splitterVector(std::string_view input, std::string_view delimiter)
{
    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(input, delimiter)) {
        result.push_back(value);
    }

    return result;
}

TEST(StringTest, SplitterVsSplitBehavior)
{
    static const char* line = "Line 2:\t\tv 1\tv 2\tv 3\tv 4\tv 5\tv 10";

    EXPECT_TRUE(str::split_view(line, "\t") == splitterVector(line, "\t"));
}

TEST(StringTest, Ellipsize)
{
    static const char* line = "What a long string";

    EXPECT_EQ("What a ...", str::ellipsize(line, 10));
    EXPECT_EQ("", str::ellipsize(line, 0));
    EXPECT_EQ("A", str::ellipsize("AH", 1));
    EXPECT_EQ("AH", str::ellipsize("AHA", 2));
    EXPECT_EQ("AHA", str::ellipsize("AHA", 3));
    EXPECT_EQ("...", str::ellipsize("AHA!", 3));
    EXPECT_EQ(line, str::ellipsize(line, truncate<int>(std::strlen(line))));
}
}
