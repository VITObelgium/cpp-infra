#pragma once

#include <complex>
#include <gdal_priv.h>
#include <gdal_version.h>
#include <typeinfo>

#include "infra/span.h"

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

#if GDAL_VERSION_MAJOR > 2
template <>
struct TypeResolve<uint64_t>
{
    static constexpr GDALDataType value = GDT_UInt64;
};

template <>
struct TypeResolve<int64_t>
{
    static constexpr GDALDataType value = GDT_Int64;
};
#endif

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

inline GDALDataType resolve_type(const std::type_info& info)
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

#if GDAL_VERSION_MAJOR > 2
    if (info == typeid(uint64_t)) {
        return TypeResolve<uint64_t>::value;
    } else if (info == typeid(int64_t)) {
        return TypeResolve<int64_t>::value;
    }
#endif

    return GDT_Unknown;
}

inline const std::type_info& resolve_type(GDALDataType type)
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
        // return typeid(std::complex<int16_t>);
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

int throw_if_not_supported(int result);
void throw_last_error(std::string_view msg);
void check_error(CPLErr err, std::string_view msg);
void check_error(OGRErr err, std::string_view msg);
CPLStringList create_string_list(std::span<const std::string> driverOptions);

template <typename T>
T* check_pointer(T* instance, std::string_view msg)
{
    if (instance == nullptr) {
        throw_last_error(msg);
    }

    return instance;
}

/* Overload that accepts a callback for creating the error message
 * Use if you want to avoid constructing the error message if the pointer is ok */
template <typename T, typename Callable>
T* check_pointer_msg_cb(T* instance, Callable&& msgCb)
{
    if (instance == nullptr) {
        throw_last_error(msgCb());
    }

    return instance;
}

template <typename T>
class CplPointer
{
public:
    CplPointer() = default;
    CplPointer(T* ptr)
    : _ptr(ptr)
    {
    }

    ~CplPointer()
    {
        CPLFree(_ptr);
    }

    T** ptrAddress()
    {
        return &_ptr;
    }

    operator T*()
    {
        return _ptr;
    }

    T* get()
    {
        return _ptr;
    }

    const T* get() const
    {
        return _ptr;
    }

    operator bool() const noexcept
    {
        return _ptr != nullptr;
    }

private:
    T* _ptr = nullptr;
};
}
