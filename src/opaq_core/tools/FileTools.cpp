#include "FileTools.h"
#include "../Exceptions.h"

#include <fstream>
#include <sstream>
#include <experimental/filesystem>

namespace opaq
{
namespace FileTools
{

namespace fs = std::experimental::filesystem;

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
        throw RunTimeException("Failed to open file for reading: {}", filename);
    }

    return readStreamContents(fileStream);
}

void writeTextFile(const std::string& filename, const std::string& contents)
{
    std::ofstream fs(filename.c_str(), std::ios::trunc);
    if (!fs.is_open())
    {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    fs << contents;
}

bool exists(const std::string& filename)
{
    return fs::exists(filename);
}

bool del(const std::string& filename)
{
    return fs::remove(filename);
}

}
}
