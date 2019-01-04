#pragma once

#include <map>
#include <string>

namespace rio {

class parser
{
public:
    static parser* get();

    void process(std::string& s);
    void clear();
    void add_pattern(const std::string& key, const std::string& value);

private:
    parser() = default;

    std::map<std::string, std::string> _patterns;
};

}
