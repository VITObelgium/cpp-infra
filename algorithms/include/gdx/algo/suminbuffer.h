#pragma once

#include "gdx/algo/cast.h"
#include "gdx/algo/conversion.h"
#include "gdx/algo/nodata.h"

#include "gdx/cell.h"
#include "gdx/exception.h"

#include <gsl/span>
#include <vector>

namespace gdx {

// "summed area table" technique, see wikipedia
template <template <typename> typename RasterType, typename T>
RasterType<double> computeIntegralImage(const RasterType<T>& image)
{
    const auto nRows = image.metadata().rows;
    const auto nCols = image.metadata().cols;

    RasterType<double> summedArea(image.metadata());

    summedArea(0, 0) = image(0, 0);

    for (int c = 1; c < nCols; ++c) {
        summedArea(0, c) = summedArea(0, c - 1) + image(0, c);
    }

    for (int r = 1; r < nRows; ++r) {
        summedArea(r, 0) = summedArea(r - 1, 0) + image(r, 0);
    }

    for (int r = 1; r < nRows; ++r) {
        double rowSum            = image(r, 0);
        auto* pImage             = &image(r, 1);
        auto* pPrevSummedAreaRow = &summedArea(r - 1, 1);
        auto* pSummedAreaRow     = &summedArea(r, 1);
        for (int c = 1; c < nCols; ++c) {
            rowSum += *pImage;                              // rowSum += image(r, c);
            *pSummedAreaRow = rowSum + *pPrevSummedAreaRow; // summedArea(r,c) = rowSum + summedArea(r-1,c);
            ++pImage;
            ++pPrevSummedAreaRow;
            ++pSummedAreaRow;
        }
    }

    return summedArea;
}

// summed circular area, using sliding window technique.  Much slower than integral image technique.
void computeCircleBorderOffsets(const int radius /*cells*/,
    std::vector<Cell>& plusRight,
    std::vector<Cell>& minLeft,
    std::vector<Cell>& plusDown,
    std::vector<Cell>& minTop);

template <typename T>
long double computeSumInCells(const int32_t row, const int32_t col, const int32_t rows, int32_t cols,
    gsl::span<const T> image, gsl::span<const Cell> cells)
{
    long double result = 0.0;

    for (const auto& cell : cells) {
        const int r = row + cell.r;
        const int c = col + cell.c;
        if (0 <= r && r < rows && 0 <= c && c < cols) {
            result += image[cols * r + c];
        }
    }

    return result;
}

// this routine also works for rc outside the integral image
template <typename TValue, template <typename> typename RasterType, typename TIntegralImageElem>
TValue computeSumWithinRectangle(int r0, int c0, int r1, int c1, const RasterType<TIntegralImageElem>& I /*integral image*/, const int nRows, const int nCols)
{
    if (r0 > nRows - 1) r0 = nRows - 1;
    if (c0 > nCols - 1) c0 = nCols - 1;
    if (r1 > nRows - 1) r1 = nRows - 1;
    if (c1 > nCols - 1) c1 = nCols - 1;
    const TIntegralImageElem A = (r0 >= 0 && c0 >= 0 ? I(r0, c0) : 0);
    const TIntegralImageElem B = (r0 >= 0 && c1 >= 0 ? I(r0, c1) : 0);
    const TIntegralImageElem C = (r1 >= 0 && c1 >= 0 ? I(r1, c1) : 0);
    const TIntegralImageElem D = (r1 >= 0 && c0 >= 0 ? I(r1, c0) : 0);
    return TValue(C + A - B - D);
}

// this routine also works for rc outside the integral image
template <typename TValue, template <typename> typename RasterType, typename TIntegralImageElem>
TValue computeSumWithinRectangleAround(const int r, const int c, const int radius, const RasterType<TIntegralImageElem>& I /*integral image*/, const int nRows, const int nCols)
{
    int r0 = r - radius - 1;
    int c0 = c - radius - 1;
    int r1 = r + radius;
    int c1 = c + radius;

    return computeSumWithinRectangle<TValue>(r0, c0, r1, c1, I, nRows, nCols);
}

template <typename T, template <typename> typename RasterType, typename TIntegralImageElem>
T computeSumWithinCircle(const int32_t row, const int32_t col, const int32_t radius /*cells*/,
    int32_t rows, int32_t cols, gsl::span<const T> image,
    long double& prevSum, int& prevR, int& prevC,
    std::vector<Cell>& plusRight,
    std::vector<Cell>& minLeft,
    std::vector<Cell>& plusDown,
    std::vector<Cell>& minTop,
    const RasterType<TIntegralImageElem>& I /*integral image, used to quickly detect circles with no values */)
{
    long double thisSum = 0;

    if (computeSumWithinRectangleAround<float>(row, col, radius, I, rows, cols) == 0) {
        // we are done
    } else if (prevC + 1 == col && prevR == row) {
        thisSum = prevSum;
        thisSum += computeSumInCells(row, col, rows, cols, image, plusRight);
        thisSum -= computeSumInCells(row, col, rows, cols, image, minLeft);
    } else if (prevR + 1 == row && prevC == col) {
        thisSum = prevSum;
        thisSum += computeSumInCells(row, col, rows, cols, image, plusDown);
        thisSum -= computeSumInCells(row, col, rows, cols, image, minTop);
    } else {
        const int rad2 = radius * radius;
        for (int dR = -radius; dR <= +radius; ++dR) {
            const int r = row + dR;
            if (0 <= r && r < rows) {
                const int dR2 = dR * dR;
                for (int dC = -radius; dC <= +radius; ++dC) {
                    const int c = col + dC;
                    if (0 <= c && c < cols) {
                        if (dR2 + dC * dC <= rad2) {
                            thisSum += image[cols * r + c];
                        }
                    }
                }
            }
        }
    }

    prevR   = row;
    prevC   = col;
    prevSum = thisSum;

    return static_cast<T>(thisSum);
}

template <template <typename> typename RasterType, typename T>
RasterType<T> sumInBuffer(const RasterType<T>& ras, float radiusInMeter)
{
    static_assert(!std::is_same_v<T, uint8_t>, "Bad overload chosen for uint8_t");

    auto src = ras.copy();
    replace_nodata_in_place(src, 0);

    float radiusInCells = radiusInMeter / static_cast<float>(ras.metadata().cellSize);
    int32_t radius      = int32_t(radiusInCells);

    RasterType<T> result(ras.metadata());

    const auto rows = ras.metadata().rows;
    const auto cols = ras.metadata().cols;

    long double prevSum = 0;
    int prevR           = -radius + 1;
    int prevC           = -radius + 1;

    // datastructure for RoundBuffer style
    std::vector<Cell> plusRight;
    std::vector<Cell> minLeft;
    std::vector<Cell> plusDown;
    std::vector<Cell> minTop;

    auto summedArea = computeIntegralImage(src);
    computeCircleBorderOffsets(radius, plusRight, minLeft, plusDown, minTop);

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            result(r, c) = computeSumWithinCircle<T>(r, c, radius, rows, cols, src, prevSum, prevR, prevC, plusRight, minLeft, plusDown, minTop, summedArea);
        }
    }

    return result;
}

