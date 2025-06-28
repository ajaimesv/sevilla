//
// Created by Andres Jaimes on 25/06/25.
//

#include <locale>
#include <codecvt>
#include "c_api.h"
#include "csv_parser.h"

thread_local sevilla::csv_parser csv_parser;

extern "C" DLL_EXPORT
size_t sv_parse_line(const char* line, const char separator) {
    return csv_parser.parse_line(line, separator);
}

extern "C" DLL_EXPORT
size_t sv_parse_line_w(const wchar_t* line, const wchar_t separator) {
    // utf-8/utf-16 converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    try {
        const std::wstring ws_line = std::wstring(line);
        // convert to std::string assuming utf-8
        const std::string u8_line = converter.to_bytes(ws_line);
        // do a simple conversion for the separator, since we expect ASCII
        const char sep = separator <= 127 ? static_cast<char>(separator) : ',';
        return sv_parse_line(u8_line.c_str(), sep);
    } catch (...) {
        return -1;
    }
}

extern "C" DLL_EXPORT
size_t sv_field_count() {
    return csv_parser.size();
}

extern "C" DLL_EXPORT
const char* sv_field(size_t index) {
    try {
        return csv_parser[index].c_str();
    } catch (...) {
        return nullptr;
    }
}

extern "C" DLL_EXPORT
const wchar_t* sv_field_w(size_t index) {
    thread_local std::wstring converted;

    // utf-8/utf-16 converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    try {
        converted = converter.from_bytes(csv_parser[index]);
        return converted.c_str();
    } catch (...) {
        return nullptr;
    }
}
