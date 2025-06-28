//
// Created by Andres Jaimes on 25/06/25.
//

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <map>
#include <sstream>
#include <string>
#include <curl/curl.h>

namespace sevilla {

    class http_client {
    private:
        static bool initialized;

        /**
         * Support function for writing the response body to a string.
         * - https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
         * Note: In C++, callbacks CANNOT be non-static class member functions.
         * - https://curl.se/libcurl/c/libcurl-tutorial.html
         */
        static size_t write_callback(const char* contents, size_t size, size_t nmemb, std::string* buffer) {
            const size_t total_size = size * nmemb;
            buffer->append(contents, total_size);
            return total_size;
        }

    public:
        std::string url;
        std::string method;
        std::string auth_basic_username;
        std::string auth_basic_password;
        std::string auth_bearer_token;
        std::string user_agent;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> query_params;
        // specific for form parameters (application/x-www-form-urlencoded)
        std::map<std::string, std::string> form_params;
        std::string request_body;
        int status_code = 0;
        std::string response_body;
        int error;
        std::string error_message;

        std::string ca_info_file = "ca-bundle.crt";
        /*
         * Total process timeout: connection + server response.
         */
        long max_timeout = 30000; // milliseconds
        /*
         * Connection only timeout.
         */
        long connection_timeout = 5000; // milliseconds

        /**
         * This function sets up the program environment that libcurl needs.
         * - https://curl.se/libcurl/c/libcurl-tutorial.html
         * - https://curl.se/libcurl/c/curl_global_init.html
         */
        static void init() {
            if (!initialized) {
                curl_global_init(CURL_GLOBAL_DEFAULT);
                initialized = true;
            }
        }

        /**
         * This function releases resources acquired by the initializer.
         * - https://curl.se/libcurl/c/curl_global_cleanup.html
         */
        static void deinit() {
            if (initialized) {
                curl_global_cleanup();
                initialized = false;
            }
        }

        /**
         * Make a remote request.
         */
        void make_request();

        void reset();

        static const std::string encode(const std::string& str);

        static const std::string encode_map(const std::map<std::string, std::string>& map);

        const void add_query_params();

    };


}

#endif //HTTP_CLIENT_H
