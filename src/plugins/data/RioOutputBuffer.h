#pragma once

#include "data/IMappingBuffer.h"

#include <H5Cpp.h>

namespace opaq {

class RioOutputBuffer : public IMappingBuffer
{
public:
    RioOutputBuffer();

    static std::string name();

    /**
     * <!-- created if it does not exist, replaced if it does -->
     * <filename>/path/to/datafile.h5</filename>
     */
    void configure(const inf::XmlNode& configuration, const std::string& componentName, IEngine& engine) override;

    void openResultsFile(chrono::date_time begin, chrono::date_time end,
        const Pollutant& pol, Aggregation::Type agg,
        const std::vector<Station>& stations, const Grid& grid, GridType gridType) override;
    void addResults(const std::vector<double>& results) override;
    void closeResultsFile() override;

private:
    void throwIfNotConfigured() const;

    size_t _index;
    H5::StrType _stringType;
    std::string _filePattern;
    std::unique_ptr<H5::H5File> _h5File;
};
}
