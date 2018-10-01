#include "FileTools.h"
#include "../Exceptions.h"

#include <fstream>
#include <sstream>

namespace opaq {
namespace FileTools {

bool exists(const std::string& filename)
{
    // see https://stackoverflow.com/a/12774387
    if (FILE* file = fopen(filename.c_str(), "r")) {
        fclose(file);
        return true;
    }

    return false;
}

bool remove(const std::string& filename)
{
    if (exists(filename)) {
        return ::remove(filename.c_str()) == 0;
    }

    return true;
}
}
}
