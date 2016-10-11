/*
 * FileTools.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#pragma once

#include <cstdio>
#include <string>

namespace OPAQ
{
namespace FileTools
{
    std::string readStreamContents(std::istream& stream);
    std::string readFileContents(const std::string& filename);
    bool exists(const std::string& filename);
    bool del(const std::string& filename);
}
}
