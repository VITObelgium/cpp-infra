#include "infra/configdocument.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <pugixml.hpp>
#include <variant>

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
    std::variant<pugi::xml_node_iterator, pugi::xml_named_node_iterator> iter;
    ConfigNode node;
};

ConfigNode::ConfigNode()
: _pimpl(std::make_unique<Pimpl>())
{
}

ConfigNode::~ConfigNode() = default;

ConfigNode::ConfigNode(const ConfigNode& other)
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->node = other._pimpl->node;
}

ConfigNode::ConfigNode(ConfigNode&&) = default;

ConfigNode& ConfigNode::operator=(const ConfigNode& other)
{
    _pimpl->node = other._pimpl->node;
    return *this;
}

ConfigNode& ConfigNode::operator=(ConfigNode&&) = default;

std::string_view ConfigNode::attribute(const char* name) const
{
    return _pimpl->node.attribute(name).value();
}

std::string_view ConfigNode::attribute(const std::string& name) const
{
    return attribute(name.c_str());
}

std::string ConfigNode::attribute(const char* name, std::string_view valueIfNotPresent) const
{
    auto value = attribute(name);
    if (value.empty()) {
        return std::string(valueIfNotPresent);
    }

    return std::string(value);
}

std::string ConfigNode::attribute(const std::string& name, std::string_view valueIfNotPresent) const
{
    return attribute(name.c_str(), valueIfNotPresent);
}

std::string_view ConfigNode::name() const
{
    return _pimpl->node.name();
}

std::string_view ConfigNode::value() const
{
    return _pimpl->node.child_value();
}

std::string_view ConfigNode::trimmedValue() const
{
    return str::trimmedView(value());
}

std::string ConfigNode::value(std::string_view valueIfNotPresent) const
{
    std::string name = _pimpl->node.name();
    if (name.empty()) {
        name = valueIfNotPresent;
    }

    return name;
}

std::string ConfigNode::trimmedValue(std::string_view valueIfNotPresent) const
{
    return str::trim(value(valueIfNotPresent));
}

template <typename T>
std::optional<T> ConfigNode::value() const
{
    return str::toNumeric<T>(_pimpl->node.child_value());
}

template std::optional<double> ConfigNode::value<double>() const;
template std::optional<float> ConfigNode::value<float>() const;
template std::optional<int32_t> ConfigNode::value<int32_t>() const;
template std::optional<int64_t> ConfigNode::value<int64_t>() const;

bool ConfigNode::operator!() const noexcept
{
    return !_pimpl->node;
}

ConfigNode::operator bool() const noexcept
{
    return static_cast<bool>(_pimpl->node);
}

template <typename T>
std::optional<T> ConfigNode::attribute(const char* name) const
{
    return str::toNumeric<T>(_pimpl->node.attribute(name).value());
}

template std::optional<double> ConfigNode::attribute<double>(const char* name) const;
template std::optional<float> ConfigNode::attribute<float>(const char* name) const;
template std::optional<int32_t> ConfigNode::attribute<int32_t>(const char* name) const;
template std::optional<int64_t> ConfigNode::attribute<int64_t>(const char* name) const;

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

ConfigObjectRange<ConfigNodeIterator> ConfigNode::children() const
{
    ConfigNodeIterator begin, end;
    begin._pimpl       = std::make_unique<ConfigNodeIterator::Pimpl>();
    end._pimpl         = std::make_unique<ConfigNodeIterator::Pimpl>();
    begin._pimpl->iter = _pimpl->node.begin();
    end._pimpl->iter   = _pimpl->node.end();
    return ConfigObjectRange<ConfigNodeIterator>(std::move(begin), std::move(end));
}

ConfigObjectRange<ConfigNodeIterator> ConfigNode::children(const char* name) const
{
    auto range = _pimpl->node.children(name);

    ConfigNodeIterator begin, end;
    begin._pimpl       = std::make_unique<ConfigNodeIterator::Pimpl>();
    end._pimpl         = std::make_unique<ConfigNodeIterator::Pimpl>();
    begin._pimpl->iter = range.begin();
    end._pimpl->iter   = range.end();
    return ConfigObjectRange<ConfigNodeIterator>(std::move(begin), std::move(end));
}

ConfigObjectRange<ConfigNodeIterator> ConfigNode::children(const std::string& name) const
{
    return children(name.c_str());
}

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

ConfigNode ConfigDocument::rootNode() const
{
    ConfigNode root;
    root._pimpl->node = _pimplDoc->doc.root().first_child();
    return root;
}

ConfigNodeIterator::ConfigNodeIterator(ConfigNodeIterator&&) = default;
ConfigNodeIterator::ConfigNodeIterator(const ConfigNodeIterator& other)
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->iter = other._pimpl->iter;
}

ConfigNodeIterator::~ConfigNodeIterator()       = default;
ConfigNodeIterator& ConfigNodeIterator::operator=(ConfigNodeIterator&& other) = default;

ConfigNodeIterator& ConfigNodeIterator::operator++()
{
    std::visit([](auto& iter) { ++iter; }, _pimpl->iter);
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

ConfigNode& ConfigNodeIterator::operator*()
{
    return std::visit([this](auto& iter) -> ConfigNode& {
        _pimpl->node._pimpl->node = *iter;
        return _pimpl->node;
    },
        _pimpl->iter);
}

const ConfigNode& ConfigNodeIterator::operator*() const
{
    return std::visit([this](auto& iter) -> const ConfigNode& {
        _pimpl->node._pimpl->node = *iter;
        return _pimpl->node;
    },
        _pimpl->iter);
}

} // namespace infra
