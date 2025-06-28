//
// Created by Andres Jaimes on 25/06/25.
//

#include "json.hpp"

#if defined(_WIN32)
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            // Library initialization
            sevilla::http_client::init();
            break;
        case DLL_PROCESS_DETACH:
            // Clean up any used resources
            sevilla::http_client::deinit();
            break;
    }
    return TRUE;
}

#elif defined(__APPLE__) || defined(__linux__)
#include "http_client.h"

__attribute__((constructor))
void lib_init() {
    // Library initialization
    sevilla::http_client::init();
}

__attribute__((destructor))
void lib_deinit() {
    // Clean up any used resources
    sevilla::http_client::deinit();
}

#endif


std::string make_error(const std::string& msg) {
    nlohmann::json j;
    j["error"] = msg;
    // additional metadata could be added here, like timestamps, logs, etc.
    return j.dump();
}
