//
// Created by Andres Jaimes on 27/06/25.
//

#include <sstream>
#include "email_client.h"

#include <iomanip>
#include <random>
#include <regex>

namespace sevilla {

    void email_client::set_sender(const std::string& email, const std::string& name) {
        if (!is_valid_email(email))
            throw std::invalid_argument("Invalid sender email address");
        sender = std::make_tuple(email, name);
    }

    size_t email_client::total_recipients() const {
        return recipients.size() + cc_recipients.size() + bcc_recipients.size();
    }

    void email_client::set_recipients(const std::map<std::string, std::string>& recipients) {
        for (const auto& recipient : recipients) {
            if (!is_valid_email(recipient.first))
                throw std::invalid_argument("Invalid recipient email address");
        }
        this->recipients = recipients;
    }

    void email_client::set_cc_recipients(const std::map<std::string, std::string>& recipients) {
        for (const auto& recipient : recipients) {
            if (!is_valid_email(recipient.first))
                throw std::invalid_argument("Invalid cc recipient email address");
        }
        cc_recipients = recipients;
    }

    void email_client::set_bcc_recipients(const std::map<std::string, std::string>& recipients) {
        for (const auto& recipient : recipients) {
            if (!is_valid_email(recipient.first))
                throw std::invalid_argument("Invalid bcc recipient email address");
        }
        bcc_recipients = recipients;
    }

    bool email_client::is_valid_email(const std::string& email) {
        // Simple but practical email regex pattern
        static const std::regex pattern(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");

        return std::regex_match(email, pattern);
    }

    std::string email_client::build_url(const std::string& host, const int port, const bool use_tls) {
        // Port 587 is commonly used for secure mail submission (see RFC 4403)
        const int p = port > 0 ? port : (use_tls ? 587: 25);
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
            return "\"" + name + "\" " + normalize_email(email);
        }
    }

    std::string email_client::build_recipient_list(const std::map<std::string, std::string>& recipients) {
        std::ostringstream recips;
        std::string separator;
        for (const auto& [email, name] : recipients) {
            recips << separator << build_display_email(email, name);
            separator = ", ";
        }
        return recips.str();
    }

    void email_client::reset() {
        host.clear();
        port = 0;
        username.clear();
        password.clear();
        std::get<0>(sender).clear();
        std::get<1>(sender).clear();
        recipients.clear();
        cc_recipients.clear();
        bcc_recipients.clear();
        subject.clear();
        message.clear();
        payload.clear();
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

        return output;
    }

    std::string email_client::generate_date() {
        const std::time_t now = std::time(nullptr);
        const std::tm tm = *std::gmtime(&now);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
        return oss.str();
    }

    std::string email_client::extract_domain(const std::string& email) {
        const size_t at_pos = email.find('@');
        if (at_pos == std::string::npos || at_pos + 1 >= email.length()) {
            // this should never be called, because all our email addresses have been validated
            return "invalid-domain.com";
        }
        return email.substr(at_pos + 1);
    }

    std::string email_client::generate_message_id(const std::string& domain) {
        std::ostringstream oss;

        // Use time + random number to ensure uniqueness
        const std::time_t now = std::time(nullptr);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(100000, 999999);

        oss << "<" << now << "." << dis(gen) << "@" << domain << ">";
        return oss.str();
    }

    void email_client::build_payload() {
        std::ostringstream s;
        const std::string domain = extract_domain(std::get<0>(sender));

        // no bcc on this list to prevent the user's email client from displaying them
        s << "Date: " << generate_date() << "\r\n"
          << "To: " << build_recipient_list(recipients) << "\r\n"
          << "From: " << build_display_email(std::get<0>(sender), std::get<1>(sender)) << "\r\n"
          << "Cc: " << build_recipient_list(cc_recipients) << "\r\n"
          << "Message-ID: " << generate_message_id(domain) << "\r\n"
          << "Subject: " << subject << "\r\n"
          << "Content-Type: text/html; charset=UTF-8\r\n"
          << "\r\n" /* empty line to divide headers from body, see RFC 5322 */
          << convert_to_crlf(message) << "\r\n";

        payload = s.str();
    }


    void email_client::send_email() {
        error = CURLE_OK;
        error_message.clear();

        build_payload();

        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());

            // Server
            curl_easy_setopt(curl, CURLOPT_URL, build_url(host, port, use_tls).c_str());

            // STARTTLS
            if (use_tls)
                curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

            #if defined(_WIN32)
            // Windows needs the agencies file, unless curl is compiled to use the host's one.
            curl_easy_setopt(curl, CURLOPT_CAINFO, ca_info_file);
            #endif

            // Sender
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, normalize_email(std::get<0>(sender)).c_str());

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

            // Timeout
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, max_timeout);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connection_timeout);

            // Prepare the callback function that sends the body
            payload_offset = 0;  // reset before sending
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

            // Free the list of recipients
            curl_slist_free_all(recips);

            // And, clean up, QUIT
            curl_easy_cleanup(curl);
        }
    }


}
