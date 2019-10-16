#include "infra/xmldocument.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <pugixml.hpp>
#include <variant>

namespace inf {

struct XmlNode::Pimpl
{
    pugi::xml_node node;
};

struct XmlDocument::Pimpl
{
    pugi::xml_document doc;
};

struct XmlNodeIterator::Pimpl
{
    std::variant<pugi::xml_node_iterator, pugi::xml_named_node_iterator> iter;
    XmlNode node;
};

XmlNode::XmlNode()
: _pimpl(std::make_unique<Pimpl>())
{
}

XmlNode::~XmlNode() = default;

XmlNode::XmlNode(const XmlNode& other)
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->node = other._pimpl->node;
}

XmlNode::XmlNode(XmlNode&&) = default;

XmlNode& XmlNode::operator=(const XmlNode& other)
{
    _pimpl->node = other._pimpl->node;
    return *this;
}

XmlNode& XmlNode::operator=(XmlNode&&) = default;

std::string_view XmlNode::attribute(const char* name) const
{
    return _pimpl->node.attribute(name).value();
}

std::string_view XmlNode::attribute(const std::string& name) const
{
    return attribute(name.c_str());
}

std::optional<std::string_view> XmlNode::optional_attribute(const char* name) const
{
    std::optional<std::string_view> result;

    auto value = attribute(name);
    if (!value.empty()) {
        result = value;
    }

    return result;
}

std::optional<std::string_view> XmlNode::optional_attribute(const std::string& name) const
{
    return optional_attribute(name.c_str());
}

std::string_view XmlNode::name() const
{
    return _pimpl->node.name();
}

std::string_view XmlNode::value() const
{
    return _pimpl->node.child_value();
}

std::string_view XmlNode::trimmed_value() const
{
    return str::trimmed_view(value());
}

std::optional<std::string_view> XmlNode::optional_value() const
{
    std::optional<std::string_view> result;

    auto nodeValue = value();
    if (!nodeValue.empty()) {
        result = nodeValue;
    }

    return result;
}

std::optional<std::string_view> XmlNode::optional_trimmed_value() const
{
    auto result = optional_value();
    if (result.has_value()) {
        result = str::trimmed_view(*result);
    }

    return result;
}

template <typename T>
std::optional<T> XmlNode::value() const
{
    return str::to_numeric<T>(_pimpl->node.child_value());
}

template std::optional<double> XmlNode::value<double>() const;
template std::optional<float> XmlNode::value<float>() const;
template std::optional<int32_t> XmlNode::value<int32_t>() const;
template std::optional<int64_t> XmlNode::value<int64_t>() const;

void XmlNode::print(std::ostream& os) const
{
    _pimpl->node.print(os);
}

std::string XmlNode::to_string() const
{
    std::ostringstream ss;
    print(ss);
    return ss.str();
}

bool XmlNode::operator!() const noexcept
{
    return !_pimpl->node;
}

XmlNode::operator bool() const noexcept
{
    return static_cast<bool>(_pimpl->node);
}

template <typename T>
std::optional<T> XmlNode::attribute(const char* name) const
{
    return str::to_numeric<T>(_pimpl->node.attribute(name).value());
}

template std::optional<double> XmlNode::attribute<double>(const char* name) const;
template std::optional<float> XmlNode::attribute<float>(const char* name) const;
template std::optional<int32_t> XmlNode::attribute<int32_t>(const char* name) const;
template std::optional<int64_t> XmlNode::attribute<int64_t>(const char* name) const;

XmlNode XmlNode::child(const char* name) const
{
    XmlNode node;
    node._pimpl->node = _pimpl->node.child(name);
    return node;
}

XmlNode XmlNode::child(const std::string& name) const
{
    return child(name.c_str());
}

XmlNodeRange<XmlNodeIterator> XmlNode::children() const
{
    XmlNodeIterator begin, end;
    begin._pimpl       = std::make_unique<XmlNodeIterator::Pimpl>();
    end._pimpl         = std::make_unique<XmlNodeIterator::Pimpl>();
    begin._pimpl->iter = _pimpl->node.begin();
    end._pimpl->iter   = _pimpl->node.end();
    return XmlNodeRange<XmlNodeIterator>(std::move(begin), std::move(end));
}

XmlNodeRange<XmlNodeIterator> XmlNode::children(const char* name) const
{
    auto range = _pimpl->node.children(name);

    XmlNodeIterator begin, end;
    begin._pimpl       = std::make_unique<XmlNodeIterator::Pimpl>();
    end._pimpl         = std::make_unique<XmlNodeIterator::Pimpl>();
    begin._pimpl->iter = range.begin();
    end._pimpl->iter   = range.end();
    return XmlNodeRange<XmlNodeIterator>(std::move(begin), std::move(end));
}

XmlNodeRange<XmlNodeIterator> XmlNode::children(const std::string& name) const
{
    return children(name.c_str());
}

XmlNode XmlNode::select_child(std::string_view selector) const
{
    XmlNode node;
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

XmlDocument::XmlDocument(const char* data, bool isPath)
: _pimplDoc(std::make_unique<Pimpl>())
{
    if (isPath) {
        auto result = _pimplDoc->doc.load_file(data);
        if (!result) {
            throw RuntimeError("Failed to load xml file ({})", result.description());
        }
    } else {
        auto result = _pimplDoc->doc.load_string(data);
        if (!result) {
            throw RuntimeError("Failed to load xml string ({})", result.description());
        }
    }

    _pimpl->node = _pimplDoc->doc;
}

XmlDocument XmlDocument::load_from_file(const fs::path& filename)
{
    return XmlDocument(filename.u8string().c_str(), true);
}

XmlDocument XmlDocument::load_from_string(const char* data)
{
    return XmlDocument(data, false);
}

XmlDocument::XmlDocument()              = default;
XmlDocument::~XmlDocument()             = default;
XmlDocument::XmlDocument(XmlDocument&&) = default;
XmlDocument& XmlDocument::operator=(XmlDocument&&) = default;

XmlNode XmlDocument::root_node() const
{
    XmlNode root;
    root._pimpl->node = _pimplDoc->doc.root().first_child();
    return root;
}

XmlNodeIterator::XmlNodeIterator(XmlNodeIterator&&) = default;
XmlNodeIterator::XmlNodeIterator(const XmlNodeIterator& other)
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->iter = other._pimpl->iter;
}

XmlNodeIterator::~XmlNodeIterator()       = default;
XmlNodeIterator& XmlNodeIterator::operator=(XmlNodeIterator&& other) = default;

XmlNodeIterator& XmlNodeIterator::operator++()
{
    std::visit([](auto& iter) { ++iter; }, _pimpl->iter);
    return *this;
}

bool XmlNodeIterator::operator==(const XmlNodeIterator& other) const
{
    return _pimpl->iter == other._pimpl->iter;
}

bool XmlNodeIterator::operator!=(const XmlNodeIterator& other) const
{
    return _pimpl->iter != other._pimpl->iter;
}

XmlNode& XmlNodeIterator::operator*()
{
    return std::visit([this](auto& iter) -> XmlNode& {
        _pimpl->node._pimpl->node = *iter;
        return _pimpl->node;
    },
        _pimpl->iter);
}

const XmlNode& XmlNodeIterator::operator*() const
{
    return std::visit([this](auto& iter) -> const XmlNode& {
        _pimpl->node._pimpl->node = *iter;
        return _pimpl->node;
    },
        _pimpl->iter);
}

} // namespace inf
