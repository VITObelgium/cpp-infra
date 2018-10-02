#include "mapper.hpp"

namespace rio {

mapper::mapper(const config& cf, const std::shared_ptr<network>& net)
: _aggr(cf.aggr())
, _net(net)
, _detection_limit(cf.detection_limit())
, _missing_value(cf.missing_value())
{
    // get the generiao options from the <options> tag, which is present of all mappers
    auto optEl = cf.modelConfig().child("options");
    if (!optEl) {
        throw std::runtime_error("no <options> in the mapper configuration");
    }

    try {
        std::stringstream ss;
        optEl.print(ss);
        pt::read_xml(ss, _opts, pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments);
    } catch (...) {
        throw std::runtime_error("unable to parse options into tree");
    }
}

mapper::~mapper()
{
}

}
