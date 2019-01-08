#pragma once

namespace gdx {

namespace internal {

template <
    template <typename> typename BinaryPredicate,
    template <typename> typename RasterType,
    typename T1,
    typename T2,
    typename std::enable_if_t<std::is_arithmetic_v<typename RasterType<T1>::mask_value_type>, int>* = nullptr>
RasterType<uint8_t> transformRasterBinaryResult(const RasterType<T1>& lhs, const RasterType<T2>& rhs)
{
    using WidestType = decltype(T1() * T2());

    throw_on_size_mismatch(lhs, rhs);

    RasterType<uint8_t> result(lhs.metadata(), combineMask(lhs.mask_data(), rhs.mask_data()));
    if (lhs.metadata().nodata.has_value() || rhs.metadata().nodata.has_value()) {
        result.set_nodata(std::numeric_limits<uint8_t>::max());
    }

    auto pred = BinaryPredicate<WidestType>();
    static_assert(std::is_convertible_v<decltype(pred(0, 0)), uint8_t>, "Predicat result not convertible to uint8_t");

    std::transform(begin(lhs), end(lhs), begin(rhs), begin(result), [pred](T1 l, T2 r) {
        return static_cast<uint8_t>(pred(static_cast<WidestType>(l), static_cast<WidestType>(r)));
    });
    return result;
}

template <
    template <typename> typename BinaryPredicate,
    template <typename> typename RasterType,
    typename T1,
    typename T2,
    typename std::enable_if_t<RasterType<T1>::with_nodata, int>* = nullptr>
RasterType<uint8_t> transformRasterBinaryResult(const RasterType<T1>& lhs, const RasterType<T2>& rhs)
{
    using WidestType = decltype(T1() * T2());

    throw_on_size_mismatch(lhs, rhs);

    auto nodata = std::numeric_limits<uint8_t>::max();
    RasterType<uint8_t> result(lhs.metadata());
    if (lhs.metadata().nodata.has_value() || rhs.metadata().nodata.has_value()) {
        result.set_nodata(nodata);
    }

    auto pred = BinaryPredicate<WidestType>();
    static_assert(std::is_convertible_v<decltype(pred(0, 0)), uint8_t>, "Predicat result not convertible to uint8_t");

    std::transform(begin(lhs), end(lhs), begin(rhs), begin(result), [&lhs, &rhs, nodata, pred](T1 val1, T2 val2) {
        if (lhs.is_nodataValue(val1) || rhs.is_nodataValue(val2)) {
            return nodata;
        }

        return static_cast<uint8_t>(pred(static_cast<WidestType>(val1), static_cast<WidestType>(val2)));
    });
    return result;
}

template <
    template <typename> typename RasterType,
    typename TRaster,
    typename UnaryPredicate,
    typename std::enable_if_t<std::is_arithmetic_v<typename RasterType<TRaster>::mask_value_type>, int>* = nullptr>
RasterType<uint8_t> transformRasterBinaryResult(const RasterType<TRaster>& ras, UnaryPredicate&& pred)
{
    RasterType<uint8_t> result(ras.metadata(), ras.mask_data());
    if (ras.metadata().nodata.has_value()) {
        result.set_nodata(std::numeric_limits<uint8_t>::max());
    }

    std::transform(begin(ras), end(ras), begin(result), pred);
    return result;
}

template <
    template <typename> typename RasterType,
    typename TRaster,
    typename UnaryPredicate,
    typename std::enable_if_t<RasterType<TRaster>::with_nodata, int>* = nullptr>
RasterType<uint8_t> transformRasterBinaryResult(const RasterType<TRaster>& ras, UnaryPredicate&& pred)
{
    auto nodata = std::numeric_limits<uint8_t>::max();

    RasterType<uint8_t> result(ras.metadata());
    if (ras.metadata().nodata.has_value()) {
        result.set_nodata(nodata);
    }

    std::transform(begin(ras), end(ras), begin(result), [&ras, nodata, pred](TRaster value) {
        if (ras.is_nodataValue(value)) {
            return nodata;
        }

        return pred(value);
    });
    return result;
}
}

template <template <typename> typename RasterType, typename TRaster>
RasterType<TRaster> rasterEqualOneOf(const RasterType<TRaster>& ras, const std::vector<TRaster>& values)
{
    RasterType<TRaster> result(ras.metadata());

    for (int32_t i = 0; i < ras.size(); ++i) {
        if (ras.is_nodata(i)) {
            result[i] = ras[i];
        } else {
            auto iter = std::find(values.begin(), values.end(), ras[i]);
            result[i] = (iter != values.end());
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<uint8_t> isClose(const RasterType<T>& lhs, const RasterType<T>& rhs, T relTolerance = T(1e-05), T absTolerance = T(1e-08))
{
    throw_on_size_mismatch(lhs, rhs);

    RasterType<uint8_t> result(lhs.metadata());

    auto isEqual        = cpu::float_equal_to<T>(relTolerance, absTolerance);
    const auto dataSize = lhs.size();
#pragma omp parallel for
    for (int32_t i = 0; i < dataSize; ++i) {
        if (lhs.is_nodata(i) || rhs.is_nodata(i)) {
            result[i] = lhs.is_nodata(i) == rhs.is_nodata(i) ? 1 : 0;
        } else {
            result[i] = isEqual(lhs[i], rhs[i]);
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T1, typename T2>
RasterType<uint8_t> equals(const RasterType<T1>& lhs, const RasterType<T2>& rhs)
{
    return internal::transformRasterBinaryResult<std::equal_to>(lhs, rhs);
}

template <template <typename> typename RasterType, typename TRaster, typename TScalar>
RasterType<uint8_t> equals(const RasterType<TRaster>& ras, TScalar value)
{
    static_assert(std::is_scalar_v<TScalar>, "Arithmetic operation called with non scalar type");
    return internal::transformRasterBinaryResult(ras, cpu::equal_to_scalar<TRaster>(value));
}
}
