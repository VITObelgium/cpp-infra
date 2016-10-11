/*
 * GzipReader.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#pragma once

#include "../Exceptions.h"
#include "FileTools.h"
#include <fstream>
#include <memory>

#include <boost/iostreams/filtering_stream.hpp>

namespace OPAQ
{

class GzipReader
{
public:
    /** Open the gzip file
      * Throws IOException */
    void open(const std::string& filename);

    /** Reads a line from the file and returns as a std::string */
    std::string readLine();

    /** filepointer is at the end of the file */
    bool eof() const noexcept;

    /** close the input file */
    void close() noexcept;

private:
    std::ifstream _file;
    std::unique_ptr<boost::iostreams::filtering_istream> _filterStream;
};
}
