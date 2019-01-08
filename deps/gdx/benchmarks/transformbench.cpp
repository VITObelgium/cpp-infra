#include "gdx/algo/algorithm.h"
#include "gdx/maskedraster.h"
#include "gdx/rasteriterator.h"
#include "gdx/rasterspan.h"
#include "infra/cast.h"

#include <benchmark/benchmark.h>
#include <iostream>
#include <numeric>

using namespace inf;
using namespace gdx;

static void stdtransformpure(benchmark::State& state)
{
    const auto size = state.range(0);

    std::vector<int> vec(size);
    for (auto _ : state) {
        std::transform(begin(vec), end(vec), begin(vec), [](auto& value) {
            return value * 2;
        });
    }
}

static void stdtransform(benchmark::State& state)
{
    const auto size = state.range(0);

    std::vector<int> vec(size);
    for (auto _ : state) {
        std::transform(begin(vec), end(vec), begin(vec), [](auto& value) {
            if (value != -9999) {
                return value;
            } else {
                return value * 2;
            }
        });
    }
}

static void gdxtransform(benchmark::State& state)
{
    const auto size = state.range(0);
    const auto dim  = truncate<int>(size);

    std::vector<int> vec(size);
    RasterMetadata meta(dim, dim, -9999);
    auto span = make_raster_span(vec, meta);

    for (auto _ : state) {
        gdx::transform(span, span, [](auto& value) {
            return value * 2;
        });
    }
}

static void gdxtransformmask(benchmark::State& state)
{
    const auto dim = truncate<int>(std::sqrt(state.range(0)));

    MaskedRaster<int> ras(RasterMetadata(dim, dim, -9999));

    for (auto _ : state) {
        gdx::transform(ras, ras, [](auto& value) {
            return value * 2;
        });
    }
}

#ifdef GDX_HAS_PARALLEL_ALGO

static void gdxtransformpar(benchmark::State& state)
{
    const auto size = state.range(0);

    std::vector<int> vec(size);
    auto span = make_value_span(vec, -9999);

    for (auto _ : state) {
        gdx::transform(std::execution::par_unseq, optionalvalue_begin(span), optionalvalue_end(span), optionalvalue_begin(span), [](auto& value) {
            return value * 2;
        });
    }
}

BENCHMARK(gdxtransformpar)->Arg(25)->Arg(100)->Arg(900000);

#endif

BENCHMARK(stdtransformpure)->Arg(25)->Arg(100)->Arg(900000);
BENCHMARK(stdtransform)->Arg(25)->Arg(100)->Arg(900000);
BENCHMARK(gdxtransform)->Arg(25)->Arg(100)->Arg(900000);
BENCHMARK(gdxtransformmask)->Arg(25)->Arg(100)->Arg(900000);

BENCHMARK_MAIN();
