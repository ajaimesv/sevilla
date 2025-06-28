//
// Created by Andres Jaimes on 28/06/25.
//

#include <locale>
#include <codecvt>
#include "c_api.h"
#include "utils.h"

extern "C" DLL_EXPORT
const char* su_slugify(const char* input) {
    thread_local std::string result;
    result = sevilla::slugify(input);
    return result.c_str();
}

extern "C" DLL_EXPORT
const wchar_t* su_slugify_w(const wchar_t* input) {
    thread_local std::wstring converted;

    // utf-8/utf-16 converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    try {
        const std::wstring ws_input = std::wstring(input);
        // convert to std::string assuming utf-8
        const std::string u8_input = converter.to_bytes(ws_input);
        converted = converter.from_bytes(su_slugify(u8_input.c_str()));
        return converted.c_str();
    } catch (...) {
        converted = converter.from_bytes(make_error("UTF conversion exception"));
        return converted.c_str();
    }
}
