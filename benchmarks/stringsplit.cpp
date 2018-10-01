#include "infra/string.h"

#include <benchmark/benchmark.h>
#include <cstdlib>
#include <iostream>
#include <string.h>

using namespace infra;

static char s_sepchar[] = " \t;,";
static void split(std::vector<std::string>& list, const char* s)
{
    char* ptok;
    char* p = strdup(s);

    list.clear();

    if (!(ptok = strtok(p, s_sepchar))) return;
    list.push_back(ptok);
    while ((ptok = strtok(nullptr, s_sepchar)))
        list.push_back(ptok);

    free(p);
}

static std::vector<std::string_view> splitViewInline(std::string_view str, char delimiter)
{
    std::vector<std::string_view> tokens;
    size_t length     = 0;
    const char* start = str.data();
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == delimiter) {
            tokens.emplace_back(start, length);
            length = 0;

            if (i + 1 < str.size()) {
                start = &str[i + 1];
            }
        } else {
            ++length;
        }
    }

    if (length > 0) {
        tokens.emplace_back(start, length);
    }

    if (*start == delimiter) {
        tokens.emplace_back(std::string_view());
    }

    if (tokens.empty()) {
        tokens.emplace_back(str);
    }

    return tokens;
}

static const std::string s_inputStr = "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25";

static void strtoksplit(benchmark::State& state)
{
    for (auto _ : state) {
        std::vector<std::string> result;
        split(result, s_inputStr.c_str());

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

static void splittedView(benchmark::State& state)
{
    for (auto _ : state) {
        auto result = str::splitView(s_inputStr, " \t;,", str::SplitOpt::DelimiterIsCharacterArray);

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

static void splitted(benchmark::State& state)
{
    for (auto _ : state) {
        auto result = str::split(s_inputStr, " \t;,", str::SplitOpt::DelimiterIsCharacterArray);

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

static void splittedViewSingleChar(benchmark::State& state)
{
    for (auto _ : state) {
        auto result = str::splitView(s_inputStr, ' ');

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

static void splittedViewSingleCharInline(benchmark::State& state)
{
    for (auto _ : state) {
        auto result = splitViewInline(s_inputStr, ' ');

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

static void splitSingleChar(benchmark::State& state)
{
    for (auto _ : state) {
        auto result = str::split(s_inputStr, ' ');

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

class LocalSplitter
{
public:
    LocalSplitter(std::string_view src, std::string sep)
    : _src(src)
    , _sep(std::move(sep))
    {
    }

    class const_iterator
    {
    public:
        using value_type        = std::string_view;
        using pointer           = const value_type*;
        using reference         = const value_type&;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = int;

        explicit const_iterator(const LocalSplitter& sp)
        : _splitter(&sp)
        {
        }

        explicit const_iterator()
        : _splitter(nullptr)
        {
        }

        const_iterator& operator++() noexcept
        {
            if (!_splitter->finished()) {
                _value = _splitter->next();
            } else {
                _splitter = nullptr;
            }
            return *this;
        }

        const_iterator operator++(int) noexcept
        {
            const_iterator result(*this);
            ++(*this);
            return result;
        }

        reference operator*() const noexcept
        {
            return _value;
        }

        pointer operator->() const noexcept
        {
            return &_value;
        }

        bool operator!=(const const_iterator& other) const noexcept
        {
            return _splitter != other._splitter;
        }

    private:
        const LocalSplitter* _splitter;
        std::string_view _value;
    };

    const_iterator begin() const noexcept
    {
        return ++const_iterator(*this);
    }

    const_iterator end() const noexcept
    {
        return const_iterator();
    }

    std::string_view next() const noexcept
    {
        const auto endOfCurrent = _src.substr(_pos).find_first_of(_sep);
        if (endOfCurrent != std::string_view::npos && endOfCurrent != _src.size()) {
            // skip all occurrences of the delimeter
            const auto startOfNext = _src.substr(_pos + endOfCurrent).find_first_not_of(_sep);
            const auto old_pos     = _pos;
            _pos += endOfCurrent;

            if (startOfNext != std::string_view::npos) {
                _pos += startOfNext;
            } else {
                _finished = true;
            }

            return _src.substr(old_pos, endOfCurrent);
        } else {
            _finished = true;
            return _src.substr(_pos);
        }
    }

    bool finished() const noexcept
    {
        return _finished;
    }

private:
    std::string_view _src;
    std::string _sep;
    mutable size_t _pos    = 0;
    mutable bool _finished = false;
};

static void splitter(benchmark::State& state)
{
    for (auto _ : state) {
        std::vector<std::string_view> result;
        for (auto& tok : str::Splitter(s_inputStr, " \t;,")) {
            result.push_back(tok);
        }

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

static void splitterAlgo(benchmark::State& state)
{
    for (auto _ : state) {
        std::vector<std::string_view> result;
        for (auto& tok : LocalSplitter(s_inputStr, " \t;,")) {
            result.push_back(tok);
        }

        if (result.size() != 25) {
            std::exit(EXIT_FAILURE);
        }
    }
}

BENCHMARK(strtoksplit);
BENCHMARK(splittedView);
BENCHMARK(splitted);
BENCHMARK(splittedViewSingleChar);
BENCHMARK(splittedViewSingleCharInline);
BENCHMARK(splitSingleChar);
BENCHMARK(splitter);
BENCHMARK(splitterAlgo);

BENCHMARK_MAIN();
