#include "infra/charset.h"

#include "infra/cast.h"
#include "infra/exception.h"

#include <unicode/ucsdet.h>
#include <unicode/unistr.h>

namespace inf {

CharacterSet detect_character_set(const fs::path& path)
{
    auto data = file::read_as_text(path);
    return detect_character_set_from_data(std::string_view(data));
}

CharacterSet detect_character_set_from_data(const std::string_view data)
{
    UErrorCode icuError = U_ZERO_ERROR;
    LocalUCharsetDetectorPointer charsetDetector(ucsdet_open(&icuError));
    if (U_FAILURE(icuError)) {
        return CharacterSet::Unknown;
    }

    // send text
    ucsdet_setText(charsetDetector.getAlias(), data.data(), truncate<int32_t>(data.size()), &icuError);
    if (U_FAILURE(icuError)) {
        return CharacterSet::Unknown;
    }

    // detect language
    auto* charsetMatch = ucsdet_detect(charsetDetector.getAlias(), &icuError);
    if (U_FAILURE(icuError)) {
        return CharacterSet::Unknown;
    }

    std::string_view name = ucsdet_getName(charsetMatch, &icuError);
    if (U_FAILURE(icuError)) {
        return CharacterSet::Unknown;
    }

    if (name == "ISO-8859-1") {
        return CharacterSet::ISO_8859_1;
    } else if (name == "UTF-8") {
        return CharacterSet::Utf8;
    } else if (name == "UTF-16LE") {
        return CharacterSet::Utf16LE;
    } else if (name == "UTF-16BE") {
        return CharacterSet::Utf16BE;
    }

    return CharacterSet::Unknown;
}

std::string convert_to_utf8(std::string_view input)
{
    std::string result;
    icu::UnicodeString(input.data(), input.size(), US_INV).toUTF8String(result);
    return result;
}

}
