#include "infra/xmldocument.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace inf::test {

using namespace testing;

static const char* s_xmlData = R"("
    <xml>
        <node1 attr1="5">
            <subnode1 attr1="6"/>
        </node1>
        <node2 attr2="45">
            aValue
        </node2>
        <node3>some text</node3>
        <node3>some text 2</node3>
    </xml>
")";

class ConfigReaderTest : public Test
{
protected:
    ConfigReaderTest()
    {
        doc = XmlDocument::load_from_string(s_xmlData);
    }

    XmlDocument doc;
};

TEST_F(ConfigReaderTest, readFromString)
{
    EXPECT_EQ("5", doc.child("xml").child("node1").attribute("attr1"));
    EXPECT_EQ("6", doc.child("xml").child("node1").child("subnode1").attribute("attr1"));

    EXPECT_EQ("5", doc.select_child("xml.node1").attribute("attr1"));
    EXPECT_EQ("6", doc.select_child("xml.node1.subnode1").attribute("attr1"));

    auto xmlNode = doc.child("xml");
    EXPECT_EQ("5", xmlNode.select_child("node1").attribute("attr1"));
    EXPECT_EQ("6", xmlNode.select_child("node1.subnode1").attribute("attr1"));

    EXPECT_EQ("\n            aValue\n        ", xmlNode.select_child("node2").value());
    EXPECT_EQ("aValue", xmlNode.select_child("node2").trimmed_value());
    EXPECT_EQ("some text", xmlNode.select_child("node3").value());
}

TEST_F(ConfigReaderTest, iterateChildren)
{
    auto xmlNode = doc.child("xml");

    std::vector<std::string> childNames;
    for (const auto& child : xmlNode.children()) {
        childNames.push_back(std::string(child.name()));
    }

    EXPECT_THAT(childNames, ContainerEq(std::vector<std::string>{"node1", "node2", "node3", "node3"}));
}

TEST_F(ConfigReaderTest, iterateNamedChildren)
{
    auto xmlNode = doc.child("xml");

    std::vector<std::string> childNames;
    for (const auto& child : xmlNode.children("node1")) {
        childNames.push_back(std::string(child.name()));
    }

    EXPECT_THAT(childNames, ContainerEq(std::vector<std::string>{"node1"}));

    childNames.clear();
    for (const auto& child : xmlNode.children("node3")) {
        childNames.push_back(std::string(child.name()));
    }

    EXPECT_THAT(childNames, ContainerEq(std::vector<std::string>{"node3", "node3"}));
}

TEST_F(ConfigReaderTest, rootNode)
{
    EXPECT_EQ("xml", doc.rootNode().name());
}

TEST_F(ConfigReaderTest, rootChildren)
{
    std::vector<std::string> names;
    for (auto& child : doc.children()) {
        names.emplace_back(child.name());
    }

    EXPECT_THAT(names, ContainerEq(std::vector<std::string>{"xml"}));
}

TEST_F(ConfigReaderTest, nonExistingNodes)
{
    auto child = doc.child("lmx");
    EXPECT_FALSE(child);
    EXPECT_TRUE(child.name().empty());
    EXPECT_TRUE(child.attribute("").empty());

    child = doc.child("lmx").child("notpresent").child("nope");
    EXPECT_FALSE(child);
    EXPECT_TRUE(child.name().empty());
    EXPECT_TRUE(child.attribute("").empty());
}
}
