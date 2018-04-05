#pragma once

#include <gdal_priv.h>
#include <typeinfo>

namespace infra::gdal {

template <typename T>
struct TypeResolve
{
    static constexpr GDALDataType value = GDT_Unknown;
};

template <>
struct TypeResolve<uint8_t>
{
    static constexpr GDALDataType value = GDT_Byte;
};

template <>
struct TypeResolve<uint16_t>
{
    static constexpr GDALDataType value = GDT_UInt16;
};

template <>
struct TypeResolve<int16_t>
{
    static constexpr GDALDataType value = GDT_Int16;
};

template <>
struct TypeResolve<uint32_t>
{
    static constexpr GDALDataType value = GDT_UInt32;
};

template <>
struct TypeResolve<int32_t>
{
    static constexpr GDALDataType value = GDT_Int32;
};

template <>
struct TypeResolve<float>
{
    static constexpr GDALDataType value = GDT_Float32;
};

template <>
struct TypeResolve<double>
{
    static constexpr GDALDataType value = GDT_Float64;
};

inline GDALDataType resolveType(const std::type_info& info)
{
    if (info == typeid(uint8_t)) {
        return TypeResolve<uint8_t>::value;
    } else if (info == typeid(uint16_t)) {
        return TypeResolve<uint16_t>::value;
    } else if (info == typeid(int16_t)) {
        return TypeResolve<int16_t>::value;
    } else if (info == typeid(uint32_t)) {
        return TypeResolve<uint32_t>::value;
    } else if (info == typeid(int32_t)) {
        return TypeResolve<int32_t>::value;
    } else if (info == typeid(float)) {
        return TypeResolve<float>::value;
    } else if (info == typeid(double)) {
        return TypeResolve<double>::value;
    }

    return GDT_Unknown;
}

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
