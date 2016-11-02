/*
 * StringTools.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: vlooys
 */

#include "StringTools.h"

namespace opaq
{
namespace StringTools
{

const char* StringSplitter::WhiteSpaceSeparator = "\t\n\r\f";

static size_t findFirstDelimiter(const std::string& str, const char* delimiter, const int delimiterCount, size_t prev)
{
    size_t out = std::string::npos;
    for (int i = 0; i < delimiterCount; i++)
    {
        size_t pos         = str.find_first_of(delimiter[i], prev);
        if (pos < out) out = pos;
    }
    return out;
}

bool replace(std::string& str, const std::string& from,
             const std::string& to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

std::vector<std::string> tokenize(const std::string& source,
                                  const char* delimiter, const int delimiterCount, bool keepEmpty)
{
    std::vector<std::string> results;

    size_t prev = 0;
    size_t next = 0;

    while ((next = findFirstDelimiter(source, delimiter, delimiterCount, prev)) != std::string::npos)
    {
        if (keepEmpty || (next - prev != 0)) {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next + 1;
    }

    if (prev < source.size()) {
        results.push_back(source.substr(prev));
    }

    return results;
}

int find(char* list[], unsigned int listSize, const std::string& item)
{
    for (unsigned int i = 0; i < listSize; i++)
    {
        if (std::string(list[i]).compare(item) == 0)
            return i;
    }
    return -1; // not found
}

StringSplitter::StringSplitter(boost::string_ref src, std::string sep)
: _src(src)
, _sep(std::move(sep))
{
}

StringSplitter::const_iterator::const_iterator(const StringSplitter* sp)
: _splitter(sp)
{
}

StringSplitter::const_iterator::const_iterator()
: _splitter(nullptr)
{
}

StringSplitter::const_iterator& StringSplitter::const_iterator::operator++()
{
    if (!_splitter->finished())
        _value = _splitter->next();
    else
        _splitter = nullptr;
    return *this;
}

StringSplitter::const_iterator StringSplitter::const_iterator::operator++(int)
{
    StringSplitter::const_iterator result(*this);
    ++(*this);
    return result;
}

StringSplitter::const_iterator::reference StringSplitter::const_iterator::operator*()
{
    return _value;
}

StringSplitter::const_iterator::pointer StringSplitter::const_iterator::operator->()
{
    return &_value;
}

bool StringSplitter::const_iterator::operator !=(const StringSplitter::const_iterator& other) const noexcept
{
    return _splitter != other._splitter;
}

StringSplitter::const_iterator StringSplitter::begin() const noexcept
{
    return ++StringSplitter::const_iterator(this);
}

StringSplitter::const_iterator StringSplitter::end() const noexcept
{
    return StringSplitter::const_iterator();
}

boost::string_ref StringSplitter::next() const noexcept
{
    const auto endOfCurrent = _src.substr(_pos).find_first_of(_sep);
    if (endOfCurrent != std::string::npos && endOfCurrent != _src.size())
    {
        // skip all occurrences of the delimeter
        const auto startOfNext = _src.substr(_pos + endOfCurrent).find_first_not_of(_sep);
        const auto old_pos = _pos;
        _pos += endOfCurrent;

        if (startOfNext != std::string::npos)
        {
            _pos += startOfNext;
        }
        else
        {
            _finished = true;
        }

        return _src.substr(old_pos, endOfCurrent);
    }
    else
    {
        _finished = true;
        return _src.substr(_pos);
    }
}

bool StringSplitter::finished() const noexcept
{
    return _finished;
}

}
}
