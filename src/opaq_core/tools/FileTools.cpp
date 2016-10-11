/*
 * FileTools.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "FileTools.h"

#include <fstream>
#include <sstream>

namespace OPAQ
{
namespace FileTools
{

std::string readStreamContents(std::istream& stream)
{
    stream.seekg(0, std::ios::end);
    auto length = stream.tellg();
    stream.seekg(0, std::ios::beg);

    std::string buffer;
    buffer.resize(length);
    stream.read(&buffer[0], length);
    return buffer;
}

std::string readFileContents(const std::string& filename)
{
    std::ifstream fileStream(filename.c_str(), std::ifstream::binary);

    if (!fileStream.is_open())
    {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

    return readStreamContents(fileStream);
}

bool exists(const std::string& filename)
{
    /*
	 * see https://stackoverflow.com/a/12774387
	 */
    if (FILE* file = fopen(filename.c_str(), "r")) {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }
}

bool del(const std::string& filename)
{
    if (exists(filename)) {
        return remove(filename.c_str()) == 0;
    }
    else
    {
        return true;
    }
}

}
}
