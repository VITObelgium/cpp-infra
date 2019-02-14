#pragma once

#include <complex>
#include <gdal_priv.h>
#include <typeinfo>

namespace inf::gdal {

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

inline const type_info& resolveType(GDALDataType type)
{
    switch (type) {
    case GDT_Byte:
        return typeid(uint8_t);
    case GDT_UInt16:
        return typeid(uint16_t);
    case GDT_Int16:
        return typeid(int16_t);
    case GDT_UInt32:
        return typeid(uint32_t);
    case GDT_Int32:
        return typeid(int32_t);
    case GDT_Float32:
        return typeid(float);
    case GDT_Float64:
        return typeid(double);
    case GDT_CInt16:
        return typeid(int16_t);
        return typeid(std::complex<int16_t>);
    case GDT_CInt32:
        return typeid(std::complex<int32_t>);
    case GDT_CFloat32:
        return typeid(std::complex<float>);
    case GDT_CFloat64:
        return typeid(std::complex<double>);
    case GDT_Unknown:
    default:
        return typeid(void);
    }
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
