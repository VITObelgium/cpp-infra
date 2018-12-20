#include "ifdmwriter.hpp"
#include "parser.hpp"
#include "strfun.hpp"

namespace rio {

using namespace inf;


void ifdmwriter::fwrite_string(const std::string& s, unsigned int n)
{
    // make a copy and resize, length() doesnt count the nul, or prob does the resize
    
    // just trim, don't throw...
    //if (s.length() > n)
    //    throw std::runtime_error( "string too large for size given in ifdmwriter::fwrite_string : " + s );

    std::string str = s;
    str.resize(n, ' '); // pad with spaces to allow trimming in fortran
    fwrite(str.data(), n, 1, _fs);
    return;
}


ifdmwriter::ifdmwriter(const XmlNode& cnf)
: outputhandler(cnf)
, _pattern("rio_%timestamp%.txt")
, _version(1)
, _nt(0)
, _tmode(0)
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
                    std::cout << " Found ifdm griddef for grid " << cnf.grid() << std::endl;

                    _xul     = kv.second.get<float>("<xmlattr>.xul");
                    _yul     = kv.second.get<float>("<xmlattr>.yul");
                    _dx      = kv.second.get<float>("<xmlattr>.dx");
                    _dy      = kv.second.get<float>("<xmlattr>.dy");
                    _epsg    = kv.second.get<int>("<xmlattr>.epsg");
                    _missing = kv.second.get<int>("<xmlattr>.missing");

                    _nx = kv.second.get<int>("<xmlattr>.nx");
                    _ny = kv.second.get<int>("<xmlattr>.ny");

                    _mapfile_pattern = kv.second.get<std::string>("");
                    found            = true;
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
        throw std::runtime_error("error importing ifdm griddef file : " + mapfile);
    }

    if (_griddef.size() != _grid->size())
        throw std::runtime_error("ifdm griddef file does not match grid size, got:" +
                                 std::to_string(_griddef.size()) + ", expected : " + std::to_string(_grid->size()));

    // construct filename
    std::string filename = _pattern;
    rio::parser::get()->process(filename);

    std::cout << " Opening " << filename << "\n";
    _fs = std::fopen(filename.c_str(), "wb");
    if (!_fs)
        throw std::runtime_error("unable to open ifdm outputfile : " + filename);

    // get time information, cast to int to be really sure they are int's 
    // and not some funky boost date type
    int yr = static_cast<int>(cnf.start_time().date().year());
    int mn = static_cast<int>(cnf.start_time().date().month());
    int dy = static_cast<int>(cnf.start_time().date().day());

    int hour = static_cast<int>(cnf.start_time().time_of_day().hours());
    int min  = static_cast<int>(cnf.start_time().time_of_day().minutes());
    int sec  = static_cast<int>(cnf.start_time().time_of_day().seconds());

    int dt = 3600 * 24;
    if ( ! cnf.aggr().compare("1h") ) dt = 3600;

    // write header
    std::cout << " Writing header...\n";

    //1. set number of header bytes
    int nbytes = 15 * sizeof(int) + 4 * sizeof(float) + 98 * sizeof(char);
    fwrite(&nbytes, sizeof(int), 1, _fs);
    fwrite(&_version, sizeof(int), 1, _fs);    
    
    //2. write dimension information
    fwrite(&_nt, sizeof(int), 1, _fs); // 4 byte (32 bit) integers
    fwrite(&_nx, sizeof(int), 1, _fs);  
    fwrite(&_ny, sizeof(int), 1, _fs); 

    //3. write grid specifications
    fwrite(&_xul, sizeof(float), 1, _fs);  // 4 byte float (32 bit)
    fwrite(&_yul, sizeof(float), 1, _fs);

    fwrite(&_dx, sizeof(float), 1, _fs);
    fwrite(&_dy, sizeof(float), 1, _fs);

    fwrite(&_epsg, sizeof(int), 1, _fs);

    // 4. write timing information
    fwrite(&yr, sizeof(int),   1, _fs);
    fwrite(&mn, sizeof(int),   1, _fs);
    fwrite(&dy, sizeof(int),   1, _fs);
    fwrite(&hour, sizeof(int), 1, _fs);
    fwrite(&min, sizeof(int),  1, _fs);
    fwrite(&sec, sizeof(int),  1, _fs);
    fwrite(&_tmode, sizeof(int), 1, _fs);
    fwrite(&dt, sizeof(int),   1, _fs);

    // 5. missing value
    fwrite(&_missing, sizeof(int), 1, _fs);
    
    // 6. write optional run information : here we have 98 bytes
    fwrite_string(cnf.pol(), 4); 
    fwrite_string(cnf.aggr(), 4); 
    fwrite_string(cnf.ipol_class(), 10);
    fwrite_string(cnf.ipol(), 10);
    fwrite_string(cnf.configuration(), 10);
    fwrite_string(cnf.author(), 30);
    fwrite_string(cnf.email(), 30);

    // prepare databuffer for single map
    _buffer.resize(_nx * _ny );

    return;
}

void ifdmwriter::write(const boost::posix_time::ptime& /*curr_time*/,
    const std::map<std::string, double>& /*obs*/,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& uncert)
{

    // fill buffer with missing values
    std::fill(_buffer.begin(), _buffer.end(), static_cast<float>(_missing));

    // fill buffer with values
    for (unsigned int i = 0; i < values.size(); i++) {
        //unsigned int ix = (_griddef[i].row - 1) * _nx + (_griddef[i].col - 1);
        
        // this is the correct layout to ensure proper reading in the file
        unsigned int ix = (_griddef[i].col - 1) * _ny + (_griddef[i].row - 1);
        if (ix >= _buffer.size()) throw std::runtime_error("griddef map file indexes outside of bounds for gridcell " + std::to_string(i + 1) + " ...");
        _buffer[ix] = values(i);
    }

    // dump the map    
    fwrite(_buffer.data(), sizeof(float), _buffer.size(), _fs);    

    // increment the _nt
    _nt++;

    return;
}

void ifdmwriter::close(void)
{
    std::cout << "Closing ifdmwriter..." << std::endl;

    // return the position of the filepointer to the second field, first we have 
    // 4 bytes indicating the total length of the header in bytes, skip those...
    fseek(_fs, 8, SEEK_SET);
    fwrite(&_nt, sizeof(int), 1, _fs);

    std::fclose(_fs);
    return;
}
}
