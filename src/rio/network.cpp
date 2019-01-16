#include "network.hpp"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/log.h"
#include "infra/xmldocument.h"
#include "parser.hpp"
#include "strfun.hpp"
#include "xmltools.hpp"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace rio {

using namespace inf;

network::network(const XmlNode& el)
{
    if (!el) throw std::runtime_error("Invalid TiXmlElement pointer in network()...");

    std::string stationfile(el.child("filename").value());
    if (stationfile.empty()) {
        throw std::runtime_error("No <filename> in <stations> tag...");
    }

    rio::parser::get()->process(stationfile);

    _load_file(stationfile);
}

network::network(std::string stationfile)
{
    _load_file(stationfile);
}

void network::_load_file(std::string stationfile)
{
    _stations.clear();

    Log::debug("Importing {}", stationfile);

    file::Handle fp(stationfile.c_str(), "r");
    if (!fp.is_open()) {
        throw RuntimeError("Unable to open file: {}", stationfile);
    }

    char line[rio::strfun::LINESIZE];
    char* ptok;

    std::vector<double> proxy;

    // read header
    fgets(line, sizeof(line), fp);
    // read file
    while (fgets(line, sizeof(line), fp)) {
        if (rio::strfun::trim(line)[0] == '#') continue;
        if (!strlen(rio::strfun::trim(line))) continue;

        if (!(ptok = strtok(line, rio::strfun::SEPCHAR))) continue;
        int id = atoi(ptok);
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        std::string name = ptok;
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double x = atof(ptok);
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double y = atof(ptok);
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        double alt = atof(ptok);
        if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
        int type = atoi(ptok);

        // now read the proxy field...
        proxy.clear();
        while (ptok = strtok(NULL, rio::strfun::SEPCHAR))
            proxy.push_back(atof(ptok));

        // and fill
        station s(name, type, x, y, alt);
        s.setProxy(proxy);

        // add to the network
        _stations.push_back(s);
    }

    // perform some checks on the stations etc..
    Log::debug("Checking network for consistency...");
    int nproxy = -1;
    for (auto& s : _stations) {
        if (nproxy == -1)
            nproxy = static_cast<int>(s.proxy().size());
        else if (nproxy != s.proxy().size()) {
            throw RuntimeError("size of proxy vector  for {} does not match previous stations", s.name());
        }
    }
}

std::ostream& operator<<(std::ostream& out, const network& net)
{
    out << "network:" << std::endl;
    for (const auto& s : net._stations)
        out << s;

    return out;
}
}
