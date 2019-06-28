#pragma once

#include "infra/database.h"
#include "infra/enumutils.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/log.h"
#include "infra/string.h"

#include <string>
#include <variant>

namespace inf {

static const char* s_stringType = "string";
static const char* s_int32Type  = "integer32";
static const char* s_int64Type  = "integer64";
static const char* s_doubleType = "double";
static const char* s_boolType   = "bool";

template <typename T, typename GroupTag, typename GroupTag::Id Id>
struct ParameterDefinition
{
    using group_tag                           = GroupTag;
    using value_type                          = T;
    using default_value_type                  = std::conditional_t<std::is_same_v<T, std::string>, const char*, T>;
    static constexpr typename GroupTag::Id id = Id;

    constexpr ParameterDefinition(const char* storageName, default_value_type iDefaultValue)
    : name(storageName)
    , defaultValue(iDefaultValue)
    {
    }

    const char* name;
    default_value_type defaultValue;
};

template <typename ParameterDescription>
class Configuration
{
public:
    
    Configuration(const fs::path& db, std::string_view tableName)
    : _dbPath(db)
    , _tableName(tableName)
    {
    }

    

    void read_from_disk(inf::db::AbstractDatabase& db)
    {
        // set default values
        typename ParameterDescription::visitParameters([this](auto& param) {
            using ParameterDef            = std::decay_t<decltype(param)>;
            _params[enum_value(param.id)] = typename ParameterDef::value_type(param.defaultValue);
        });

        for (auto& p : db.get_config_parameters(_tableName)) {
            typename ParameterDescription::visitParameters([this, &p](auto& param) {
                using ParameterDef  = std::decay_t<decltype(param)>;
                using ParameterType = typename ParameterDef::value_type;

                if (p.name == param.name) {
                    if constexpr (std::is_same_v<ParameterType, int32_t>) {
                        if (auto val = str::toInt32(p.value); val.has_value()) {
                            _params[enum_value(param.id)] = val.value();
                        }
                    } else if constexpr (std::is_same_v<ParameterType, int64_t>) {
                        if (auto val = str::toInt64(p.value); val.has_value()) {
                            _params[enum_value(param.id)] = val.value();
                        }
                    } else if constexpr (std::is_same_v<ParameterType, double>) {
                        if (auto val = str::toDouble(p.value); val.has_value()) {
                            _params[enum_value(param.id)] = val.value();
                        }
                    } else if constexpr (std::is_same_v<ParameterType, bool>) {
                        if (auto val = str::toInt32(p.value); val.has_value()) {
                            _params[enum_value(param.id)] = val.value() == 1;
                        }
                    } else if constexpr (std::is_same_v<ParameterType, std::string>) {
                        _params[enum_value(param.id)] = p.value;
                    } else {
                        static_assert(dependent_false_v<ParameterType>, "Unsupported parameter type");
                    }
                }
            });
        }
    }

    void store_to_disk(inf::db::AbstractDatabase& db)
    {
        int index = 0;
        for (auto& paramVar : _params) {
            typename ParameterDescription::visitParameters([this, index, &paramVar, &db](auto& param) {
                using ParameterDef  = std::decay_t<decltype(param)>;
                using ParameterType = typename ParameterDef::value_type;

                if (index == enum_value(param.id)) {
                    inf::ConfigParameter configParam;
                    configParam.name = param.name;

                    if constexpr (std::is_same_v<ParameterType, int32_t>) {
                        configParam.type  = s_int32Type;
                        configParam.value = std::to_string(std::get<int32_t>(paramVar));
                    } else if constexpr (std::is_same_v<ParameterType, int64_t>) {
                        configParam.type  = s_int64Type;
                        configParam.value = std::to_string(std::get<int64_t>(paramVar));
                    } else if constexpr (std::is_same_v<ParameterType, double>) {
                        configParam.type  = s_doubleType;
                        configParam.value = std::to_string(std::get<double>(paramVar));
                    } else if constexpr (std::is_same_v<ParameterType, bool>) {
                        configParam.type  = s_boolType;
                        configParam.value = std::get<bool>(paramVar) ? "1" : "0";
                    } else if constexpr (std::is_same_v<ParameterType, std::string>) {
                        configParam.type  = s_stringType;
                        configParam.value = std::get<std::string>(paramVar);
                    } else {
                        static_assert(dependent_false_v<ParameterType>, "Unsupported parameter type");
                    }

                    db.set_config_parameter(_tableName, configParam);
                }
            });

            ++index;
        }
    }


    fs::path databasePath() const
    {
        return _dbPath;
    }

    template <typename Parameter>
    auto get(Parameter) const
    {
        static_assert(std::is_same_v<ParameterDescription, typename Parameter::group_tag>, "Invalid parameter provided, belongs to a different group");
        static_assert(inf::enum_value(Parameter::id) < std::tuple_size<decltype(_params)>::value, "Invalid parameter index, make sure the Count Id is present");

        using T        = typename Parameter::value_type;
        auto& paramVar = _params[inf::enum_value(Parameter::id)];
        assert(std::holds_alternative<T>(paramVar));
        return std::get<T>(paramVar);
    }

    template <typename Parameter>
    void set(Parameter, const typename Parameter::value_type& value)
    {
        constexpr auto index = inf::enum_value(Parameter::id);
        static_assert(std::is_same_v<ParameterDescription, typename Parameter::group_tag>, "Invalid parameter provided, belongs to a different group");
        static_assert(index < std::tuple_size<decltype(_params)>::value, "Invalid parameter index, make sure the Count Id is present");

        _params[index] = ConfigParameter(value);
    }

private:
    using ConfigParameter = std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>;

    fs::path _dbPath;
    std::array<ConfigParameter, ParameterDescription::ParameterCount> _params;
    std::string _tableName;
};
}
