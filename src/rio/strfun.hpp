#pragma once

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace rio {

namespace strfun {

static constexpr int LINESIZE   = 1024;
static constexpr char SEPCHAR[] = " \t;,";

char* trim(char* str);

bool replace(std::string& str, const std::string& from, const std::string& to);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
void str2vec(std::vector<double>& x, std::string s);

}

}
