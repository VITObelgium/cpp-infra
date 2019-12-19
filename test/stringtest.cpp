#include "infra/string.h"
#include "infra/cast.h"

#include <doctest/doctest.h>

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

using namespace doctest;
using namespace std::string_literals;

TEST_CASE("StringTest.LowerCase")
{
    string testString = "TESTSTRING";
    str::lowercase_in_place(testString);
    CHECK("teststring" == testString);

    testString = "teststring";
    str::lowercase_in_place(testString);
    CHECK("teststring" == testString);

    testString = "~!@#$%^&*()_1234567890-";
    str::lowercase_in_place(testString);
    CHECK("~!@#$%^&*()_1234567890-" == testString);

    testString = "H_ell_O";
    str::lowercase_in_place(testString);
    CHECK("h_ell_o" == testString);

    CHECK(std::string("hello") == str::lowercase("HeLLo"));
}

TEST_CASE("StringTest.UpperCase")
{
    string testString = "teststring";
    str::uppercase_in_place(testString);
    CHECK("TESTSTRING" == testString);

    testString = "teststring";
    str::uppercase_in_place(testString);
    CHECK("TESTSTRING" == testString);

    testString = "~!@#$%^&*()_1234567890-";
    str::uppercase_in_place(testString);
    CHECK("~!@#$%^&*()_1234567890-" == testString);

    testString = "H_ell_O";
    str::uppercase_in_place(testString);
    CHECK("H_ELL_O" == testString);

    CHECK(std::string("HELLO") == str::uppercase("HeLLo"));
}

TEST_CASE("StringTest.Iequals")
{
    CHECK(str::iequals("TEST", "test"));
    CHECK(str::iequals("TEST", "TEST"));
    CHECK(str::iequals("test", "test"));
    CHECK(str::iequals("", ""));

    CHECK_FALSE(str::iequals("TEST", "TOST"));
    CHECK_FALSE(str::iequals("TEST", ""));
    CHECK_FALSE(str::iequals("", "TEST"));
}

TEST_CASE("StringTest.Replace_in_place")
{
    string testString = "abcaabbabbab";
    str::replace_in_place(testString, "ab", "a");
    CHECK("acaababa" == testString);
    str::replace_in_place(testString, "a", "z");
    CHECK("zczzbzbz" == testString);

    testString = "stringstringstring";
    str::replace_in_place(testString, "stringstring", "string");
    CHECK("stringstring" == testString);
}

TEST_CASE("StringTest.Replace")
{
    string testString = "abcaabbabbab";
    CHECK("acaababa" == str::replace(testString, "ab", "a"));
    CHECK("zcazbzbz" == str::replace(testString, "ab", "z"));

    testString = "stringstringstring";
    CHECK("stringstring" == str::replace(testString, "stringstring", "string"));
}

