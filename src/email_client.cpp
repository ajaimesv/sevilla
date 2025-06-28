//
// Created by Andres Jaimes on 27/06/25.
//

#include <sstream>
#include "email_client.h"

namespace sevilla {

    std::string email_client::build_url(const std::string& host, const int port) {
        // Port 587 is commonly used for secure mail submission (see RFC 4403)
        const int p = port > 0 ? port : 587;
        std::ostringstream url;
        url << "smtp://" << host << ":" << p;
        return url.str();
    }

    std::string email_client::normalize_email(const std::string& email) {
        if (email.empty()) return email;

        if (email[0] == '<' && email[email.length() - 1] == '>') {
            return email;
        } else {
            return '<' + email + '>';
        }
    }

    std::string email_client::build_display_email(const std::string& email, const std::string& name) {
        if (email.empty()) return email;

        if (name.empty()) {
            return normalize_email(email);
        } else {
            return name + " " + normalize_email(email);
        }
    }

    void email_client::reset() {
        host.clear();
        port = 0;
        username.clear();
        password.clear();
        sender.clear();
        recipients.clear();
        cc_recipients.clear();
        bcc_recipients.clear();
        subject.clear();
        body.clear();
        body_converted_to_crlf.clear();
        error = CURLE_OK;
        error_message.clear();
    }

    std::string email_client::convert_to_crlf(const std::string& input) {
        std::string output;
        output.reserve(input.size() + input.size() / 10);  // pre-allocate extra space

        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '\r') {
                // Check for CRLF already
                if (i + 1 < input.size() && input[i + 1] == '\n') {
                    output += "\r\n";
                    ++i;
                } else {
                    output += "\r\n";
                }
            } else if (input[i] == '\n') {
                output += "\r\n";
            } else {
                output += input[i];
            }
        }

        // Ensure the string ends with CRLF
        if (output.size() < 2 || output.substr(output.size() - 2) != "\r\n") {
            output += "\r\n";
        }

        return output;
    }

    void email_client::send_email() {
        error = CURLE_OK;
        error_message.clear();

        // convert the body's line endings, and make sure it ends with '\r\n'
        body_converted_to_crlf = convert_to_crlf(body);

        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());

            // Server
            curl_easy_setopt(curl, CURLOPT_URL, build_url(host, port).c_str());

            // SSL, TLS
            curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

            #if defined(_WIN32)
            // Windows needs the agencies file, unless curl is compiled to use the host's one.
            curl_easy_setopt(curl, CURLOPT_CAINFO, ca_info_file);
            #endif

            // Sender
            for (const auto& [email, name]: sender) {
                curl_easy_setopt(curl, CURLOPT_MAIL_FROM, normalize_email(email).c_str());
            }

            // Recipients, add only email addresses
            // - https://curl.se/libcurl/c/CURLOPT_MAIL_RCPT.html
            struct curl_slist *recips = NULL;
            for (const auto& [email, name]: recipients) {
                recips = curl_slist_append(recips, normalize_email(email).c_str());
            }
            for (const auto& [email, name]: cc_recipients) {
                recips = curl_slist_append(recips, normalize_email(email).c_str());
            }
            for (const auto& [email, name]: bcc_recipients) {
                recips = curl_slist_append(recips, normalize_email(email).c_str());
            }
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recips);

            // Headers, allow name + email address
            struct curl_slist *headers = NULL;
            for (const auto& [email, name]: recipients) {
                headers = curl_slist_append(headers, ("To: " + build_display_email(email, name)).c_str());
            }
            for (const auto& [email, name]: sender) {
                headers = curl_slist_append(headers, ("From: " + build_display_email(email, name)).c_str());
            }
            for (const auto& [email, name]: cc_recipients) {
                headers = curl_slist_append(headers, ("Cc: " + build_display_email(email, name)).c_str());
            }
            for (const auto& [email, name]: bcc_recipients) {
                headers = curl_slist_append(headers, ("Bcc: " + build_display_email(email, name)).c_str());
            }
            headers = curl_slist_append(headers, ("Subject: " + subject).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Prepare the callback function that sends the body
            body_offset = 0;  // reset before sending
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
            curl_easy_setopt(curl, CURLOPT_READDATA, this); // pass pointer to current instance
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            // Send the message
            CURLcode res = curl_easy_perform(curl);

            // Check for errors
            if(res != CURLE_OK) {
                error = res;
                error_message = curl_easy_strerror(res);
            }

            // Free the list of recipients and headers
            curl_slist_free_all(recips);
            curl_slist_free_all(headers);

            // And, clean up
            curl_easy_cleanup(curl);
        }
    }


}
