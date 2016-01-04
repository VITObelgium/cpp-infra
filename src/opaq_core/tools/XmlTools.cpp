/*
 * XmlTools.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "XmlTools.h"

namespace OPAQ {

XmlTools::XmlTools() {
}

XmlTools::~XmlTools() {
}

TiXmlElement* XmlTools::getElementByAttribute( TiXmlElement *parent,
		const std::string &childName, const std::string & attrName, const std::string & attrValue )
	throw ( ElementNotFoundException ) {

	if ( ! parent ) throw ElementNotFoundException("parent element does not exist");

	TiXmlElement *el = parent->FirstChildElement( childName );
	while( el ) {

		const char *val = el->Attribute( attrName.c_str() );
		if ( val ) {
			// we have the attribute
			if ( ! attrValue.compare( val ) ) return el;
		}

		el = el->NextSiblingElement( childName );
	}


	throw ElementNotFoundException("Did not find child with requested attribute/value pair");
}




TiXmlElement * XmlTools::getElement(TiXmlElement * parent,
		const std::string & childName, TiXmlDocument * refDoc)
				throw (ElementNotFoundException) {
	TiXmlElement * element = parent->FirstChildElement(childName);
	if (!element) {
		throw ElementNotFoundException(
				"Child with name " + childName + " not found");
	}
	if (refDoc != NULL) {
		std::string ref;
		if (element->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
			// ref attribute found
			if (FileTools::exists(ref)) {
				// file found
				if (!refDoc->LoadFile(ref)) {
					std::stringstream ss;
					ss << "Failed to load file in ref attribute: " << ref;
					throw ElementNotFoundException(ss.str());
				}
				TiXmlElement * fileElement = refDoc->FirstChildElement(childName);
				if (!fileElement) {
					std::stringstream ss;
					ss << "File in ref attribute (" << ref
							<< ") does not have '" << childName
							<< "' as root element";
					throw ElementNotFoundException(ss.str());
				}
				return fileElement;
			} else {
				std::stringstream ss;
				ss << "File in ref attribute '" << ref << "' not found.";
				throw ElementNotFoundException(ss.str());
			}
		} else {
			// ref attribute not found
			return element;
		}
	} else {
		return element;
	}
}

} /* namespace OPAQ */
