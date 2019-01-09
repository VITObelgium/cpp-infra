#include "outputhandler.hpp"

namespace rio {

using namespace inf;

outputhandler::outputhandler()
{
}

outputhandler::outputhandler(const XmlNode& cnf)
{
    if (!cnf) {
        throw std::runtime_error("invalid xml elemement pointer in output handler");
    }

    std::stringstream ss;
    cnf.print(ss);
    pt::read_xml(ss, _xml, pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments);
}

}