TEST_CASE("StringOperationsTest.Split")
{
    string testString = "A-B-C";
    vector<string> tokenized;
    tokenized = str::split(testString, "-");
    REQUIRE(3u == tokenized.size());
    CHECK("A"s == tokenized[0]);
    CHECK("B"s == tokenized[1]);
    CHECK("C"s == tokenized[2]);

    testString = ";";
    tokenized  = str::split(testString, ";");
    REQUIRE(2u == tokenized.size());
    CHECK(""s == tokenized[0]);
    CHECK(""s == tokenized[1]);

    testString = ";";
    tokenized  = str::split(testString, ';');
    REQUIRE(2u == tokenized.size());
    CHECK(""s == tokenized[0]);
    CHECK(""s == tokenized[1]);

    testString = "A_*_B_*_C";
    tokenized  = str::split(testString, "_*_");
    REQUIRE(3u == tokenized.size());
    CHECK("A"s == tokenized[0]);
    CHECK("B"s == tokenized[1]);
    CHECK("C"s == tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::split(testString, "_**_");
    REQUIRE(1u == tokenized.size());
    CHECK("A_*_B_*_C"s == tokenized[0]);

    testString = "A_*_B_*_C";
    tokenized  = str::split(testString, "_C");
    REQUIRE(2u == tokenized.size());
    CHECK("A_*_B_*"s == tokenized[0]);
    CHECK(""s == tokenized[1]);

    testString = "";
    tokenized  = str::split(testString, ";");
    REQUIRE(1u == tokenized.size());
    CHECK(""s == tokenized[0]);

    testString = "string";
    tokenized  = str::split(testString, ";");
    REQUIRE(1u == tokenized.size());
    CHECK("string"s == tokenized[0]);

    testString = "";
    tokenized  = str::split(testString, ';');
    REQUIRE(1u == tokenized.size());
    CHECK(""s == tokenized[0]);

    testString = "A,,C";
    tokenized  = str::split(testString, ",");
    REQUIRE(3u == tokenized.size());
    CHECK("A"s == tokenized[0]);
    CHECK(""s == tokenized[1]);
    CHECK("C"s == tokenized[2]);
}

TEST_CASE("StringOperationsTest.SplitAndTrim")
{
    string testString = "A - B - C";
    vector<string> tokenized;
    tokenized = str::split(testString, "-", str::SplitOpt::Trim);
    REQUIRE(3u == tokenized.size());
    CHECK("A"s == tokenized[0]);
    CHECK("B"s == tokenized[1]);
    CHECK("C"s == tokenized[2]);
}

TEST_CASE("StringOperationsTest.SplitNoEmpty")
{
    string testString = "A,,C";
    vector<string> tokenized;
    tokenized = str::split(testString, ",", str::SplitOpt::NoEmpty);
    REQUIRE(2u == tokenized.size());
    CHECK("A"s == tokenized[0]);
    CHECK("C"s == tokenized[1]);
}

TEST_CASE("StringOperationsTest.SplitAndTrimNoEmpty")
{
    string testString        = " A,  ,C  ";
    vector<string> tokenized = str::split(testString, ',', str::SplitOpt::Trim | str::SplitOpt::NoEmpty);
    REQUIRE(2u == tokenized.size());
    CHECK("A"s == tokenized[0]);
    CHECK("C"s == tokenized[1]);

    testString = " A , , ,  C  ";
    tokenized  = str::split(testString, ", ", str::SplitOpt::Trim | str::SplitOpt::NoEmpty);
    REQUIRE(2u == tokenized.size());
    CHECK("A"s == tokenized[0]);
    CHECK("C"s == tokenized[1]);
}

TEST_CASE("StringOperationsTest.SplittedView")
{
    string testString = "A-B-C";
    vector<string_view> tokenized;
    tokenized = str::split_view(testString, '-');
    CHECK(3u == tokenized.size());
    CHECK("A" == tokenized[0]);
    CHECK("B" == tokenized[1]);
    CHECK("C" == tokenized[2]);

    testString = ";";
    tokenized  = str::split_view(testString, ';');
    REQUIRE(2u == tokenized.size());
    CHECK("" == tokenized[0]);
    CHECK("" == tokenized[1]);

    testString = ";";
    tokenized  = str::split_view(testString, ";");
    REQUIRE(2u == tokenized.size());
    CHECK("" == tokenized[0]);
    CHECK("" == tokenized[1]);

    testString = ";;";
    tokenized  = str::split_view(testString, ';');
    REQUIRE(3u == tokenized.size());
    CHECK("" == tokenized[0]);
    CHECK("" == tokenized[1]);
    CHECK("" == tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::split_view(testString, "_*_");
    REQUIRE(3u == tokenized.size());
    CHECK("A" == tokenized[0]);
    CHECK("B" == tokenized[1]);
    CHECK("C" == tokenized[2]);

    testString = "A_*_B_*_C";
    tokenized  = str::split_view(testString, "_**_");
    CHECK(1u == tokenized.size());
    CHECK("A_*_B_*_C" == tokenized[0]);

    testString = "";
    tokenized  = str::split_view(testString, ',');
    REQUIRE(1u == tokenized.size());
    CHECK("" == tokenized[0]);

    testString = "string";
    tokenized  = str::split_view(testString, ',');
    CHECK(1u == tokenized.size());
    CHECK("string" == tokenized[0]);

    testString = "";
    tokenized  = str::split_view(testString, ",");
    REQUIRE(1u == tokenized.size());
    CHECK("" == tokenized[0]);

    testString = "Line1\nLine2";
    tokenized  = str::split_view(testString, "\r\n", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    REQUIRE(2u == tokenized.size());
    CHECK("Line1" == tokenized[0]);
    CHECK("Line2" == tokenized[1]);
}

TEST_CASE("StringOperationsTest.SplittedViewDelimiterString")
{
    string testString = "A-B.C|D+E++";
    auto tokenized    = str::split_view(testString, "-.+", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    CHECK(tokenized == std::vector<std::string_view>({"A", "B", "C|D", "E", ""}));
    tokenized = str::split_view(testString, "-.+", str::SplitOpt::DelimiterIsCharacterArray);
    CHECK(tokenized == std::vector<std::string_view>({"A", "B", "C|D", "E", "", ""}));

    testString = "- This, a sample string.";
    tokenized  = str::split_view(testString, " ,.-", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    CHECK(tokenized == std::vector<std::string_view>({"", "This", "a", "sample", "string", ""}));
}

TEST_CASE("StringTest.Trim")
{
    CHECK("astring"s == str::trim("astring"));
    CHECK("a"s == str::trim("a "));
    CHECK("a"s == str::trim(" a"));
    CHECK("a a  a"s == str::trim("  a a  a "));
    CHECK("a \r\t\n a  a"s == str::trim("  \r \n\t\r\n a \r\t\n a  a \t\t\t"));
    CHECK("" == str::trim(""));
    CHECK("" == str::trim(" \r\n\t"));
}

TEST_CASE("StringTest.Trim_in_place")
{
    std::string str(" please trim me    .  ");
    str::trim_in_place(str);
    CHECK("please trim me    ." == str);

    str = std::string("please trim me.");
    str::trim_in_place(str);
    CHECK("please trim me."s == str);
}

TEST_CASE("StringTest.JoinStrings")
{
    CHECK("one,two,three" == str::join<std::vector<string>>({"one", "two", "three"}, ","));
    CHECK("one" == str::join<std::vector<string>>({"one"}, ","));
}

TEST_CASE("StringTest.JoinStringViews")
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

    CHECK("one,two,three" == str::join<std::vector<StringViewable>>({{"one"}, {"two"}, {"three"}}, ","));
}

struct Streamable
{
    int value;
};

std::ostream& operator<<(std::ostream& os, const Streamable& s)
{
    return os << s.value;
}

TEST_CASE("StringTest.JoinStreamables")
{
    static_assert(is_streamable_v<Streamable>, "Streamable should be streamable");
    static_assert(!can_cast_to_string_view_v<Streamable>, "Streamable should not be convertible to string_view");

    CHECK("1, 2, 3" == str::join<std::vector<Streamable>>({{1}, {2}, {3}}, ", "));
    CHECK("one" == str::join<std::vector<string>>({"one"}, ","));
}

TEST_CASE("StringTest.starts_with")
{
    CHECK(str::starts_with("TestOne", ""));
    CHECK(str::starts_with("TestOne", "T"));
    CHECK(str::starts_with("TestOne", "Te"));
    CHECK(str::starts_with("TestOne", "TestOn"));
    CHECK(str::starts_with("TestOne", "TestOne"));

    CHECK(str::starts_with("TestOne", std::string("TestOn")));
    CHECK(str::starts_with("TestOne", std::string("TestOne")));

    CHECK_FALSE(str::starts_with("TestOne", "es"));
    CHECK_FALSE(str::starts_with("TestOne", "t"));

    CHECK_FALSE(str::starts_with("", "."));
}

TEST_CASE("StringTest.starts_with_ignore_case")
{
    CHECK(str::starts_with_ignore_case("TestOne", ""));
    CHECK(str::starts_with_ignore_case("TestOne", "t"));
    CHECK(str::starts_with_ignore_case("TestOne", "TE"));
    CHECK(str::starts_with_ignore_case("TestOne", "TeStOn"));
    CHECK(str::starts_with_ignore_case("TestOne", "tEsToNe"));

    CHECK(str::starts_with_ignore_case("TestOne", std::string("teston")));
    CHECK(str::starts_with_ignore_case("TestOne", std::string("TESTONE")));

    CHECK_FALSE(str::starts_with_ignore_case("TestOne", "es"));
    CHECK_FALSE(str::starts_with_ignore_case("", "."));
}

TEST_CASE("StringTest.ends_with")
{
    CHECK(str::ends_with("TestOne", ""));
    CHECK(str::ends_with("TestOne", "e"));
    CHECK(str::ends_with("TestOne", "ne"));
    CHECK(str::ends_with("TestOne", "One"));
    CHECK(str::ends_with("TestOne", "TestOne"));

    CHECK_FALSE(str::ends_with("TestOne", "On"));
    CHECK_FALSE(str::ends_with("TestOne", "TestOn"));
    CHECK_FALSE(str::ends_with("TestOne", "TestOne."));

    CHECK_FALSE(str::ends_with("", "."));
}

TEST_CASE("StringTest.ends_with_ignore_case")
{
    CHECK(str::ends_with_ignore_case("TestOne", ""));
    CHECK(str::ends_with_ignore_case("TestOne", "e"));
    CHECK(str::ends_with_ignore_case("TestOne", "E"));
    CHECK(str::ends_with_ignore_case("TestOne", "ne"));
    CHECK(str::ends_with_ignore_case("TestOne", "NE"));
    CHECK(str::ends_with_ignore_case("TestOne", "One"));
    CHECK(str::ends_with_ignore_case("TestOne", "oNe"));
    CHECK(str::ends_with_ignore_case("TestOne", "TestOne"));
    CHECK(str::ends_with_ignore_case("TestOne", "testone"));

    CHECK_FALSE(str::ends_with_ignore_case("TestOne", "On"));
    CHECK_FALSE(str::ends_with_ignore_case("TestOne", "oN"));
    CHECK_FALSE(str::ends_with_ignore_case("TestOne", "TESTON"));
    CHECK_FALSE(str::ends_with_ignore_case("TestOne", "TestOne."));

    CHECK_FALSE(str::ends_with_ignore_case("", "."));
}

TEST_CASE("StringTest.SplitterTest")
{
    static const char* line = "Line 1:\t1\t2\t3\t4\t5\t10";

    std::vector<std::string_view> result;
    auto splitter = str::Splitter(line, "\t");
    std::copy(splitter.begin(), splitter.end(), std::back_inserter(result));

    CHECK(result == std::vector<std::string_view>({"Line 1:", "1", "2", "3", "4", "5", "10"}));

    splitter = str::Splitter(line, "\t");
    CHECK("Line 1:" == *splitter.begin());
    CHECK("Line 1:" == *splitter.begin());
    CHECK(splitter.begin() != ++splitter.begin());
    CHECK(std::distance(splitter.begin(), splitter.end()) == 7);
}

TEST_CASE("StringTest.SplitterTestOnlyDelimiters")
{
    static const char* line = "-.-.-";
    auto splitter           = str::Splitter(line, ".-", str::SplitOpt::DelimiterIsCharacterArray | str::SplitOpt::JoinAdjacentCharDelimeters);
    CHECK(std::distance(splitter.begin(), splitter.end()) == 2);
}

TEST_CASE("StringTest.SplitterTestNoDelimiters")
{
    static const char* line = "Blablablablablabla";
    auto splitter           = str::Splitter(line, ".-", str::SplitOpt::DelimiterIsCharacterArray);
    CHECK(std::distance(splitter.begin(), splitter.end()) == 1);
    CHECK(line == *splitter.begin());
}

TEST_CASE("StringTest.SplitterTeststarts_withDelimeters")
{
    static const char* line = "- This, a sample string.";

    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(line, " ,.-", str::StrTokFlags)) {
        result.push_back(value);
    }

    CHECK(result == std::vector<std::string_view>({"This", "a", "sample", "string"}));
}

TEST_CASE("StringTest.SplitterTestEmptyElements")
{
    static const char* line = "Line 2:\t\tv 1\tv 2\tv 3\tv 4\tv 5\tv 10";

    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(line, "\t")) {
        result.push_back(value);
    }

    CHECK(result == std::vector<std::string_view>({"Line 2:", "", "v 1", "v 2", "v 3", "v 4", "v 5", "v 10"}));
}

std::vector<std::string_view> splitterVector(std::string_view input, std::string_view delimiter)
{
    std::vector<std::string_view> result;
    for (auto& value : str::Splitter(input, delimiter)) {
        result.push_back(value);
    }

    return result;
}

TEST_CASE("StringTest.SplitterVsSplitBehavior")
{
    static const char* line = "Line 2:\t\tv 1\tv 2\tv 3\tv 4\tv 5\tv 10";

    CHECK(str::split_view(line, "\t") == splitterVector(line, "\t"));
}

TEST_CASE("StringTest.Ellipsize")
{
    static const char* line = "What a long string";

    CHECK("What a ..." == str::ellipsize(line, 10));
    CHECK("" == str::ellipsize(line, 0));
    CHECK("A" == str::ellipsize("AH", 1));
    CHECK("AH" == str::ellipsize("AHA", 2));
    CHECK("AHA" == str::ellipsize("AHA", 3));
    CHECK("..." == str::ellipsize("AHA!", 3));
    CHECK(line == str::ellipsize(line, truncate<int>(std::strlen(line))));
}
}
