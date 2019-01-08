#pragma once

#include "gdx/exception.h"
#include "gdx/log.h"
#include "infra/string.h"

#include <gsl/span>

namespace gdx {

std::vector<std::vector<double>> readMappingFile(const std::string& mappingFile, int32_t dataColumns);
std::vector<std::vector<double>> readMappingFile(const std::string& mappingFile, int32_t dataColumns, int32_t index);
std::vector<std::vector<double>> readNMappingFile(const std::string& mappingFile);

namespace internal {

template <typename T>
std::string optionalToString(const std::optional<T>& opt)
{
    if (opt) {
        return std::to_string(*opt);
    } else {
        return "nodata";
    }
}

template <typename T, int ElementCount>
std::string valuesToString(const std::array<std::optional<T>, ElementCount>& values)
{
    return inf::str::join(values, ", ", [](const std::optional<T>& opt) {
        return optionalToString<T>(opt);
    });
}

template <typename T>
void throwOnMappingIssue(const std::vector<std::vector<double>>& mapping, std::optional<double> nodata)
{
    if constexpr (std::is_floating_point_v<T>) {
        return;
    }

    bool warningGiven = false;

    for (auto& mapList : mapping) {
        for (auto& value : mapList) {
            if (value > static_cast<double>(std::numeric_limits<T>::max())) {
                throw RuntimeError("Mapping value is bigger then input data type");
            }

            if (value < static_cast<double>(std::numeric_limits<T>::lowest())) {
                throw RuntimeError("Mapping value is smaller then input data type");
            }

            auto resultType = static_cast<T>(value);
            if (value - resultType > std::numeric_limits<T>::epsilon()) {
                throw RuntimeError("Floating point mapping values cannot be used on integral raster");
            }

            if (!warningGiven && nodata.has_value() && static_cast<T>(*nodata) == static_cast<T>(value)) {
                warningGiven = true;
                Log::warn("Reclass mapping value matches the resulting rasters nodata value, this can lead to unexpected results");
            }
        }
    }
}

template <typename T, int ElementCount>
std::optional<T> reclassValue(const std::array<std::optional<T>, ElementCount>& values, const std::vector<std::vector<double>>& mapping, bool& warningGiven)
{
    auto iter = std::find_if(mapping.begin(), mapping.end(), [&](const std::vector<double>& mappingValues) {
        for (size_t i = 0; i < values.size(); ++i) {
            if (values[i].has_value()) {
                if (values[i] != static_cast<T>(mappingValues[i])) {
                    return false;
                }
            } else if (!std::isnan(mappingValues[i])) {
                return false;
            }
        }

        return true;
    });

    if (iter == mapping.end()) {
        if (!warningGiven) {
            Log::warn("No mapping available for raster values: {}", valuesToString<T, ElementCount>(values));
            warningGiven = true;
        }

        return std::optional<T>();
    }

    auto result = iter->at(values.size());
    if (std::isnan(result)) {
        return std::optional<T>();
    }

    return static_cast<T>(result);
}
}

template <template <typename> typename RasterType, typename T>
void reclassInPlace(const std::vector<std::vector<double>>& mapping, RasterType<T>& ras)
{
    internal::throwOnMappingIssue<T>(mapping, ras.metadata().nodata);

    const auto size   = ras.size();
    bool warningGiven = false;

    for (int32_t i = 0; i < size; ++i) {
        std::array<std::optional<T>, 1> values({{ras.optional_value(i)}});
        auto v = internal::reclassValue<T, 1>(values, mapping, warningGiven);
        if (v) {
            ras[i] = *v;
            ras.mark_as_data(i);
        } else {
            if (!ras.metadata().nodata.has_value()) {
                throw RuntimeError("No mapping available for raster values: {} and no nodata value set", internal::valuesToString<T, 1>(values));
            }
            ras.mark_as_nodata(i);
        }
    }
}

template <template <typename> typename RasterType, typename T1, typename T2>
auto reclass(const std::vector<std::vector<double>>& mapping, const RasterType<T1>& ras1, const RasterType<T2>& ras2)
{
    // Use the widest type as result type
    using ResultType = decltype(T1() + T2());

    internal::throwOnMappingIssue<ResultType>(mapping, ras1.metadata().nodata);

    if (ras1.size() != ras2.size()) {
        throw InvalidArgument("Raster sizes should match {} {}", ras1.size(), ras2.size());
    }

    const auto size   = ras1.size();
    bool warningGiven = false;

    RasterType<ResultType> result(ras1.metadata());

    for (int32_t i = 0; i < size; ++i) {
        std::array<std::optional<ResultType>, 2> values = {{ras1.template optional_value_as<ResultType>(i),
            ras2.template optional_value_as<ResultType>(i)}};

        auto v = internal::reclassValue<ResultType, 2>(values, mapping, warningGiven);
        if (v) {
            result[i] = *v;
            result.mark_as_data(i);
        } else {
            result.mark_as_nodata(i);
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T1, typename T2, typename T3>
auto reclass(const std::vector<std::vector<double>>& mapping, const RasterType<T1>& ras1, const RasterType<T2>& ras2, const RasterType<T3>& ras3)
{
    // Use the widest type as result type
    using ResultType = decltype(T1() + T2() + T3());

    internal::throwOnMappingIssue<ResultType>(mapping, ras1.metadata().nodata);

    if (ras1.size() != ras2.size() || ras1.size() != ras3.size()) {
        throw InvalidArgument("Raster sizes should match {} {} {}", ras1.size(), ras2.size(), ras3.size());
    }

    const auto size   = ras1.size();
    bool warningGiven = false;

    RasterType<ResultType> result(ras1.metadata());

    for (int32_t i = 0; i < size; ++i) {
        std::array<std::optional<ResultType>, 3> values = {{ras1.template optional_value_as<ResultType>(i),
            ras2.template optional_value_as<ResultType>(i),
            ras3.template optional_value_as<ResultType>(i)}};

        auto v = internal::reclassValue<ResultType, 3>(values, mapping, warningGiven);
        if (v) {
            result[i] = *v;
            result.mark_as_data(i);
        } else {
            result.mark_as_nodata(i);
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> reclass(const std::vector<std::vector<double>>& mapping, const RasterType<T>& ras)
{
    RasterType<T> result = ras.copy();
    reclassInPlace(mapping, result);
    return result;
}

template <template <typename> typename RasterType, typename T>
auto reclass(const std::string& mappingFile, const RasterType<T>& ras)
{
    return reclass(readMappingFile(mappingFile, 1, 1), ras);
}

template <template <typename> typename RasterType, typename T1, typename T2>
auto reclass(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2)
{
    return reclass(readMappingFile(mappingFile, 2), ras1, ras2);
}

template <template <typename> typename RasterType, typename T1, typename T2, typename T3>
auto reclass(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2, const RasterType<T3>& ras3)
{
    return reclass(readMappingFile(mappingFile, 3), ras1, ras2, ras3);
}

template <template <typename> typename RasterType, typename T>
void reclassInPlace(const std::string& mappingFile, RasterType<T>& ras)
{
    reclassInPlace(readMappingFile(mappingFile, 1), ras);
}

template <template <typename> typename RasterType, typename T>
auto reclassi(const std::string& mappingFile, const RasterType<T>& ras, int32_t index)
{
    return reclass(readMappingFile(mappingFile, 1, index), ras);
}

template <template <typename> typename RasterType, typename T1, typename T2>
auto reclassi(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2, int32_t index)
{
    return reclass(readMappingFile(mappingFile, 2, index), ras1, ras2);
}

template <template <typename> typename RasterType, typename T1, typename T2, typename T3>
auto reclassi(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2, const RasterType<T3>& ras3, int32_t index)
{
    return reclass(readMappingFile(mappingFile, 3, index), ras1, ras2, ras3);
}

template <template <typename> typename RasterType, typename T>
RasterType<T> nreclass(const std::vector<std::vector<double>>& mapping, const RasterType<T>& raster)
{
    auto resultMeta = raster.metadata();
    T nodata        = std::numeric_limits<T>::max();
    if (resultMeta.nodata.has_value()) {
        nodata = static_cast<T>(*resultMeta.nodata);
    } else if constexpr (RasterType<T>::raster_type_has_nan) {
        nodata = RasterType<T>::NaN;
    }

    resultMeta.nodata = nodata;
    internal::throwOnMappingIssue<T>(mapping, resultMeta.nodata);

    RasterType<T> result(resultMeta, nodata);

    bool warningTriggered        = false;
    int countNANbecauseKeyNAN    = 0;
    int countNANbecauseNoSuchKey = 0;

    for (int i = 0; i < raster.size(); ++i) {
        if (raster.is_nodata(i)) {
            ++countNANbecauseKeyNAN;
            continue;
        }

        const auto key = raster[i];
        bool found     = false;
        for (size_t j = 0; j < mapping.size(); ++j) {
            if (T(mapping[j][0]) < key && key <= T(mapping[j][1])) {
                if (!std::isnan(mapping[j][2])) {
                    result[i] = static_cast<T>(mapping[j][2]);
                    result.mark_as_data(i);
                }

                found = true;
                break;
            }
        }

        if (!found) {
            if (!warningTriggered) {
                warningTriggered = true;
                Log::warn("nreclass : no entry for key {} (has nodata result)", key);
            }

            ++countNANbecauseNoSuchKey;
        }
    }

    if (countNANbecauseNoSuchKey > 0) {
        Log::warn("nreclass : {} times nodata result because input key is not in table", countNANbecauseNoSuchKey);
    }

    if (countNANbecauseKeyNAN > 0) {
        Log::warn("nreclass : {} times nodata result because input key is not is nodata", countNANbecauseKeyNAN);
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> nreclass(const std::string& mappingFile, const RasterType<T>& raster)
{
    return nreclass(readNMappingFile(mappingFile), raster);
}
}
