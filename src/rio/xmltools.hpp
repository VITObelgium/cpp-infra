#pragma once

#include "infra/xmldocument.h"

#include <stdexcept>
#include <vector>

namespace rio::xml {

/**
	 * Fetch a child element from a given parent element
	 * where an specific attribute has a certain value...
	 * @param parent     parent element
	 * @param childName  name of child element
	 * @param attrName   name of the attribute
	 * @param attrValue  value of the attribute
	 * @param refDoc pointer to TiXmlDocument to use for the file ref. If this is set, the method will search for the 'ref' attribute in the child and if it is present, it will read the child element from the file given by the value of the ref attribute
	 */
inf::XmlNode getElementByAttribute(const inf::XmlNode& parent, const std::string& childName, const std::string& attrName, const std::string& attrValue, inf::XmlDocument* refDoc = nullptr);

// the following versions allow for separated lists of tags in the attribute value, e.g.
// <pollutant name="pm10,no2,pm25"> etc...
// the first routine is for a single attribute, the second for multiple attributes
inf::XmlNode getElementByAttributesList(const inf::XmlNode& parent, const std::string& childName,
    const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues,
    inf::XmlDocument* refDoc = nullptr);

}
