#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace infra {

class ConfigNode
{
public:
    ConfigNode();
    ~ConfigNode();

    ConfigNode(ConfigNode&&) = default;
    ConfigNode& operator=(ConfigNode&&) = default;

    ConfigNode child(const char* name);
    ConfigNode child(const std::string& name);

    ConfigNode selectChild(std::string_view selector);

    std::string_view attribute(const char* name);
    std::string_view attribute(const std::string& name);

    std::string_view value();
    std::string_view trimmedValue();

    template <typename T>
    std::optional<T> attribute(const char* name);
    template <typename T>
    std::optional<T> attribute(const std::string& name)
    {
        return attribute<T>(name.c_str());
    }

protected:
    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

class ConfigDocument : public ConfigNode
{
public:
    static ConfigDocument loadFromFile(const std::string& filename);
    static ConfigDocument loadFromString(const std::string& data);

    ConfigDocument(const ConfigDocument&) = delete;
    ConfigDocument& operator=(const ConfigDocument&) = delete;
    ConfigDocument(ConfigDocument&&)                 = default;
    ConfigDocument& operator=(ConfigDocument&&) = default;

    ~ConfigDocument();

private:
    ConfigDocument(const std::string& data, bool isPath);

    struct Pimpl;
    std::unique_ptr<Pimpl> _pimplDoc;
};

} // namespace infra
