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

struct ConfigNodeIterator::Pimpl
{
    pugi::xml_node_iterator iter;
};

ConfigNode::ConfigNode()
: _pimpl(std::make_unique<Pimpl>())
{
}

ConfigNode::~ConfigNode()            = default;
ConfigNode::ConfigNode(ConfigNode&&) = default;
ConfigNode& ConfigNode::operator=(ConfigNode&&) = default;

std::string_view ConfigNode::attribute(const char* name) const
{
    return _pimpl->node.attribute(name).value();
}

std::string_view ConfigNode::attribute(const std::string& name) const
{
    return attribute(name.c_str());
}

std::string_view ConfigNode::value() const
{
    return _pimpl->node.child_value();
}

std::string_view ConfigNode::trimmedValue() const
{
    return str::trimmedView(value());
}

bool ConfigNode::operator!() const
{
    return !_pimpl->node;
}

template <typename T>
std::optional<T> ConfigNode::attribute(const char* name) const
{
    auto attr = _pimpl->node.attribute(name);
    if constexpr (std::is_same_v<int32_t, T>) {
        return str::toInt32(attr.value());
    } else if constexpr (std::is_same_v<int64_t, T>) {
        return str::toInt64(attr.value());
    } else if constexpr (std::is_same_v<float, T>) {
        return str::toFloat(attr.value());
    } else if constexpr (std::is_same_v<double, T>) {
        return str::toDouble(attr.value());
    } else {
        static_assert(dependent_false_v<T>, "Invalid attribute type provided");
    }
}

ConfigNode ConfigNode::child(const char* name) const
{
    ConfigNode node;
    node._pimpl->node = _pimpl->node.child(name);
    return node;
}

ConfigNode ConfigNode::child(const std::string& name) const
{
    return child(name.c_str());
}

ConfigNodeIterator ConfigNode::children() const
{
    return ConfigNodeIterator(*this);
}

//ConfigNode ConfigNode::children(const std::string& name) const
//{
//    return children(name.c_str());
//}

ConfigNode ConfigNode::selectChild(std::string_view selector) const
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

ConfigDocument::ConfigDocument()                 = default;
ConfigDocument::~ConfigDocument()                = default;
ConfigDocument::ConfigDocument(ConfigDocument&&) = default;
ConfigDocument& ConfigDocument::operator=(ConfigDocument&&) = default;

ConfigNodeIterator::ConfigNodeIterator(const ConfigNode& node)
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->iter = node._pimpl->node.begin();
}

ConfigNodeIterator::ConfigNodeIterator(ConfigNodeIterator&&) = default;
ConfigNodeIterator::~ConfigNodeIterator()                    = default;
ConfigNodeIterator& ConfigNodeIterator::operator=(ConfigNodeIterator&& other) = default;

ConfigNodeIterator& ConfigNodeIterator::operator++()
{
    ++_pimpl->iter;
    return *this;
}

bool ConfigNodeIterator::operator==(const ConfigNodeIterator& other) const
{
    return _pimpl->iter == other._pimpl->iter;
}

bool ConfigNodeIterator::operator!=(const ConfigNodeIterator& other) const
{
    return _pimpl->iter != other._pimpl->iter;
}

ConfigNode ConfigNodeIterator::operator*()
{
    ConfigNode node;
    node._pimpl->node = *_pimpl->iter;
    return node;
}

} // namespace infra
