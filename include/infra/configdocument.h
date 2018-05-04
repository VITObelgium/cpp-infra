#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace infra {

class ConfigNodeIterator;

class ConfigNode
{
public:
    ConfigNode();
    ~ConfigNode();

    ConfigNode(ConfigNode&&);
    ConfigNode& operator=(ConfigNode&&);

    ConfigNode child(const char* name) const;
    ConfigNode child(const std::string& name) const;

    ConfigNodeIterator children() const;

    /*ConfigNode children(const char* name) const;
    ConfigNode children(const std::string& name) const;*/

    ConfigNode selectChild(std::string_view selector) const;

    std::string_view attribute(const char* name) const;
    std::string_view attribute(const std::string& name) const;

    std::string_view value() const;
    std::string_view trimmedValue() const;

    template <typename T>
    std::optional<T> attribute(const char* name) const;
    template <typename T>
    std::optional<T> attribute(const std::string& name) const
    {
        return attribute<T>(name.c_str());
    }

    bool operator!() const;

protected:
    friend class ConfigNodeIterator;

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

class ConfigNodeIterator
{
public:
    ConfigNodeIterator() = default;
    ConfigNodeIterator(const ConfigNode& node);
    ConfigNodeIterator(const ConfigNodeIterator&) = delete;
    ConfigNodeIterator(ConfigNodeIterator&&);
    ~ConfigNodeIterator();

    ConfigNodeIterator& operator++();
    ConfigNodeIterator& operator=(ConfigNodeIterator&& other);
    bool operator==(const ConfigNodeIterator& other) const;
    bool operator!=(const ConfigNodeIterator& other) const;

    ConfigNode operator*();

private:
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

private:
    ConfigDocument(const std::string& data, bool isPath);

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimplDoc;
};

} // namespace infra
