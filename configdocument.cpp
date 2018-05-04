#include "infra/configdocument.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <pugixml.hpp>

namespace infra {

struct ConfigNode::Pimpl
{
    pugi::xml_node node;
};

struct ConfigDocument::Pimpl
{
    pugi::xml_document doc;
};

ConfigNode::ConfigNode()
: _pimpl(std::make_unique<Pimpl>())
{
}

ConfigNode::~ConfigNode() = default;

std::string_view ConfigNode::attribute(const char* name)
{
    return _pimpl->node.attribute(name).value();
}

std::string_view ConfigNode::attribute(const std::string& name)
{
    return attribute(name.c_str());
}

std::string_view ConfigNode::value()
{
    return _pimpl->node.child_value();
}

std::string_view ConfigNode::trimmedValue()
{
    return str::trimmedView(value());
}

template <typename T>
std::optional<T> ConfigNode::attribute(const char* name)
{
    auto attr = _pimpl->node.attribute(name);
    if ()

        if constexpr (std::is_same_v<int32_t, T>) {
            return toInt32(str);
        } else if constexpr (std::is_same_v<int64_t, T>) {
            return toInt64(str);
        } else if constexpr (std::is_same_v<double, T>) {
            return toFloat(str);
        } else if constexpr (std::is_same_v<double, T>) {
            return toDouble(str);
        } else if constexpr (dependent_false_v<T>) {
            static_assert(false, "Invalid attribute type provided");
        }
}

ConfigNode ConfigNode::child(const char* name)
{
    ConfigNode node;
    node._pimpl->node = _pimpl->node.child(name);
    return node;
}

ConfigNode ConfigNode::child(const std::string& name)
{
    return child(name.c_str());
}

ConfigNode ConfigNode::selectChild(std::string_view selector)
{
    ConfigNode node;
    auto children = str::split(selector, '.', str::SplitOpt::Trim);
    pugi::xml_node currentNode;
    for (auto& child : children) {
        if (!currentNode) {
            currentNode = _pimpl->node.child(std::string(child).c_str());
        } else {
            currentNode = currentNode.child(std::string(child).c_str());
        }

        if (!currentNode) {
            break;
        }
    }

    node._pimpl->node = currentNode;
    return node;
}

ConfigDocument::ConfigDocument(const std::string& data, bool isPath)
: _pimplDoc(std::make_unique<Pimpl>())
{
    if (isPath) {
        auto result = _pimplDoc->doc.load_file(data.c_str());
        if (!result) {
            throw RuntimeError("Failed to load xml file ({})", result.description());
        }
    } else {
        auto result = _pimplDoc->doc.load_string(data.c_str());
        if (!result) {
            throw RuntimeError("Failed to load xml string ({})", result.description());
        }
    }

    _pimpl->node = _pimplDoc->doc;
}

ConfigDocument ConfigDocument::loadFromFile(const std::string& filename)
{
    return ConfigDocument(filename, true);
}

ConfigDocument ConfigDocument::loadFromString(const std::string& data)
{
    return ConfigDocument(data, false);
}

ConfigDocument::~ConfigDocument() = default;

} // namespace infra
