/*
 * XmlTools.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef XMLTOOLS_H_
#define XMLTOOLS_H_

#include <tinyxml.h>
#include "../Logger.h"
#include "../Exceptions.h"

namespace OPAQ {

class XmlTools {
public:
	XmlTools();
	virtual ~XmlTools();

	static std::string getText(TiXmlElement * parent,
			const std::string & childName)
					throw (ElementNotFoundException) {
		TiXmlElement * child = getElement(parent, childName);
		return std::string(child->GetText());
	}

	/**
	 * Fetch an element from a given parent element
	 * @param parent parent element
	 * @param childName name of the child element
	 * @param refDoc pointer to TiXmlDocument to use for the file ref. If this is set, the method will search for the 'ref' attribute in the child and if it is present, it will read the child element from the file given by the value of the ref attribute
	 */
	static TiXmlElement * getElement(TiXmlElement * parent,
			const std::string & childName,
			TiXmlDocument * refDoc = NULL) throw (ElementNotFoundException);

	/**
	 * Fetch a child element from a given parent element
	 * where an specific attribute has a certain value...
	 * @param parent     parent element
	 * @param childName  name of child element
	 * @param attrName   name of the attribute
	 * @param attrValue  value of the attribute
	 */
	static TiXmlElement * getElementByAttribute( TiXmlElement *parent,
			const std::string &childName, const std::string & attrName,
			const std::string & attrValue )
	throw (ElementNotFoundException);

};

} /* namespace OPAQ */
#endif /* XMLTOOLS_H_ */
