/*
 * FileTools.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#pragma once

#include <cstdio>
#include <string>

namespace opaq {
namespace FileTools {
bool exists(const std::string& filename);
bool remove(const std::string& filename);
}
}
