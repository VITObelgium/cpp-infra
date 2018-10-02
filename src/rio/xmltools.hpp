#pragma once

#include <vector>
#include <stdexcept>

#include <tinyxml.h>

namespace rio
{

namespace xml
{
    TiXmlElement* getElement(TiXmlElement* parent, const std::string& childName, TiXmlDocument* refDoc = nullptr);

    std::string getText(TiXmlElement* parent, const std::string& childName);

    std::string getChildValue(TiXmlElement* parent, const char* childName);
    std::string getChildValue(TiXmlElement* parent, const char* childName, const char* defaultValue);

    template <typename T>
    T getChildValue(TiXmlElement* parent, const char* childName)
    {
        T result;
        std::stringstream ss;
        ss << getChildValue(parent, childName);
        ss >> result;

        return result;
    }

    template <typename T>
    T getChildValue(TiXmlElement* parent, const char* childName, const T& defaultValue)
    {
        auto* element = parent->FirstChildElement(childName);
        if (!element)
        {
            return defaultValue;
        }

        T result;
        std::stringstream ss;
        ss << element->GetText();
        ss >> result;

        return result;
    }

    /**
	 * Fetch a child element from a given parent element
	 * where an specific attribute has a certain value...
	 * @param parent     parent element
	 * @param childName  name of child element
	 * @param attrName   name of the attribute
	 * @param attrValue  value of the attribute
	 * @param refDoc pointer to TiXmlDocument to use for the file ref. If this is set, the method will search for the 'ref' attribute in the child and if it is present, it will read the child element from the file given by the value of the ref attribute
	 */
    TiXmlElement* getElementByAttribute(TiXmlElement* parent, const std::string& childName,
                                               const std::string& attrName, const std::string& attrValue,
                                               TiXmlDocument* refDoc = NULL);


  // the following versions allow for separated lists of tags in the attribute value, e.g.
  // <pollutant name="pm10,no2,pm25"> etc...
  // the first routine is for a single attribute, the second for multiple attributes
  TiXmlElement* getElementByAttributeList(TiXmlElement* parent, const std::string& childName,
					  const std::string& attrName, const std::string& attrValue,
					  TiXmlDocument* refDoc = NULL);

  TiXmlElement* getElementByAttributesList(TiXmlElement* parent, const std::string& childName,
					   const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues,
					   TiXmlDocument* refDoc = NULL);

}

}
