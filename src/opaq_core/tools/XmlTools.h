#pragma once

#include "../Exceptions.h"
#include "../Logger.h"
#include <tinyxml.h>
#include <vector>

namespace OPAQ
{

namespace XmlTools
{
    /**
     * Fetch an element from a given parent element
	 * @param parent parent element
	 * @param childName name of the child element
	 * @param refDoc pointer to TiXmlDocument to use for the file ref. If this is set, the method will search for the 'ref' attribute in the child and if it is present, it will read the child element from the file given by the value of the ref attribute
	 */
    TiXmlElement* getElement(TiXmlElement* parent, const std::string& childName, TiXmlDocument* refDoc = nullptr);

    inline std::string getText(TiXmlElement* parent, const std::string& childName)
    {
        TiXmlElement* child = getElement(parent, childName);
        const char* text = child->GetText();
        return text ? text : "";
    }

    template <typename T>
    T getChildValue(TiXmlElement* parent, const char* childName)
    {
        auto* element = parent->FirstChildElement(childName);
        if (!element)
        {
            throw BadConfigurationException("{} element not found", childName);
        }

        T result;
        std::stringstream ss;
        ss << element->GetText();
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

    /**
	 * Fetch a child element from a given parent element
	 * where an list of attributes have a certain value...
	 * @param parent     parent element
	 * @param childName  name of child element
	 * @param attrName   vector of names for the attributes
	 * @param attrValue  vector of values for the attributes
	 * @param refDoc pointer to TiXmlDocument to use for the file ref. If this is set, the method will search for the 'ref' attribute in the child and if it is present, it will read the child element from the file given by the value of the ref attribute
	 */
    TiXmlElement* getElementByAttributes(TiXmlElement* parent, const std::string& childName,
                                                const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues,
                                                TiXmlDocument* refDoc = NULL);
}

}
