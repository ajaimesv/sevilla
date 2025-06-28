//
// Created by Andres Jaimes on 28/06/25.
//

#include <locale>
#include <codecvt>
#include "c_api.h"
#include "utils.h"

extern "C" DLL_EXPORT
const char* sv_slugify(const char* input) {
    thread_local std::string result;

    if (input == nullptr) {
        result = "";
        return result.c_str();
    }

    result = sevilla::slugify(input);
    return result.c_str();
}

extern "C" DLL_EXPORT
const wchar_t* sv_slugify_w(const wchar_t* input) {
    thread_local std::wstring converted;

    if (input == nullptr) {
        converted = L"";
        return converted.c_str();
    }

    // utf-8/utf-16 converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    try {
        const std::wstring ws_input = std::wstring(input);
        // convert to std::string assuming utf-8
        const std::string u8_input = converter.to_bytes(ws_input);
        converted = converter.from_bytes(sv_slugify(u8_input.c_str()));
    } catch (...) {
        converted = converter.from_bytes(make_error("UTF conversion exception"));
    }

    return converted.c_str();

}
