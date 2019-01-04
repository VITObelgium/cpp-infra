#include "parser.hpp"
#include "strfun.hpp"

namespace rio {

parser* parser::get()
{
    static parser instance;
    return &instance;
}

void parser::process(std::string& s)
{
    for (const auto& it : _patterns)
        strfun::replaceAll(s, it.first, it.second);
}

void parser::clear()
{
    _patterns.clear();
}

void parser::add_pattern(const std::string& key, const std::string& value)
{
    _patterns[key] = value;
}

}
