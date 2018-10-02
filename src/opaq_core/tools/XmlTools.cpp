#include "XmlTools.h"
#include "infra/xmldocument.h"
#include "tools/FileTools.h"

namespace opaq {
namespace XmlTools {

using namespace inf;

inf::XmlNode getElement(const inf::XmlNode& parent, const std::string& childName, inf::XmlDocument* refDoc)
{
    auto element = parent.child(childName);
    if (!element) {
        throw ElementNotFoundException("Child with name {} not found", childName);
    }

    if (refDoc == nullptr) {
        return element;
    } else {
        auto ref = std::string(element.attribute("ref"));
        if (ref.empty()) {
            // ref attribute not found
            return element;
        }

        if (!FileTools::exists(ref)) {
            throw ElementNotFoundException("File in ref attribute '{}' not found.", ref);
        }

        *refDoc          = XmlDocument::load_from_file(ref);
        auto fileElement = refDoc->child(childName);
        if (!fileElement) {
            throw ElementNotFoundException("File in ref attribute ({}) does not have '{}' as root element", ref, childName);
        }

        return fileElement;
    }
}
}
}
