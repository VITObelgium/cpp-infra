/*
 * StringTools.h
 *
 *  Created on: Feb 26, 2014
 *      Author: vlooys
 */

#pragma once

#include <string>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/string_ref.hpp>

namespace opaq {
namespace StringTools {

bool replace(std::string& str, const std::string& from, const std::string& to);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> tokenize(std::string_view source, const char* delimiter = " \t\n\r\f", const int delimiterCount = 5, bool keepEmpty = false);
int find(char* list[], unsigned int listSize, const std::string& item);

class StringSplitter
{
public:
    static const char* WhiteSpaceSeparator;

    StringSplitter(boost::string_ref src, std::string sep);

    // avoid copying strings during construction
    StringSplitter(const std::string&& src, boost::string_ref sep) = delete;

    class const_iterator : public boost::iterator_facade<const_iterator, const boost::string_ref, boost::single_pass_traversal_tag>
    {
    public:
        explicit const_iterator(const StringSplitter* sp);
        explicit const_iterator();

        const_iterator& operator++();
        const_iterator operator++(int);

        reference operator*();
        pointer operator->();

        bool operator!=(const const_iterator& other) const noexcept;

    private:
        const StringSplitter* _splitter;
        boost::string_ref _value;
    };

    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;

    boost::string_ref next() const noexcept;
    bool finished() const noexcept;

private:
    boost::string_ref _src;
    std::string _sep;
    mutable size_t _pos    = 0;
    mutable bool _finished = false;
};
}
}
