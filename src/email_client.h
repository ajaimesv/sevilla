//
// Created by Andres Jaimes on 27/06/25.
//

#ifndef EMAIL_CLIENT_H
#define EMAIL_CLIENT_H

#include <map>
#include <string>
#include <curl/curl.h>

namespace sevilla {

    class email_client {

    private:
        /**
         * Keeps track of the number of bytes sent.
         */
        size_t body_offset = 0;

        /**
         * Stores a copy of the body attribute, processed with CR/LF characters
         * as required by the protocol.
         */
        std::string body_converted_to_crlf;

        /**
         * Support function for sending the request body.
         * - https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
         *
         * Note: In C++, callbacks CANNOT be non-static class member functions.
         * - https://curl.se/libcurl/c/libcurl-tutorial.html
         */
        static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp) {
            // This is our way to receive a pointer to the current instance: "this",
            // so we're able to read the 'body_converted_to_crlf' member attribute.
            email_client *self = static_cast<email_client*>(userp);
            size_t buffer_size = size * nmemb;

            if (self->body_offset >= self->body_converted_to_crlf.size())
                return 0;  // done sending

            size_t remaining = self->body_converted_to_crlf.size() - self->body_offset;
            size_t copy_size = std::min(buffer_size, remaining);

            memcpy(ptr, self->body_converted_to_crlf.c_str() + self->body_offset, copy_size);
            self->body_offset += copy_size;

            return copy_size;
        }

    public:
        std::string host;
        int port;
        std::string username;
        std::string password;

        std::map<std::string, std::string> sender;
        std::map<std::string, std::string> recipients;
        std::map<std::string, std::string> cc_recipients;
        std::map<std::string, std::string> bcc_recipients;
        std::string subject;
        std::string body;


        int error;
        std::string error_message;

        /**
         * Sends an email.
         * - https://curl.se/libcurl/c/smtp-tls.html
         */
        void send_email();
        void reset();

        static std::string build_url(const std::string& host, const int port);
        static std::string normalize_email(const std::string& email);
        static std::string build_display_email(const std::string& email, const std::string& name);

        /**
         * This function processes 'input', by making sure that all the lines end
         * with a full CR/LF set of chars. Also, it makes the whole string end with CR/LF.
         * These changes are required by protocol.
         *
         * The function is used internally to make 'body' comform to the expected protocol.
         */
        static std::string convert_to_crlf(const std::string& input);
    };

}

#endif //EMAIL_CLIENT_H
