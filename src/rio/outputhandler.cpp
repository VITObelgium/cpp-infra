#include "outputhandler.hpp"


namespace rio
{

outputhandler::outputhandler( TiXmlElement *cnf )
{

    if ( ! cnf ) throw std::runtime_error( "invalid xml elemement pointer in output handler" );   
    std::stringstream ss;
    ss << (*cnf);
    pt::read_xml( ss, _xml, pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments );
}

outputhandler::~outputhandler()
{
}


}
