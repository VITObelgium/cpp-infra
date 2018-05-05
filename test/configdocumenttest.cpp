#include "infra/configdocument.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace infra::test {

using namespace testing;

static const char* xmlData = R"("
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

TEST(ConfigReaderTest, readFromString)
{
    auto doc = ConfigDocument::loadFromString(xmlData);
    EXPECT_EQ("5", doc.child("xml").child("node1").attribute("attr1"));
    EXPECT_EQ("6", doc.child("xml").child("node1").child("subnode1").attribute("attr1"));

    EXPECT_EQ("5", doc.selectChild("xml.node1").attribute("attr1"));
    EXPECT_EQ("6", doc.selectChild("xml.node1.subnode1").attribute("attr1"));

    auto xmlNode = doc.child("xml");
    EXPECT_EQ("5", xmlNode.selectChild("node1").attribute("attr1"));
    EXPECT_EQ("6", xmlNode.selectChild("node1.subnode1").attribute("attr1"));

    EXPECT_EQ("\n            aValue\n        ", xmlNode.selectChild("node2").value());
    EXPECT_EQ("aValue", xmlNode.selectChild("node2").trimmedValue());
    EXPECT_EQ("some text", xmlNode.selectChild("node3").value());
}

TEST(ConfigReaderTest, iterateChildren)
{
    auto doc     = ConfigDocument::loadFromString(xmlData);
    auto xmlNode = doc.child("xml");

    std::vector<std::string> childNames;
    for (const auto& child : xmlNode.children()) {
        childNames.push_back(std::string(child.name()));
    }

    EXPECT_THAT(childNames, ContainerEq(std::vector<std::string>{"node1", "node2", "node3", "node3"}));
}

TEST(ConfigReaderTest, iterateNamedChildren)
{
    auto doc     = ConfigDocument::loadFromString(xmlData);
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
}
