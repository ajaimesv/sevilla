//
// Created by Andres Jaimes on 27/06/25.
//

#include <locale>
#include <codecvt>
#include <sstream>
#include "c_api.h"
#include "email_client.h"
#include "json.hpp"

extern "C" DLL_EXPORT
const char* sec_send_email(const char* request) {
    thread_local sevilla::email_client email_client;
    thread_local std::string result;

    try {
        email_client.reset();
        nlohmann::json req = nlohmann::json::parse(request);

        if (req.contains("host") && req["host"].is_string())
            email_client.host = req["host"];
        if (req.contains("port") && req["port"].is_number_integer())
            email_client.port = req["port"];
        if (req.contains("username") && req["username"].is_string())
            email_client.username = req["username"];
        if (req.contains("password") && req["password"].is_string())
            email_client.password = req["password"];
        if (req.contains("subject") && req["subject"].is_string())
            email_client.subject = req["subject"];
        if (req.contains("body") && req["body"].is_string())
            email_client.body = req["body"];
        if (req.contains("sender") && req["sender"].is_object()) {
            email_client.sender = req.at("sender").get<std::map<std::string, std::string>>();
            if (email_client.sender.size() != 1)
                throw std::invalid_argument("Sender: invalid value. There must be exactly one sender.");
        }
        if (req.contains("recipients") && req["recipients"].is_object())
            email_client.recipients = req.at("recipients").get<std::map<std::string, std::string>>();
        if (req.contains("cc_recipients") && req["cc_recipients"].is_object())
            email_client.cc_recipients = req.at("cc_recipients").get<std::map<std::string, std::string>>();
        if (req.contains("bcc_recipients") && req["bcc_recipients"].is_number_integer())
            email_client.bcc_recipients = req["bcc_recipients"];

        if (email_client.recipients.size() == 0 &&
            email_client.cc_recipients.size() == 0 &&
            email_client.bcc_recipients.size() == 0) {
            throw std::invalid_argument("Recipients: invalid value. There must be at least one recipient, either regular, cc, or bcc.");
        }

        email_client.send_email();

        if (email_client.error == CURLE_OK) {
            nlohmann::json j;
            j["result"] = "ok";
            result = j.dump();
            return result.c_str();
        } else {
            std::ostringstream os;
            os << "Error " << email_client.error << ": " << email_client.error_message;
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
const wchar_t* sec_send_email_w(const wchar_t* request) {
    thread_local std::wstring converted;

    // utf-8/utf-16 converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    try {
        const std::wstring ws_request = std::wstring(request);
        // convert to std::string assuming utf-8
        const std::string u8_request = converter.to_bytes(ws_request);
        converted = converter.from_bytes(sec_send_email(u8_request.c_str()));
        return converted.c_str();
    } catch (...) {
        converted = converter.from_bytes(make_error("UTF conversion exception"));
        return converted.c_str();
    }
}
