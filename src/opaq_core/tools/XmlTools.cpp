/*
 * XmlTools.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "XmlTools.h"

// small helper function to check for the mattching of attributes
static bool _checkMatch(TiXmlElement* el, const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues)
{

    unsigned int matches = 0;
    for (unsigned int i = 0; i < attrNames.size(); i++)
    {
        const char* val = el->Attribute(attrNames[i].c_str());
        if (val)
            if (!attrValues[i].compare(attrValues[i])) matches++;
    }
    return (matches == attrNames.size());
}

namespace OPAQ
{
namespace XmlTools
{

TiXmlElement* getElementByAttribute(TiXmlElement* parent, const std::string& childName,
                                    const std::string& attrName, const std::string& attrValue,
                                    TiXmlDocument* refDoc)
{

    if (!parent) throw ElementNotFoundException("parent element does not exist");

    TiXmlElement* el = parent->FirstChildElement(childName);
    if (!el) throw ElementNotFoundException("Child with name " + childName + " not found");

    while (el)
    {

        if (refDoc != NULL) {

            std::string ref;
            if (el->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
                // ref attribute found
                if (FileTools::exists(ref)) {
                    // file found
                    if (!refDoc->LoadFile(ref)) {
                        std::stringstream ss;
                        ss << "Failed to load file in ref attribute: " << ref;
                        throw ElementNotFoundException(ss.str());
                    }
                    TiXmlElement* fileElement = refDoc->FirstChildElement(childName);
                    if (!fileElement) {
                        std::stringstream ss;
                        ss << "File in ref attribute (" << ref
                           << ") does not have '" << childName
                           << "' as root element";
                        throw ElementNotFoundException(ss.str());
                    }

                    const char* val = fileElement->Attribute(attrName.c_str());
                    if (val)
                        if (!attrValue.compare(val)) return fileElement;
                }
                else
                {
                    std::stringstream ss;
                    ss << "File in ref attribute '" << ref << "' not found.";
                    throw ElementNotFoundException(ss.str());
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

    throw ElementNotFoundException("Did not find child with requested attribute/value pair");
}

TiXmlElement* getElementByAttributes(TiXmlElement* parent, const std::string& childName,
                                     const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues,
                                     TiXmlDocument* refDoc)
{

    if (!parent) throw ElementNotFoundException("parent element does not exist");
    if (attrValues.size() != attrNames.size()) throw InvalidArgumentsException("attrValues doesnt match attrNames");
    if (attrValues.size() == 0) throw InvalidArgumentsException("no attributes given...");

    TiXmlElement* el = parent->FirstChildElement(childName);
    if (!el) throw ElementNotFoundException("Child with name " + childName + " not found");

    while (el)
    {

        if (refDoc != NULL) {

            std::string ref;
            if (el->QueryStringAttribute("ref", &ref) == TIXML_SUCCESS) {
                // ref attribute found
                if (FileTools::exists(ref)) {
                    // file found
                    if (!refDoc->LoadFile(ref)) {
                        std::stringstream ss;
                        ss << "Failed to load file in ref attribute: " << ref;
                        throw ElementNotFoundException(ss.str());
                    }
                    TiXmlElement* fileElement = refDoc->FirstChildElement(childName);
                    if (!fileElement) {
                        std::stringstream ss;
                        ss << "File in ref attribute (" << ref
                           << ") does not have '" << childName
                           << "' as root element";
                        throw ElementNotFoundException(ss.str());
                    }

                    if (_checkMatch(fileElement, attrNames, attrValues)) return fileElement;
                }
                else
                {
                    std::stringstream ss;
                    ss << "File in ref attribute '" << ref << "' not found.";
                    throw ElementNotFoundException(ss.str());
                }
            }
            else
            {
                // ref attribute not found
                if (_checkMatch(el, attrNames, attrValues)) return el;
            }
        }
        else
        {
            // -- no ref _doc, just check the attribute
            if (_checkMatch(el, attrNames, attrValues)) return el;
        }

        el = el->NextSiblingElement(childName);
    }

    throw ElementNotFoundException("Did not find child with requested attribute/value pair");
}

TiXmlElement* getElement(TiXmlElement* parent, const std::string& childName, TiXmlDocument* refDoc)
{
    TiXmlElement* element = parent->FirstChildElement(childName);
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
                TiXmlElement* fileElement = refDoc->FirstChildElement(childName);
                if (!fileElement) {
                    std::stringstream ss;
                    ss << "File in ref attribute (" << ref
                       << ") does not have '" << childName
                       << "' as root element";
                    throw ElementNotFoundException(ss.str());
                }
                return fileElement;
            }
            else
            {
                std::stringstream ss;
                ss << "File in ref attribute '" << ref << "' not found.";
                throw ElementNotFoundException(ss.str());
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
