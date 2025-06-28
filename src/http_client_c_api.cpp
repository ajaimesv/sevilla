//
// Created by Andres Jaimes on 25/06/25.
//

#include <locale>
#include <codecvt>
#include "c_api.h"
#include "http_client.h"
#include "json.hpp"

extern "C" DLL_EXPORT
const char* shc_request(const char* request) {
    thread_local sevilla::http_client http_client;
    thread_local std::string result;

    try {
        http_client.reset();
        nlohmann::json req = nlohmann::json::parse(request);

        if (req.contains("url") && req["url"].is_string())
            http_client.url = req["url"];
        if (req.contains("method") && req["method"].is_string())
            http_client.method = req["method"];
        if (req.contains("auth_basic_username") && req["auth_basic_username"].is_string())
            http_client.auth_basic_username = req["auth_basic_username"];
        if (req.contains("auth_basic_password") && req["auth_basic_password"].is_string())
            http_client.auth_basic_password = req["auth_basic_password"];
        if (req.contains("auth_bearer_token") && req["auth_bearer_token"].is_string())
            http_client.auth_bearer_token = req["auth_bearer_token"];
        if (req.contains("user_agent") && req["user_agent"].is_string())
            http_client.user_agent = req["user_agent"];
        if (req.contains("request_body") && req["request_body"].is_string())
            http_client.request_body = req["request_body"];
        if (req.contains("headers") && req["headers"].is_object())
            http_client.headers = req.at("headers").get<std::map<std::string, std::string>>();
        if (req.contains("query_params") && req["query_params"].is_object())
            http_client.query_params = req.at("query_params").get<std::map<std::string, std::string>>();
        if (req.contains("form_params") && req["form_params"].is_object())
            http_client.form_params = req.at("form_params").get<std::map<std::string, std::string>>();
        if (req.contains("max_timeout") && req["max_timeout"].is_number_integer())
            http_client.max_timeout = req["max_timeout"];
        if (req.contains("connection_timeout") && req["connection_timeout"].is_number_integer())
            http_client.connection_timeout = req["connection_timeout"];

        http_client.make_request();

        if (http_client.error == CURLE_OK) {
            nlohmann::json j;
            j["status_code"] = http_client.status_code;
            j["body"] = http_client.response_body;
            result = j.dump();
            return result.c_str();
        } else {
            std::ostringstream os;
            os << "Error " << http_client.error << ": " << http_client.error_message;
            result = make_error(os.str());
            return result.c_str();
        }
    } catch (std::exception& e) {
        result = make_error(e.what());
        return result.c_str();
    } catch (...) {
        result = make_error("Unknown exception");
        return result.c_str();
    }
}

extern "C" DLL_EXPORT
const wchar_t* shc_request_w(const wchar_t* request) {
    thread_local std::wstring converted;

    // utf-8/utf-16 converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    try {
        const std::wstring ws_request = std::wstring(request);
        // convert to std::string assuming utf-8
        const std::string u8_request = converter.to_bytes(ws_request);
        converted = converter.from_bytes(shc_request(u8_request.c_str()));
        return converted.c_str();
    } catch (...) {
        converted = converter.from_bytes(make_error("UTF conversion exception"));
        return converted.c_str();
    }
}
