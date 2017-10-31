#pragma once

#include <gdal_priv.h>

namespace infra::gdal {

template <typename T>
struct type_resolve
{
    static constexpr GDALDataType value = GDT_Unknown;
};

template <>
struct type_resolve<uint8_t>
{
    static constexpr GDALDataType value = GDT_Byte;
};

template <>
struct type_resolve<uint16_t>
{
    static constexpr GDALDataType value = GDT_UInt16;
};

template <>
struct type_resolve<int16_t>
{
    static constexpr GDALDataType value = GDT_Int16;
};

template <>
struct type_resolve<uint32_t>
{
    static constexpr GDALDataType value = GDT_UInt32;
};

template <>
struct type_resolve<int32_t>
{
    static constexpr GDALDataType value = GDT_Int32;
};

template <>
struct type_resolve<float>
{
    static constexpr GDALDataType value = GDT_Float32;
};

template <>
struct type_resolve<double>
{
    static constexpr GDALDataType value = GDT_Float64;
};

void throwLastError(const char* msg);
void checkError(CPLErr err, const char* msg);
void checkError(OGRErr err, const char* msg);
void checkError(CPLErr err, const std::string& msg);
void checkError(OGRErr err, const std::string& msg);

template <typename T>
T* checkPointer(T* instance, const char* msg)
{
    if (instance == nullptr) {
        throwLastError(msg);
    }

    return instance;
}
}