template <template <typename> typename RasterType>
inline RasterType<int32_t> sumInBuffer(const RasterType<uint8_t>& ras, float radiusInMeter)
{
    // cast uint8_t raster to int32_t, otherwise chances to overflow are too high
    auto intRaster = raster_cast<int32_t>(ras);
    intRaster.set_nodata(-1);
    return sumInBuffer(intRaster, radiusInMeter);
}

inline void computeRectOnMapAround(const int row, const int col, const int radius, int& r0, int& c0, int& r1, int& c1, int nRows, int nCols)
{
    r0 = row - radius;
    if (r0 < 0) r0 = 0;
    r1 = row + radius;
    if (r1 > nRows - 1) r1 = nRows - 1;
    c0 = col - radius;
    if (c0 < 0) c0 = 0;
    c1 = col + radius;
    if (c1 > nCols - 1) c1 = nCols - 1;
}

template <typename T>
inline auto computeD2(const T arow, const T acol, const T brow, const T bcol) -> T
{
    const T drow = arow - brow;
    const T dcol = acol - bcol;
    return drow * drow + dcol * dcol;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> maxInBuffer(const RasterType<T>& ras, float radiusInMeter)
{
    float radiusInCells            = radiusInMeter / static_cast<float>(ras.metadata().cellSize);
    const long long radius2InCells = (long long)(ceil(radiusInCells * radiusInCells));
    const auto rows                = ras.metadata().rows;
    const auto cols                = ras.metadata().cols;

    RasterType<T> result(ras.metadata());
    result.fill(0);

    for (int32_t row = 0; row < rows; ++row) {
        for (int32_t col = 0; col < cols; ++col) {
            int r0, c0, r1, c1;
            computeRectOnMapAround(row, col, int(ceil(radiusInCells)), r0, c0, r1, c1, rows, cols);
            T value = std::numeric_limits<T>::lowest();
            for (int r = r0; r <= r1; ++r) {
                for (int c = c0; c <= c1; ++c) {
                    if (computeD2<long long>(row, col, r, c) <= radius2InCells) {
                        if (!ras.is_nodata(r, c) && value < ras(r, c)) {
                            value = ras(r, c);
                        }
                    }
                }
            }
            result(row, col) = value;
        }
    }

    return result;
}

#ifdef HAVE_OPENCL
template <typename T>
typename gpu::Raster<T> sumInBuffer(const gpu::Raster<T>& ras, float radiusInMeter)
{
    namespace bc = boost::compute;

    int32_t radius = static_cast<int32_t>(radiusInMeter / ras.metadata().cellSize);

    gpu::Raster<T> result(ras.metadata());

    const auto rows = ras.metadata().rows;
    const auto cols = ras.metadata().cols;

    auto outputIter = result.begin();

    //std::vector<int> hostIndices;
    //const auto size = (radius * 2) + 1;
    //hostIndices.reserve(size * size);
    //for (auto i = 0; i <= size; ++i)
    //{
    //    for (auto j = 0; j <= size; ++j)
    //    {
    //        hostIndices.push_back(i * cols + j);
    //    }
    //}

    //bc::vector<uint32_t> indices(hostIndices.size());
    ////bc::copy(hostIndices.begin(), hostIndices.end(), indices.begin());
    //bc::fill(indices.begin(), indices.end(), 2.f);

    bc::float2_ center(0, 0);
    BOOST_COMPUTE_CLOSURE(T, in_circle, (T cellValue), (radius, rows, cols, center),
        {
            const int id       = get_global_id(0);
            const float2 point = (float2)(id % cols, id / cols);

            if (distance(point, center) <= radius) {
                return isnan(cellValue) ? 0 : cellValue;
            }

            return 0;
        });

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            center = bc::float2_(static_cast<float>(c), static_cast<float>(r));

            bc::transform_reduce(ras.begin(), ras.end(), outputIter++, in_circle, bc::plus<T>());

            /*bc::transform_reduce(
                bc::make_permutation_iterator(ras.begin(), indices.begin()),
                bc::make_permutation_iterator(ras.end(), indices.end()),
                outputIter++, in_circle, bc::plus<T>()
            );*/

            //bc::vector<T> res(ras.size());
            //bc::transform(
            //    //bc::make_permutation_iterator(ras.begin(), indices.begin()),
            //    //bc::make_permutation_iterator(ras.end(), indices.end()),
            //    ras.begin(),
            //    ras.end(),
            //    res.begin(), in_circle
            //);

            /*std::vector<T> hostRes(res.size());
            bc::copy(res.begin(), res.end(), hostRes.begin());*/
        }
    }

    return result;
}
#endif
}
