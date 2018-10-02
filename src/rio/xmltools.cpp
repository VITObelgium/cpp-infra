/*
 * XmlTools.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */
#include "strfun.hpp"

#include "xmltools.hpp"

// list version
static bool _checkMatchList(TiXmlElement* el,
			    const std::vector<std::string>& attrNames,
			    const std::vector<std::string>& attrValues)
{

  unsigned int matches = 0;
  for (unsigned int i = 0; i < attrNames.size(); i++)
  {
    const char* val = el->Attribute(attrNames[i].c_str());
    if (val)
    {
      std::vector<std::string> lst;
      rio::strfun::split( lst, val );
      for ( const auto& s : lst )
	if ( ! attrValues[i].compare(s) ) { matches++; continue; }
    }
  }
  return (matches == attrNames.size());
}


bool exists(const std::string& filename)
{
    // see https://stackoverflow.com/a/12774387
    if (FILE* file = fopen(filename.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    
    return false;
}


namespace rio
{
namespace xml

{

std::string getText(TiXmlElement* parent, const std::string& childName)
{
    TiXmlElement* child = getElement(parent, childName);
    const char* text = child->GetText();
    return text ? text : "";
}

std::string getChildValue(TiXmlElement* parent, const char* childName)
{
    auto* element = parent->FirstChildElement(childName);
    if (!element)
    {
      throw std::runtime_error("Element not found : " + std::string( childName ) );
    }

    return element->GetText();
}

std::string getChildValue(TiXmlElement* parent, const char* childName, const char* defaultValue)
{
    auto* element = parent->FirstChildElement(childName);
    if (!element)
    {
        return defaultValue;
    }

    return element->GetText();
}

  

TiXmlElement* getElementByAttribute(TiXmlElement* parent, const std::string& childName,
                                    const std::string& attrName, const std::string& attrValue,
                                    TiXmlDocument* refDoc)
{

    if (!parent) throw std::runtime_error("parent element does not exist");

    TiXmlElement* el = parent->FirstChildElement(childName);
    if (!el) throw std::runtime_error("Element not found : " + std::string( childName ) );

    while (el)
    {

        if (refDoc != NULL) {

            std::string ref;
            if (el->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
                // ref attribute found
                if (exists(ref)) {
                    // file found
                    if (!refDoc->LoadFile(ref)) {
                        std::stringstream ss;
                        ss << "Failed to load file in ref attribute: " << ref;
                        throw std::runtime_error("Element not found : " + ss.str() );
                    }
                    TiXmlElement* fileElement = refDoc->FirstChildElement(childName);
                    if (!fileElement) {
                        std::stringstream ss;
                        ss << "File in ref attribute (" << ref
                           << ") does not have '" << childName
                           << "' as root element";
                        throw std::runtime_error("Element not found : " + ss.str() );
                    }

                    const char* val = fileElement->Attribute(attrName.c_str());
                    if (val)
                        if (!attrValue.compare(val)) return fileElement;
                }
                else
                {
                    std::stringstream ss;
                    ss << "File in ref attribute '" << ref << "' not found.";
                    throw std::runtime_error("Element not found : " + ss.str() );
                }
            }
            else
            {
                // ref attribute not found
                const char* val = el->Attribute(attrName.c_str());
                if (val)
                    if (!attrValue.compare(val)) return el;
            }
        }
        else
        {
            // -- no ref _doc, just check the attribute
            const char* val = el->Attribute(attrName.c_str());
            if (val)
                if (!attrValue.compare(val)) return el;
        }

        el = el->NextSiblingElement(childName);
    }

    throw std::runtime_error("Did not find child with requested attribute/value pair");
}

TiXmlElement* getElementByAttributeList(TiXmlElement* parent, const std::string& childName,
					const std::string& attrName, const std::string& attrValue,
					TiXmlDocument* refDoc)
{

    if (!parent) throw std::runtime_error("parent element does not exist");

    TiXmlElement* el = parent->FirstChildElement(childName);
    if (!el) throw std::runtime_error("Element not found : " + std::string( childName ) );

    while (el)
    {

        if (refDoc != NULL) {

            std::string ref;
            if (el->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
                // ref attribute found
                if (exists(ref)) {
                    // file found
                    if (!refDoc->LoadFile(ref)) {
                        std::stringstream ss;
                        ss << "Failed to load file in ref attribute: " << ref;
                        throw std::runtime_error("Element not found : " + ss.str() );
                    }
                    TiXmlElement* fileElement = refDoc->FirstChildElement(childName);
                    if (!fileElement) {
                        std::stringstream ss;
                        ss << "File in ref attribute (" << ref
                           << ") does not have '" << childName
                           << "' as root element";
                        throw std::runtime_error("Element not found : " + ss.str() );
                    }

                    const char* val = fileElement->Attribute(attrName.c_str());
                    if ( val )
		    {
		      // check the list of the attribute... tokenize val as commma separated values
		      std::vector<std::string> lst;
		      rio::strfun::split( lst, val );
		      for ( const auto& s : lst )
			if (!attrValue.compare(s)) return fileElement;
		    }
                }
                else
                {
                    std::stringstream ss;
                    ss << "File in ref attribute '" << ref << "' not found.";
                    throw std::runtime_error("Element not found : " + ss.str() );
                }
            }
            else
            {
                // ref attribute not found
                const char* val = el->Attribute(attrName.c_str());
		if ( val )
		{
		  // check the list of the attribute... tokenize val as commma separated values
		  std::vector<std::string> lst;
		  rio::strfun::split( lst, val );
		  for ( const auto& s : lst )
		    if (!attrValue.compare(s)) return el;
		}
            }
        }
        else
        {
            // -- no ref _doc, just check the attribute
	    const char* val = el->Attribute(attrName.c_str());
	    if ( val )
	    {
	      std::cout << "Checking attribute : " << val << " for " << attrValue << std::endl;
	      // check the list of the attribute... tokenize val as commma separated values
	      std::vector<std::string> lst;
	      rio::strfun::split( lst, val );
	      for ( const auto& s : lst )
		if (!attrValue.compare(s)) return el;
	    }
	
        }

        el = el->NextSiblingElement(childName);
    }

    throw std::runtime_error("Did not find child with requested attribute/value pair");
}


TiXmlElement* getElementByAttributesList(TiXmlElement* parent, const std::string& childName,
					 const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues,
					 TiXmlDocument* refDoc )
{

    if (!parent) throw std::runtime_error("parent element does not exist");
    if (attrValues.size() != attrNames.size()) throw std::runtime_error("attrValues doesnt match attrNames");
    if (attrValues.size() == 0) throw std::runtime_error("no attributes given...");

    TiXmlElement* el = parent->FirstChildElement(childName);
    if (!el) throw std::runtime_error("Element not found : " + std::string( childName ) );

    while (el)
    {

        if (refDoc != NULL) {

            std::string ref;
            if (el->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
                // ref attribute found
                if (exists(ref)) {
                    // file found
                    if (!refDoc->LoadFile(ref)) {
                        std::stringstream ss;
                        ss << "Failed to load file in ref attribute: " << ref;
                        throw std::runtime_error("Element not found : " + ss.str() );
                    }
                    TiXmlElement* fileElement = refDoc->FirstChildElement(childName);
                    if (!fileElement) {
                        std::stringstream ss;
                        ss << "File in ref attribute (" << ref
                           << ") does not have '" << childName
                           << "' as root element";
                        throw std::runtime_error("Element not found : " + ss.str() );
                    }

                    if (_checkMatchList(fileElement, attrNames, attrValues)) return fileElement;
                }
                else
                {
                    std::stringstream ss;
                    ss << "File in ref attribute '" << ref << "' not found.";
                    throw std::runtime_error("Element not found : " + ss.str() );
                }
            }
            else
            {
                // ref attribute not found
                if (_checkMatchList(el, attrNames, attrValues)) return el;
            }
        }
        else
        {
            // -- no ref _doc, just check the attribute
            if (_checkMatchList(el, attrNames, attrValues)) return el;
        }

        el = el->NextSiblingElement(childName);
    }

    throw std::runtime_error("Did not find child with requested attribute/value pair");
}



  

TiXmlElement* getElement(TiXmlElement* parent, const std::string& childName, TiXmlDocument* refDoc)
{
    TiXmlElement* element = parent->FirstChildElement(childName);
    if (!element) {
        throw std::runtime_error("Element not found : " + std::string( childName ) );

    }
    if (refDoc != NULL) {
        std::string ref;
        if (element->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
            // ref attribute found
            if (exists(ref)) {
                // file found
                if (!refDoc->LoadFile(ref)) {
                    std::stringstream ss;
                    ss << "Failed to load file in ref attribute: " << ref;
                    throw std::runtime_error("Element not found : " + ss.str() );
                }
                TiXmlElement* fileElement = refDoc->FirstChildElement(childName);
                if (!fileElement) {
                    std::stringstream ss;
                    ss << "File in ref attribute (" << ref
                       << ") does not have '" << childName
                       << "' as root element";
                    throw std::runtime_error("Element not found : " + ss.str() );
                }
                return fileElement;
            }
            else
            {
                std::stringstream ss;
                ss << "File in ref attribute '" << ref << "' not found.";
                throw std::runtime_error("Element not found : " + ss.str() );
            }
        }
        else
        {
            // ref attribute not found
            return element;
        }
    }
    else
    {
        return element;
    }
}
}
}
