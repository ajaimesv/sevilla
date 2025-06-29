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
        std::tuple<std::string, std::string> sender;
        std::map<std::string, std::string> recipients;
        std::map<std::string, std::string> cc_recipients;
        std::map<std::string, std::string> bcc_recipients;

        /**
         * Keeps track of the number of bytes sent.
         */
        size_t payload_offset = 0;

        /**
         * Stores the message payload, that is, the whole message to send, headers included.
         */
        std::string payload;

        /**
         * Support function for sending the request payload.
         * - https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
         *
         * Note: In C++, callbacks CANNOT be non-static class member functions.
         * - https://curl.se/libcurl/c/libcurl-tutorial.html
         */
        static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp) {
            // This is our way to receive a pointer to the current instance: "this",
            // so we're able to read the 'payload' member attribute.
            email_client *self = static_cast<email_client*>(userp);
            size_t buffer_size = size * nmemb;

            if (self->payload_offset >= self->payload.size())
                return 0;  // done sending

            size_t remaining = self->payload.size() - self->payload_offset;
            size_t copy_size = std::min(buffer_size, remaining);

            memcpy(ptr, self->payload.c_str() + self->payload_offset, copy_size);
            self->payload_offset += copy_size;

            return copy_size;
        }

    public:
        std::string host;
        int port;
        std::string username;
        std::string password;

        std::string subject;
        std::string message;

        int error;
        std::string error_message;

        /*
         * Total process timeout: connection + server response.
         */
        long max_timeout = 30000; // milliseconds
        /*
         * Connection only timeout.
         */
        long connection_timeout = 10000; // milliseconds

        void set_sender(const std::string& email, const std::string& name);
        void set_recipients(const std::map<std::string, std::string>& recipients);
        void set_cc_recipients(const std::map<std::string, std::string>& recipients);
        void set_bcc_recipients(const std::map<std::string, std::string>& recipients);
        std::string& get_payload() { return payload; }

        /**
         * Sends an email.
         * - https://curl.se/libcurl/c/smtp-tls.html
         */
        void send_email();
        void reset();

        size_t total_recipients() const;
        static bool is_valid_email(const std::string& email);
        static std::string build_url(const std::string& host, const int port);
        static std::string normalize_email(const std::string& email);
        static std::string build_display_email(const std::string& email, const std::string& name);
        static std::string generate_date();
        static std::string extract_domain(const std::string& email);
        static std::string generate_message_id(const std::string& domain);
        static std::string build_recipient_list(const std::map<std::string, std::string>& recipients);

        /**
         * This function processes 'input', by making sure that all the lines end
         * with a full CR/LF set of chars. Also, it makes the whole string end with CR/LF.
         * These changes are required by protocol.
         *
         * The function is used internally to make 'body' comform to the expected protocol.
         */
        static std::string convert_to_crlf(const std::string& input);

        /**
         * Builds the message payload. This is the actual text sent to the SMTP server.
         */
        void build_payload();
    };

}

#endif //EMAIL_CLIENT_H
