#include "output.hpp"

#include "apswriter.hpp"
#include "asciiwriter.hpp"
#include "h5writer.hpp"
#include "ifdmwriter.hpp"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/xmldocument.h"
#include "ircelwriter.hpp"
#include "jsonwriter.hpp"

#include "xmltools.hpp"

namespace rio {

using namespace inf;

output::output(const XmlNode& el, std::vector<std::string> req_outputs)
{
    if (!el) {
        throw std::runtime_error("Error in XmlNode in outputhandler::select_outputs");
    }

    for (const auto& req : req_outputs) {
        // lookup the element
        XmlNode outEl;
        try {
            outEl = rio::xml::getElementByAttribute(el, "handler", "name", req);
        } catch (...) {
            throw RuntimeError("Requested output type {} not available", req);
        }

        std::string req_class;
        try {
            req_class = outEl.attribute("class");
        } catch (...) {
            throw RuntimeError("output handler config for {} does not contain class attribute", req);
        }

        // check the class
        if (boost::equals(req_class, "asciiwriter")) {
            _list.push_back(std::make_unique<rio::asciiwriter>(outEl));
            std::cout << " Added asciiwriter to output list" << std::endl;

        } else if (boost::equals(req_class, "ircelwriter")) {
            _list.push_back(std::make_unique<rio::ircelwriter>(outEl));
            std::cout << " Added ircelwriter to output list" << std::endl;

        } else if (boost::equals(req_class, "h5writer")) {
            _list.push_back(std::make_unique<rio::h5writer>(outEl));
            std::cout << " Added h5writer to output list" << std::endl;

        } else if (boost::equals(req_class, "apswriter")) {
            _list.push_back(std::make_unique<rio::apswriter>(outEl));
            std::cout << " Added apswriter to output list" << std::endl;

        } else if (boost::equals(req_class, "jsonwriter")) {
            _list.push_back(std::make_unique<rio::jsonwriter>(outEl));
            std::cout << " Added jsonwriter to output list" << std::endl;

        } else if (boost::equals(req_class, "ifdmwriter")) {
            _list.push_back(std::make_unique<rio::ifdmwriter>(outEl));
            std::cout << " Added ifdmwriter to output list" << std::endl;

        } else {
            throw RuntimeError("Invalid output class requested: {}", req_class);
        }
    }
}

output::output(std::unique_ptr<rio::outputhandler> output)
{
    _list.push_back(std::move(output));
}

output::~output()
{
}

void output::init(const rio::config& cnf,
    const std::shared_ptr<rio::network> net,
    const std::shared_ptr<rio::grid> grid)
{
    for (const auto& w : _list) {
        w->init(cnf, net, grid);
    }
}

void output::write(const boost::posix_time::ptime& curr_time,
    const std::unordered_map<std::string, double>& obs,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& uncert)
{
    file::create_directory_if_not_exists("./output");

    for (const auto& w : _list) {
        w->write(curr_time, obs, values, uncert);
    }
}

void output::close(void)
{
    for (const auto& w : _list) {
        w->close();
    }
}
}
