#include "ifdmwriter.hpp"
#include "parser.hpp"
#include "strfun.hpp"

namespace rio {

using namespace inf;

ifdmwriter::ifdmwriter(const XmlNode& cnf)
: outputhandler(cnf)
, _pattern("rio_%timestamp%.txt")
, _nt(0)
{
    try {
        _pattern = _xml.get<std::string>("handler.location");
    } catch (...) {
        throw std::runtime_error("invalid configuration in ifdmwriter XML config, <location> required");
    }

   
}

ifdmwriter::~ifdmwriter()
{
}

void ifdmwriter::init(const rio::config& cnf,
    const std::shared_ptr<rio::network> net,
    const std::shared_ptr<rio::grid> grid)
{
    // do some parsing of cnf.getOutput();
    _net  = net;
    _grid = grid;

     try {
        // lookup the
        bool found = false;
        for (const auto& kv : _xml.get_child("handler")) {
            //std::cout << "checking aps element : " << kv.first << std::endl;
            if (!kv.first.compare("griddef")) {
                if (!kv.second.get<std::string>("<xmlattr>.grid").compare(cnf.grid())) {
                    std::cout << "  Found IFDM griddef for grid " << cnf.grid() << std::endl;

                    _xull    = kv.second.get<float>("<xmlattr>.xull");
                    _yull    = kv.second.get<float>("<xmlattr>.yull");
                    _dx      = kv.second.get<float>("<xmlattr>.dx");
                    _dy      = kv.second.get<float>("<xmlattr>.dy");
                    _epsg    = kv.second.get<float>("<xmlattr>.epsg");
                    _missing = kv.second.get<float>("<xmlattr>.missing");

                    _nx = kv.second.get<int>("<xmlattr>.nx");
                    _ny = kv.second.get<int>("<xmlattr>.ny");

                    _mapfile_pattern = kv.second.get<std::string>("");
                    found                  = true;
                }
            }
        }
        if (!found) throw std::runtime_error("no matching <griddef> element for selected grid found... ");

    } catch (...) {
        throw std::runtime_error("error in ifdmwriter, no appropriate <griddef> for selected grid required");
    }

    // now import the griddef file and setup the mapping arrays
    std::string mapfile = _mapfile_pattern;
    rio::parser::get()->process(mapfile);
    _griddef.clear();
    try {
        FILE* fp = fopen(mapfile.c_str(), "r");
        if (!fp) throw std::runtime_error("Unable to open file: " + mapfile);

        char line[rio::strfun::LINESIZE];
        char* ptok;
        // read header
        fgets(line, sizeof(line), fp);
        // read file
        while (fgets(line, sizeof(line), fp)) {
            grididx_t idx;

            if (rio::strfun::trim(line)[0] == '#') continue;
            if (!strlen(rio::strfun::trim(line))) continue;

            if (!(ptok = strtok(line, rio::strfun::SEPCHAR))) continue;
            if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
            idx.row = atoi(ptok);
            if (!(ptok = strtok(NULL, rio::strfun::SEPCHAR))) continue;
            idx.col = atoi(ptok);

            _griddef.push_back(idx);
        }

        fclose(fp);
    } catch (...) {
        throw std::runtime_error("error importing aps mapping file : " + mapfile);
    }

    if (_griddef.size() != _grid->size()) 
        throw std::runtime_error("ifdm griddef file does not match grid size, got:" +
                                 std::to_string(_griddef.size()) + ", expected : " + std::to_string(_grid->size()));




    _fs = std::fopen( "ifdm_input.bin",  "wb" );
    if (!_fs)
        throw std::runtime_error("unable to open ifdm outputfile...");
   
    int num  = 100;
    int nx = 59;
    int ny = 67;
    float dx = 4000.;
    float dy = 4000.;


    // write header
    std::cout << "wrote : " << fwrite(&num, sizeof(int), 1, _fs) << " bytes...\n";
    std::cout << "wrote : " << fwrite(&nx, sizeof(int), 1, _fs) << " bytes...\n";
    std::cout << "wrote : " << fwrite(&ny, sizeof(int), 1, _fs) << " bytes...\n";
    std::cout << "wrote : " << fwrite(&dx, sizeof(float), 1, _fs) << " bytes...\n";
    std::cout << "wrote : " << fwrite(&dy, sizeof(float), 1, _fs) << " bytes...\n";

    return;
}

void ifdmwriter::write(const boost::posix_time::ptime& /*curr_time*/,
    const std::map<std::string, double>& /*obs*/,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& uncert)
{
    // dump the map



    // increment the _nt
    _nt++;

    return;
}

void ifdmwriter::close(void)
{
    std::cout << "Closing ifdmwriter..." << std::endl;
    
    // updating the number of entries, depending on the number of maps produced...
    rewind(_fs);
    int num = 50;
    fwrite(&num, sizeof(int), 1, _fs);
        
    std::fclose( _fs );
    return;
}

}
