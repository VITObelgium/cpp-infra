#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "station.hpp"

namespace inf {
class XmlNode;
}

namespace rio {

class network
{
public:
    network() = default;
    network(const inf::XmlNode& el);
    network(std::string stationfile);

    std::shared_ptr<rio::station const> get(const std::string& name) const
    {
        for (const rio::station& s : _stations)
            if (!s.name().compare(name)) return std::make_shared<rio::station const>(s);
        return nullptr;
    }

    size_t size(void)
    {
        return _stations.size();
    }
    const std::vector<rio::station>& st_list(void) const
    {
        return _stations;
    }

public:
    friend std::ostream& operator<<(std::ostream& out, const network& net);

private:
    std::vector<rio::station> _stations;

private:
    void _load_file(std::string stationfile);
};

}
