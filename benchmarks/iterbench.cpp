#include "gdx/algo/sum.h"
#include "gdx/denseraster.h"
#include "gdx/maskedraster.h"
#include "gdx/rasteriterator.h"

#include <benchmark/benchmark.h>
#include <iostream>
#include <numeric>

using namespace gdx;

class NoDataIter : public benchmark::Fixture
{
public:
    NoDataIter()
    : _meta(100, 100, -1.0)
    , _raster(_meta, 1)
    {
        _raster.mark_as_nodata(150);
    }

    void verifySum() const noexcept
    {
        if (int(_sum) != _raster.size() - 1) {
            std::terminate();
        }
    }

    void verifySumAll() const noexcept
    {
        if (int(_sum) != _raster.size()) {
            std::terminate();
        }
    }

    RasterMetadata _meta;
    MaskedRaster<int> _raster;
    double _sum = 0.0;
};

class DenseNoDataIter : public benchmark::Fixture
{
public:
    DenseNoDataIter()
    : _meta(100, 100, -1.0)
    , _raster(_meta, 1)
    {
        _raster.mark_as_nodata(150);
    }

    void verifySum() const noexcept
    {
        if (int(_sum) != _raster.size() - 1) {
            std::terminate();
        }
    }

    RasterMetadata _meta;
    DenseRaster<int> _raster;
    double _sum = 0.0;
};

BENCHMARK_F(NoDataIter, rawLoop)
(benchmark::State& state)
{
    const auto size = _raster.size();
    const auto& ras = _raster;

    for (auto _ : state) {
        _sum = 0.0;
        for (int i = 0; i < size; ++i) {
            if (!ras.is_nodata(i)) {
                _sum += ras[i];
            }
        }

        verifySum();
    }
}

BENCHMARK_F(NoDataIter, nodataIter)
(benchmark::State& state)
{
    for (auto _ : state) {
        _sum = static_cast<double>(std::accumulate(value_cbegin(_raster), value_cend(_raster), 0, std::plus<int>()));
        verifySum();
    }
}

BENCHMARK_F(DenseNoDataIter, rawLoop)
(benchmark::State& state)
{
    const auto size = _raster.size();
    const auto& ras = _raster;

    for (auto _ : state) {
        _sum = 0.0;
        for (int i = 0; i < size; ++i) {
            if (!ras.is_nodata(i)) {
                _sum += ras[i];
            }
        }

        verifySum();
    }
}

BENCHMARK_F(DenseNoDataIter, rawLoopRowCol)
(benchmark::State& state)
{
    const auto rows = _raster.rows();
    const auto cols = _raster.cols();
    const auto& ras = _raster;

    for (auto _ : state) {
        _sum = 0.0;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (!ras.is_nodata(r, c)) {
                    _sum += ras(r, c);
                }
            }
        }

        verifySum();
    }
}

BENCHMARK_F(DenseNoDataIter, nodataIter)
(benchmark::State& state)
{
    for (auto _ : state) {
        _sum = static_cast<double>(std::accumulate(value_cbegin(_raster), value_cend(_raster), 0, std::plus<int>()));
        verifySum();
    }
}

BENCHMARK_F(DenseNoDataIter, cellIter)
(benchmark::State& state)
{
    const auto& ras = _raster;

    for (auto _ : state) {
        _sum = 0.0;
        for (auto& cell : RasterCells(ras)) {
            if (!ras.is_nodata(cell)) {
                _sum += ras[cell];
            }
        }
        verifySum();
    }
}

static void algoSum(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    MaskedRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = gdx::sum(ras));
    }
}

static void algoSumIter(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    MaskedRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = gdx::ssum(ras));
    }
}

static void algoSumFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    MaskedRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1.f);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = gdx::sum(ras));
    }
}

static void algoSumIterFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    MaskedRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1.f);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = gdx::ssum(ras));
    }
}

BENCHMARK(algoSum)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK(algoSumIter)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK(algoSumFloat)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK(algoSumIterFloat)->Arg(10)->Arg(100)->Arg(1000);

BENCHMARK_MAIN();
