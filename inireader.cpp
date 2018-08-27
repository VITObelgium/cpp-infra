#include "infra/inireader.h"
#include "infra/exception.h"
#include "infra/string.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace inf {

/* Typedef for prototype of handler function. */
typedef int (*ini_handler)(void* user, const char* section,
    const char* name, const char* value);

/* Typedef for prototype of fgets-style reader function. */
typedef char* (*ini_reader)(char* str, int num, void* stream);

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Nonzero to allow inline comments (with valid inline comment characters
   specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
   Python 3.2+ configparser behaviour. */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

/* Nonzero to use stack, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Maximum line length for any line in INI file. */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Strip whitespace chars off end of given string, in place. Return s. */
inline static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p))) {
        *p = '\0';
    }
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
inline static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s))) {
        s++;
    }
    return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
inline static char* find_chars_or_comment(const char* s, const char* chars)
{
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
           !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
        was_space = isspace((unsigned char)(*s));
        s++;
    }
#else
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
inline static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

/* Same as ini_parse(), but takes an ini_reader function pointer instead of
   filename. Used for implementing custom or string-based I/O. */
inline int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
    void* user)
{
/* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
#else
    char* line;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME]  = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error  = 0;

#if !INI_USE_STACK
    line = (char*)malloc(INI_MAX_LINE);
    if (!line) {
        return -2;
    }
#endif

    /* Scan through stream line by line */
    while (reader(line, INI_MAX_LINE, stream) != nullptr) {
        lineno++;

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
            (unsigned char)start[1] == 0xBB &&
            (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python configparser, allow both ; and # comments at the
               start of a line */
        } else if (*start == '[') {
            /* A "[section]" line */
            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            } else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        } else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end  = '\0';
                name  = rstrip(start);
                value = lskip(end + 1);
#if INI_ALLOW_INLINE_COMMENTS
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
#endif
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            } else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    free(line);
#endif

    return error;
}

/* Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
   close the file when it's finished -- the caller must do that. */
static int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* Parse given INI-style file. May have [section]s, name=value pairs
   (whitespace stripped), and comments starting with ';' (semicolon). Section
   is "" if name=value pair parsed before any section heading. name:value
   pairs are also supported as a concession to Python's configparser.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return nonzero on success, zero on error.

   Returns 0 on success, line number of first error on parse error (doesn't
   stop on first error), -1 on file open error, or -2 on memory allocation
   error (only when INI_USE_STACK is zero).
*/
static int ini_parse(const char* filename, ini_handler handler, void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

IniReader::IniReader(const std::string& filename)
{
    auto error = ini_parse(filename.c_str(), valueHandler, this);
    if (error != 0) {
        throw RuntimeError("Failed to parse ini file, error on line {} ({})", error, filename);
    }
}

std::vector<std::string> IniReader::sections() const
{
    std::vector<std::string> result;
    for (const auto& keyValue : _values) {
        result.push_back(keyValue.first);
    }

    return result;
}

std::optional<std::string_view> IniReader::getString(std::string_view section, std::string_view name) const noexcept
{
    std::optional<std::string_view> result;

    auto sectionIter = _values.find(str::lowercase(section));
    if (sectionIter != _values.end()) {
        auto valueIter = sectionIter->second.find(str::lowercase(name));
        if (valueIter != sectionIter->second.end()) {
            result = valueIter->second;
        }
    }

    return result;
}

std::optional<long> IniReader::getInteger(std::string_view section, std::string_view name) const noexcept
{
    std::optional<long> result;

    auto valueStr = getString(section, name);
    if (valueStr) {
        char* end;
        // This parses "1234" (decimal) and also "0x4D2" (hex)
        long n = std::strtol(valueStr->data(), &end, 0);
        if (end > valueStr->data()) {
            result = n;
        }
    }

    return result;
}

std::optional<unsigned long> IniReader::getUnsignedInteger(std::string_view section, std::string_view name) const noexcept
{
    std::optional<long> result;

    auto valueStr = getString(section, name);
    if (valueStr) {
        char* end;
        // This parses "1234" (decimal) and also "0x4D2" (hex)
        long n = std::strtoul(valueStr->data(), &end, 0);
        if (end > valueStr->data()) {
            result = n;
        }
    }

    return result;
}

std::optional<double> IniReader::getReal(std::string_view section, std::string_view name) const noexcept
{
    std::optional<double> result;

    auto valueStr = getString(section, name);
    if (valueStr) {
        char* end;
        double n = strtod(valueStr->data(), &end);
        if (end > valueStr->data()) {
            result = n;
        }
    }

    return result;
}

std::optional<bool> IniReader::getBool(std::string_view section, std::string_view name) const noexcept
{
    std::optional<bool> result;

    auto valueStr = getString(section, name);
    if (valueStr) {
        // Convert to lower case to make string comparisons case-insensitive
        std::string lower = str::lowercase(*valueStr);
        if (lower == "true" || lower == "yes" || lower == "on" || lower == "1") {
            result = true;
        } else if (lower == "false" || lower == "no" || lower == "off" || lower == "0") {
            result = false;
        }
    }

    return result;
}

int IniReader::valueHandler(void* user, const char* section, const char* name, const char* value)
{
    auto* reader = static_cast<IniReader*>(user);
    reader->_values[str::lowercase(section)].emplace(str::lowercase(name), value);
    return 1;
}
}
