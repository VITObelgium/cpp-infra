#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/string.h"
#include "strfun.hpp"

#include "xmltools.hpp"

using namespace inf;

// list version
static bool _checkMatchList(const XmlNode& el, const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues)
{
    unsigned int matches = 0;
    for (size_t i = 0; i < attrNames.size(); ++i) {
        auto val = el.attribute(attrNames[i]);
        if (!val.empty()) {
            auto lst = str::split_view(val, " \t;,", str::StrTokFlags);
            for (const auto& s : lst) {
                if (s == attrValues[i]) {
                    matches++;
                    continue;
                }
            }
        }
    }
    return (matches == attrNames.size());
}

namespace rio::xml {

XmlNode getElementByAttribute(const inf::XmlNode& parent, const std::string& childName, const std::string& attrName, const std::string& attrValue, XmlDocument* refDoc)
{
    if (!parent) {
        throw std::runtime_error("parent element does not exist");
    }

    for (auto& el : parent.children(childName)) {
        if (refDoc) {
            auto refPath = fs::path(std::string(el.attribute("ref")));
            if (!refPath.empty()) {
                // ref attribute found
                if (fs::exists(refPath)) {
                    // file found
                    *refDoc = XmlDocument::load_from_file(refPath.string());
                    if (!(*refDoc)) {
                        throw RuntimeError("Element not found: Failed to load file in ref attribute '{}'", refPath);
                    }

                    auto fileElement = refDoc->child(childName);
                    if (!fileElement) {
                        throw RuntimeError("Element not found: File in ref attribute ({}) does not have '{}' as root element", refPath, childName);
                    }

                    if (fileElement.attribute(attrName) == attrValue) {
                        return fileElement;
                    }
                } else {
                    throw RuntimeError("Element not found: File in ref attribute '{}' not found", refPath);
                }
            } else {
                // ref attribute not found
                if (el.attribute(attrName) == attrValue) {
                    return el;
                }
            }
        } else {
            // -- no ref _doc, just check the attribute
            if (el.attribute(attrName) == attrValue) {
                return el;
            }
        }
    }

    throw RuntimeError("Did not find child with requested attribute/value pair {}:{}", childName, attrName);
}

inf::XmlNode getElementByAttributesList(const inf::XmlNode& parent, const std::string& childName,
    const std::vector<std::string>& attrNames, const std::vector<std::string>& attrValues,
    inf::XmlDocument* refDoc)
{
    if (!parent) {
        throw std::runtime_error("parent element does not exist");
    }

    if (attrValues.size() != attrNames.size()) {
        throw std::runtime_error("attrValues doesnt match attrNames");
    }

    if (attrValues.empty()) {
        throw std::runtime_error("no attributes given...");
    }

    for (auto& el : parent.children(childName)) {
        if (refDoc != nullptr) {
            auto refPath = fs::path(std::string(el.attribute("ref")));
            if (!refPath.empty()) {
                // ref attribute found
                if (fs::exists(refPath)) {
                    // file found
                    *refDoc = XmlDocument::load_from_file(refPath.string());
                    if (!(*refDoc)) {
                        throw RuntimeError("Element not found: Failed to load file in ref attribute '{}'", refPath);
                    }

                    auto fileElement = refDoc->child(childName);
                    if (!fileElement) {
                        throw RuntimeError("Element not found: File in ref attribute ({}) does not have '{}' as root element", refPath, childName);
                    }

                    if (_checkMatchList(fileElement, attrNames, attrValues)) {
                        return fileElement;
                    }
                } else {
                    throw RuntimeError("Element not found: File in ref attribute '{}' not found", refPath);
                }
            } else {
                // ref attribute not found
                if (_checkMatchList(el, attrNames, attrValues)) {
                    return el;
                }
            }
        } else {
            // -- no ref _doc, just check the attribute
            if (_checkMatchList(el, attrNames, attrValues)) {
                return el;
            }
        }
    }

    throw std::runtime_error("Did not find child with requested attribute/value pair");
}
}
