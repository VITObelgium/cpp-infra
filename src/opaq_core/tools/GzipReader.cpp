/*
 * GzipReader.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "GzipReader.h"
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include "StringTools.h"

namespace OPAQ
{

void GzipReader::open(const std::string& filename)
{
    close();

    _file.open(filename.c_str(), std::ios::binary);

    if (!_file.is_open())
    {
        throw IOException("File not found: {}", filename);
    }

    _filterStream = std::make_unique<boost::iostreams::filtering_istream>();
    if (boost::algorithm::ends_with(filename, ".gz"))
    {
        _filterStream->push(boost::iostreams::gzip_decompressor());
    }
    
    _filterStream->push(_file);
}

std::string GzipReader::readLine()
{
    std::string line;
    if (_filterStream)
    {
        try
        {
            std::getline(*_filterStream, line);
        }
        catch(const boost::iostreams::gzip_error& e)
        {
             throw RunTimeException("Failed to decompress line: {}", e.what());
        }
    }
    return line;
}

bool GzipReader::eof() const noexcept
{
    if (!_filterStream)
    {
        return true;

    }

    return _filterStream->eof();
}

void GzipReader::close() noexcept
{
    _filterStream.reset();
    _file.close();
}

}
