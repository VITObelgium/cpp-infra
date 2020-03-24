#include "infra/xmldocument.h"

#include <doctest/doctest.h>
#include <vector>

namespace inf::test {

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

class ConfigReaderTest
{
protected:
    ConfigReaderTest()
    {
        doc = XmlDocument::load_from_string(s_xmlData);
    }

    XmlDocument doc;
};

TEST_CASE_FIXTURE(ConfigReaderTest, "readFromString")
{
    CHECK("5" == doc.child("xml").child("node1").attribute("attr1"));
    CHECK("6" == doc.child("xml").child("node1").child("subnode1").attribute("attr1"));

    CHECK("5" == doc.select_child("xml.node1").attribute("attr1"));
    CHECK("6" == doc.select_child("xml.node1.subnode1").attribute("attr1"));

    auto xmlNode = doc.child("xml");
    CHECK("5" == xmlNode.select_child("node1").attribute("attr1"));
    CHECK("6" == xmlNode.select_child("node1.subnode1").attribute("attr1"));

    CHECK("\n            aValue\n        " == xmlNode.select_child("node2").value());
    CHECK("aValue" == xmlNode.select_child("node2").trimmed_value());
    CHECK("some text" == xmlNode.select_child("node3").value());
}

TEST_CASE_FIXTURE(ConfigReaderTest, "iterateChildren")
{
    auto xmlNode = doc.child("xml");

    std::vector<std::string> childNames;
    for (const auto& child : xmlNode.children()) {
        childNames.push_back(std::string(child.name()));
    }

    CHECK(childNames == std::vector<std::string>({"node1", "node2", "node3", "node3"}));
}

TEST_CASE_FIXTURE(ConfigReaderTest, "iterateNamedChildren")
{
    auto xmlNode = doc.child("xml");

    std::vector<std::string> childNames;
    for (const auto& child : xmlNode.children("node1")) {
        childNames.push_back(std::string(child.name()));
    }

    CHECK(childNames == std::vector<std::string>({"node1"}));

    childNames.clear();
    for (const auto& child : xmlNode.children("node3")) {
        childNames.push_back(std::string(child.name()));
    }

    CHECK(childNames == std::vector<std::string>({"node3", "node3"}));
}

TEST_CASE_FIXTURE(ConfigReaderTest, "rootNode")
{
    CHECK("xml" == doc.root_node().name());
}

TEST_CASE_FIXTURE(ConfigReaderTest, "rootChildren")
{
    std::vector<std::string> names;
    for (auto& child : doc.children()) {
        names.emplace_back(child.name());
    }

    CHECK(names == std::vector<std::string>({"xml"}));
}

TEST_CASE_FIXTURE(ConfigReaderTest, "nonExistingNodes")
{
    auto child = doc.child("lmx");
    CHECK_FALSE(child);
    CHECK(child.name().empty());
    CHECK(child.attribute("").empty());

    child = doc.child("lmx").child("notpresent").child("nope");
    CHECK_FALSE(child);
    CHECK(child.name().empty());
    CHECK(child.attribute("").empty());
}
}
