#include "jsonwriter.hpp"
#include "parser.hpp"

namespace rio {

using namespace inf;

jsonwriter::jsonwriter(const XmlNode& cnf)
: outputhandler(cnf)
, _pattern("rio_%timestamp%.json")
{
    try {
        _pattern = _xml.get<std::string>("handler.location");
    } catch (...) {
        throw std::runtime_error("invalid configuration in jsonwriter XML config, <location> required");
    }
}

jsonwriter::~jsonwriter()
{
}

void jsonwriter::init(const rio::config& cnf,
    const std::shared_ptr<rio::network> net,
    const std::shared_ptr<rio::grid> grid)
{
    // do some parsing of cnf.getOutput();
    _polName = cnf.pol();
    _net     = net;
    _grid    = grid;

    return;
}

void jsonwriter::write(const boost::posix_time::ptime& curr_time,
    const std::map<std::string, double>& /*obs*/,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& uncert)
{
    // dump the map
    std::string ofname = _pattern;
    rio::parser::get()->process(ofname);

    FILE* fp = fopen(ofname.c_str(), "wt");
    fprintf(fp, "{\n");
    fprintf(fp, "  \"type\": \"FeatureCollection\",\n");
    fprintf(fp, "  \"features\": [\n");
    for (size_t i = 0; i < _grid->size(); i++) {
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"type\" : \"Feature\",\n");
        fprintf(fp, "      \"properties\": {\n");
        fprintf(fp, "        \"id\": %zd,\n", _grid->cells()[i].id());
        fprintf(fp, "        \"pollutant\": \"%s\",\n", _polName.c_str());
        fprintf(fp, "        \"timestamp\" : \"%s\",\n", boost::posix_time::to_iso_string(curr_time).c_str());
        fprintf(fp, "        \"concentration\": %f,\n", values[i]);
        fprintf(fp, "        \"uncertainty\": %f\n", uncert[i]);
        fprintf(fp, "      },\n");
        fprintf(fp, "      \"geometry\": {\n");
        fprintf(fp, "        \"type\": \"Polygon\",\n");
        fprintf(fp, "        \"coordinates\": [ [ [%.2f, %.2f], [%.2f, %.2f], [%.2f, %.2f], [%.2f, %.2f], [%.2f, %.2f] ] ]\n",
            _grid->cells()[i].xmin(), _grid->cells()[i].ymin(),
            _grid->cells()[i].xmax(), _grid->cells()[i].ymin(),
            _grid->cells()[i].xmax(), _grid->cells()[i].ymax(),
            _grid->cells()[i].xmin(), _grid->cells()[i].ymax(),
            _grid->cells()[i].xmin(), _grid->cells()[i].ymin());
        fprintf(fp, "      }\n");
        if (i == _grid->size() - 1)
            fprintf(fp, "    }\n");
        else
            fprintf(fp, "    },\n");
    }
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    fclose(fp);

    return;
}

void jsonwriter::close(void)
{
    std::cout << "Closing jsonwriter..." << std::endl;
    return;
}

}
