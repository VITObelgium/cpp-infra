#pragma once

#include "infra/filesystem.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace inf {

class XmlNodeIterator;

template <typename Iterator>
class XmlNodeRange
{
public:
    typedef Iterator const_iterator;
    typedef Iterator iterator;

    XmlNodeRange(Iterator b, Iterator e)
    : _begin(std::move(b))
    , _end(std::move(e))
    {
    }

    XmlNodeRange(XmlNodeRange&&) = default;
    XmlNodeRange& operator=(XmlNodeRange&&) = default;

    const_iterator begin() const
    {
        return _begin;
    }

    const_iterator end() const
    {
        return _end;
    }

    iterator begin()
    {
        return _begin;
    }

    iterator end()
    {
        return _end;
    }

private:
    Iterator _begin, _end;
};

class XmlNode
{
public:
    XmlNode();
    ~XmlNode();

    XmlNode(const XmlNode&);
    XmlNode(XmlNode&&);
    XmlNode& operator=(const XmlNode&);
    XmlNode& operator=(XmlNode&&);

    XmlNode child(const char* name) const;
    XmlNode child(const std::string& name) const;

    XmlNodeRange<XmlNodeIterator> children() const;
    XmlNodeRange<XmlNodeIterator> children(const char* name) const;
    XmlNodeRange<XmlNodeIterator> children(const std::string& name) const;

    /*!
     * Obtain a subchild using a selector
     * e.g. Obtain the inner node using the selector 'xml.node.inner':
     * <xml>
     *   <node>
     *     <inner>value</inner>
     *   </node>
     * </xml>
     *
     * assert("value" == rootNode.select_child('node.inner').value())
     **/
    XmlNode select_child(std::string_view selector) const;

    std::string_view attribute(const char* name) const;
    std::string_view attribute(const std::string& name) const;

    std::optional<std::string_view> optional_attribute(const char* name) const;
    std::optional<std::string_view> optional_attribute(const std::string& name) const;

    std::string_view name() const;
    std::string_view value() const;
    std::string_view trimmed_value() const;

    std::optional<std::string_view> optional_value() const;
    std::optional<std::string_view> optional_trimmed_value() const;

    template <typename T>
    std::optional<T> value() const;

    template <typename T>
    std::optional<T> attribute(const char* name) const;
    template <typename T>
    std::optional<T> attribute(const std::string& name) const
    {
        return attribute<T>(name.c_str());
    }

    void print(std::ostream& os) const;
    std::string to_string() const;

    bool operator!() const noexcept;
    operator bool() const noexcept;

protected:
    friend class XmlDocument;
    friend class XmlNodeIterator;

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

extern template std::optional<double> XmlNode::value<double>() const;
extern template std::optional<float> XmlNode::value<float>() const;
extern template std::optional<int32_t> XmlNode::value<int32_t>() const;
extern template std::optional<int64_t> XmlNode::value<int64_t>() const;

extern template std::optional<double> XmlNode::attribute<double>(const char* name) const;
extern template std::optional<float> XmlNode::attribute<float>(const char* name) const;
extern template std::optional<int32_t> XmlNode::attribute<int32_t>(const char* name) const;
extern template std::optional<int64_t> XmlNode::attribute<int64_t>(const char* name) const;

class XmlNodeIterator
{
public:
    XmlNodeIterator() = default;
    XmlNodeIterator(const XmlNodeIterator&);
    XmlNodeIterator(XmlNodeIterator&&);
    ~XmlNodeIterator();

    XmlNodeIterator& operator++();
    XmlNodeIterator& operator=(XmlNodeIterator&& other);
    bool operator==(const XmlNodeIterator& other) const;
    bool operator!=(const XmlNodeIterator& other) const;

    XmlNode& operator*();
    const XmlNode& operator*() const;

private:
    friend class XmlNode;

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

class XmlDocument : public XmlNode
{
public:
    static XmlDocument load_from_file(const fs::path& filename);
    static XmlDocument load_from_string(const char* data);
    static XmlDocument load_from_string(std::string_view data);

    XmlDocument();
    XmlDocument(const XmlDocument&) = delete;
    XmlDocument& operator=(const XmlDocument&) = delete;
    XmlDocument(XmlDocument&&);
    XmlDocument& operator=(XmlDocument&&);

    ~XmlDocument();

    XmlNode root_node() const;

private:
    XmlDocument(const char* data, bool isPath);
    XmlDocument(std::string_view contents);

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimplDoc;
};

} // namespace inf
