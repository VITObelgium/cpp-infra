#include "XmlTools.h"
#include "infra/configdocument.h"
#include "tools/FileTools.h"

namespace opaq {
namespace XmlTools {

using namespace infra;

infra::ConfigNode getElement(const infra::ConfigNode& parent, const std::string& childName, infra::ConfigDocument* refDoc)
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

        *refDoc          = ConfigDocument::loadFromFile(ref);
        auto fileElement = refDoc->child(childName);
        if (!fileElement) {
            throw ElementNotFoundException("File in ref attribute ({}) does not have '{}' as root element", ref, childName);
        }

        return fileElement;
    }
}
}
}
