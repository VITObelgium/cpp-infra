#pragma once

#include "../Exceptions.h"
#include <string>
#include <vector>

namespace inf {
class ConfigNode;
class ConfigDocument;
}

namespace opaq {

namespace XmlTools {
/**
     * Fetch an element from a given parent element
	 * @param parent parent element
	 * @param childName name of the child element
	 * @param refDoc pointer to ConfigDocument to use for the file ref. If this is set, the method will search for the 'ref' attribute in the child and if it is present, it will read the child element from the file given by the value of the ref attribute
	 */
inf::ConfigNode getElement(const inf::ConfigNode& parent, const std::string& childName, inf::ConfigDocument* refDoc = nullptr);
}
}
