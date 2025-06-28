//
// Created by Andres Jaimes on 25/06/25.
//

#include "http_client.h"

namespace sevilla {

    bool http_client::initialized = false;

    /**
     * URL-encodes a string.
     * Notes:
     * - Since 7.82.0, the handle parameter in curl_easy_escape is ignored.
     * - If length is set to 0 (zero), curl_easy_escape uses strlen() on the input string
     * to find out the size.
     * - https://curl.se/libcurl/c/curl_easy_escape.html
     */
    const std::string http_client::encode(const std::string& str) {
        std::string result;
        char* output = curl_easy_escape(nullptr, str.c_str(), 0);
        if (output) {
            result = output;
            curl_free(output);
        }
        return result;
    }

    const std::string http_client::encode_map(const std::map<std::string, std::string> &map) {
        std::string sep = "";
        std::ostringstream oss;
        for (const auto& [key, value] : map) {
            oss << sep << encode(key) << "=" << encode(value);
            sep = "&";
        }
        return oss.str();
    }

    const void http_client::add_query_params() {
        if (!query_params.empty()) {
            const std::string qs = encode_map(query_params);
            // this condition implicitly handles the case when url is empty
            if (url.find('?') == std::string::npos) {
                url += "?" + qs;
            } else {
                if (url.back() == '&')
                    url += qs;
                else
                    url += "&" + qs;
            }
        }
    }

    void http_client::reset() {
        url.clear();
        method.clear();
        auth_basic_username.clear();
        auth_basic_password.clear();
        auth_bearer_token.clear();
        user_agent.clear();
        headers.clear();
        request_body.clear();
        status_code = 0;
        response_body.clear();
        error = CURLE_OK;
        error_message.clear();
    }


    void http_client::make_request() {
        error = CURLE_OK;
        error_message.clear();

        CURL* curl = curl_easy_init();
        if (curl) {
            add_query_params();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            #if defined(_WIN32)
            // Windows needs the agencies file, unless curl is compiled to use the host's one.
            curl_easy_setopt(curl, CURLOPT_CAINFO, ca_info_file);
            #endif

            // Add headers
            curl_slist* http_headers = nullptr;
            if (!headers.empty()) {
                for (const auto& [key, value] : headers) {
                    std::ostringstream oss;
                    oss << key << ":" << value;
                    http_headers = curl_slist_append(http_headers, oss.str().c_str());
                }
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
            }

            // Agent name
            if (!user_agent.empty())
                curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());

            // Add any provided auth data
            if (!auth_basic_username.empty()) {
                curl_easy_setopt(curl, CURLOPT_USERPWD, (auth_basic_username + ":" + auth_basic_password).c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            }

            if (!auth_bearer_token.empty()) {
                curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, auth_bearer_token.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
            }

            // Timeout
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, max_timeout);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connection_timeout);

            // Response writer
            std::string buffer;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

            // Method
            if (method == "POST") {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                if (!form_params.empty()) // for application/x-www-form-urlencoded data
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, encode_map(form_params).c_str());
                else if (!request_body.empty())
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
            } else {
                curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            }

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
            } else {
                error = res;
                error_message = curl_easy_strerror(res);
            }

            if (http_headers != nullptr)
                curl_slist_free_all(http_headers);
            curl_easy_cleanup(curl);
        } else {
            error = CURLE_FAILED_INIT;
            error_message = "Failed to initialize cURL.";
        }
    }

}
