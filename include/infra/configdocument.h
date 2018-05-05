#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace infra {

class ConfigNodeIterator;

template <typename Iterator>
class ConfigObjectRange
{
public:
    typedef Iterator const_iterator;
    typedef Iterator iterator;

    ConfigObjectRange(Iterator b, Iterator e)
    : _begin(std::move(b))
    , _end(std::move(e))
    {
    }

    ConfigObjectRange(ConfigObjectRange&&) = default;
    ConfigObjectRange& operator=(ConfigObjectRange&&) = default;

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

class ConfigNode
{
public:
    ConfigNode();
    ~ConfigNode();

    ConfigNode(const ConfigNode&);
    ConfigNode(ConfigNode&&);
    ConfigNode& operator=(const ConfigNode&);
    ConfigNode& operator=(ConfigNode&&);

    ConfigNode child(const char* name) const;
    ConfigNode child(const std::string& name) const;

    ConfigObjectRange<ConfigNodeIterator> children() const;
    ConfigObjectRange<ConfigNodeIterator> children(const char* name) const;
    ConfigObjectRange<ConfigNodeIterator> children(const std::string& name) const;

    ConfigNode selectChild(std::string_view selector) const;

    std::string_view attribute(const char* name) const;
    std::string_view attribute(const std::string& name) const;

    std::string_view name() const;
    std::string_view value() const;
    std::string_view trimmedValue() const;

    template <typename T>
    std::optional<T> value() const;

    template <typename T>
    std::optional<T> attribute(const char* name) const;
    template <typename T>
    std::optional<T> attribute(const std::string& name) const
    {
        return attribute<T>(name.c_str());
    }

    bool operator!() const noexcept;
    operator bool() const noexcept;

protected:
    friend class ConfigDocument;
    friend class ConfigNodeIterator;

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

class ConfigNodeIterator
{
public:
    ConfigNodeIterator() = default;
    ConfigNodeIterator(const ConfigNodeIterator&);
    ConfigNodeIterator(ConfigNodeIterator&&);
    ~ConfigNodeIterator();

    ConfigNodeIterator& operator++();
    ConfigNodeIterator& operator=(ConfigNodeIterator&& other);
    bool operator==(const ConfigNodeIterator& other) const;
    bool operator!=(const ConfigNodeIterator& other) const;

    ConfigNode& operator*();
    const ConfigNode& operator*() const;

private:
    friend class ConfigNode;

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

//ConfigNodeIterator begin(const ConfigNode& line);
//ConfigNodeIterator begin(ConfigNode&& line);
//ConfigNodeIterator end(const ConfigNode&);

class ConfigDocument : public ConfigNode
{
public:
    static ConfigDocument loadFromFile(const std::string& filename);
    static ConfigDocument loadFromString(const std::string& data);

    ConfigDocument();
    ConfigDocument(const ConfigDocument&) = delete;
    ConfigDocument& operator=(const ConfigDocument&) = delete;
    ConfigDocument(ConfigDocument&&);
    ConfigDocument& operator=(ConfigDocument&&);

    ~ConfigDocument();

    ConfigNode rootNode() const;

private:
    ConfigDocument(const std::string& data, bool isPath);

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimplDoc;
};

} // namespace infra
