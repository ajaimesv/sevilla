//
// Created by Andres Jaimes on 27/06/25.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/email_client.h"

TEST_CASE("total_recipients function", "[email_client][total_recipients]") {
    SECTION("must add up the recipients") {
        sevilla::email_client email_client;
        email_client.set_recipients({ { "email@email.com", "name" } });
        email_client.set_cc_recipients({ { "email@email.com", "name" } });
        email_client.set_bcc_recipients({ { "email@email.com", "name" } });
        const size_t result = email_client.total_recipients();
        REQUIRE(result == 3);
    }
}

TEST_CASE("is_valid_email function", "[email_client][is_valid_email]") {
    SECTION("should approve a valid email") {
        REQUIRE(sevilla::email_client::is_valid_email("email@email.com") == true);
        REQUIRE(sevilla::email_client::is_valid_email("email@email-email.com") == true);
        REQUIRE(sevilla::email_client::is_valid_email("email.email@email.com") == true);
        REQUIRE(sevilla::email_client::is_valid_email("email_email.email@email.com") == true);
        REQUIRE(sevilla::email_client::is_valid_email("email@email.com.subdomain") == true);
        REQUIRE(sevilla::email_client::is_valid_email("email_123@email.com") == true);
    }
    SECTION("should disapprove an invalid email") {
        REQUIRE_FALSE(sevilla::email_client::is_valid_email("email.com"));
        REQUIRE_FALSE(sevilla::email_client::is_valid_email(""));
        REQUIRE_FALSE(sevilla::email_client::is_valid_email("   "));
        REQUIRE_FALSE(sevilla::email_client::is_valid_email("aéióu@email.com"));
        REQUIRE_FALSE(sevilla::email_client::is_valid_email("email email@email.com"));
    }
}

TEST_CASE("generate_date function", "[email_client][generate_date]") {
    SECTION("must create a valid date string") {
        // like in "Sun, 29 Jun 2025 03:26:13 GMT"
        REQUIRE(sevilla::email_client::generate_date().length() > 0);
    }
}

TEST_CASE("extract_domain function", "[email_client][extract_domain]") {
    SECTION("should extract the domain from the sender email address") {
        REQUIRE(sevilla::email_client::extract_domain("email@email.com") == "email.com");
        REQUIRE(sevilla::email_client::extract_domain("email@email.com.subdomain") == "email.com.subdomain");
        REQUIRE(sevilla::email_client::extract_domain("email@email-2.com") == "email-2.com");
        REQUIRE(sevilla::email_client::extract_domain("email") == "invalid-domain.com");
        REQUIRE(sevilla::email_client::extract_domain("") == "invalid-domain.com");
    }
}

TEST_CASE("generate_message_id function", "[email_client][generate_message_id]") {
    SECTION("should generate a generate_message_id") {
        // like: "<1751167945.492879@email.com>"
        REQUIRE(sevilla::email_client::generate_message_id("email.com").length() > 0);
    }
}

TEST_CASE("build_recipient_list function", "[email_client][build_recipient_list]") {
    SECTION("should generate a build_recipient_list") {
        std::string result = sevilla::email_client::build_recipient_list({
            {"email@email.com", "Joe Doe" }
            });
        REQUIRE(result == "\"Joe Doe\" <email@email.com>");
        result = sevilla::email_client::build_recipient_list({
            {"email@email.com", "Joe Doe" },
            {"email2@email.com", "Joe Doe 2" },
            });
        if (result != "\"Joe Doe 2\" <email2@email.com>, \"Joe Doe\" <email@email.com>" &&
            result != "\"Joe Doe\" <email@email.com>, \"Joe Doe 2\" <email2@email.com>") {
            FAIL("Unexpected result: " << result);
        }
        result = sevilla::email_client::build_recipient_list({
            {"email@email.com", "Joe Doe" },
            {"email2@email.com", "" },
            });
        if (result != "<email2@email.com>, \"Joe Doe\" <email@email.com>" &&
            result != "\"Joe Doe\" <email@email.com>, \"\" <email2@email.com>") {
            FAIL("Unexpected result: " << result);
        }
    }
}

TEST_CASE("build_url function", "[email_client][build_url]") {

    SECTION("build_url must build a url when port is 0") {
        const std::string result = sevilla::email_client::build_url("host", 0);
        REQUIRE(result == "smtp://host:587");
    }

    SECTION("build_url must build a url when port is NOT 0") {
        const std::string result = sevilla::email_client::build_url("host", 8080);
        REQUIRE(result == "smtp://host:8080");
    }

    SECTION("build_url must build a url when port is negative") {
        const std::string result = sevilla::email_client::build_url("host", -10);
        REQUIRE(result == "smtp://host:587");
    }

}

TEST_CASE("normalize_email function", "[email_client][normalize_email]") {

    SECTION("normalize_email must add brackets to an email") {
        const std::string result = sevilla::email_client::normalize_email("abc@abc.com");
        REQUIRE(result == "<abc@abc.com>");
    }

    SECTION("normalize_email must NOT add brackets to an email that already has them") {
        const std::string result = sevilla::email_client::normalize_email("<abc@abc.com>");
        REQUIRE(result == "<abc@abc.com>");
    }

    SECTION("normalize_email must return an empty string if email is empty") {
        const std::string result = sevilla::email_client::normalize_email("");
        REQUIRE(result.empty());
    }

}

TEST_CASE("build_display_email function", "[email_client][build_display_email]") {

    SECTION("build_display_email must format an email without brackets") {
        const std::string result = sevilla::email_client::build_display_email("abc@abc.com", "Launchpad McQuack");
        REQUIRE(result == "\"Launchpad McQuack\" <abc@abc.com>");
    }

    SECTION("build_display_email must format an email without brackets, and without name") {
        const std::string result = sevilla::email_client::build_display_email("abc@abc.com", "");
        REQUIRE(result == "<abc@abc.com>");
    }

    SECTION("build_display_email must format an email with brackets") {
        const std::string result = sevilla::email_client::build_display_email("<abc@abc.com>", "Launchpad McQuack");
        REQUIRE(result == "\"Launchpad McQuack\" <abc@abc.com>");
    }

    SECTION("build_display_email must format an email with brackets, but without name") {
        const std::string result = sevilla::email_client::build_display_email("<abc@abc.com>", "");
        REQUIRE(result == "<abc@abc.com>");
    }

    SECTION("build_display_email must return an empty string if email is empty") {
        const std::string result = sevilla::email_client::build_display_email("", "Launchpad McQuack");
        REQUIRE(result.empty());
    }

}

TEST_CASE("convert_to_crlf function", "[email_client][convert_to_crlf]") {

    SECTION("convert_to_crlf must replace LF for CRLF chars") {
        const std::string result = sevilla::email_client::convert_to_crlf("Hello\nWonderful\nWorld!\n");
        REQUIRE(result == "Hello\r\nWonderful\r\nWorld!\r\n");
    }

    SECTION("convert_to_crlf must replace leave CRLF chars unchanged") {
        const std::string result = sevilla::email_client::convert_to_crlf("Hello\r\nWorld!");
        REQUIRE(result == "Hello\r\nWorld!");
    }

    SECTION("convert_to_crlf must replace leave last CRLF chars unchanged") {
        const std::string result = sevilla::email_client::convert_to_crlf("Hello\r\nWorld!\r\n");
        REQUIRE(result == "Hello\r\nWorld!\r\n");
    }

    SECTION("convert_to_crlf must handle empty strings") {
        const std::string result = sevilla::email_client::convert_to_crlf("");
        REQUIRE(result == "");
    }

    SECTION("convert_to_crlf leave untouched a string that only contains CRLF set of chars") {
        const std::string result = sevilla::email_client::convert_to_crlf("\r\n\r\n");
        REQUIRE(result == "\r\n\r\n");
    }

    SECTION("convert_to_crlf replace chars in a string that only contains LF chars") {
        const std::string result = sevilla::email_client::convert_to_crlf("\n\n\n");
        REQUIRE(result == "\r\n\r\n\r\n");
    }

}

TEST_CASE("build_payload function", "[email_client][build_payload]") {
    SECTION("must correctly create a payload") {
        sevilla::email_client email_client;
        email_client.set_sender("sender@email.com", "sender name");
        email_client.set_recipients({ { "email@email.com", "name" }, { "email2@email.com", "name 2" } });
        email_client.set_cc_recipients({ { "cc_email@email.com", "cc name" }, { "cc_email2@email.com", "cc name 2" } });
        email_client.set_bcc_recipients({ { "bcc_email@email.com", "bcc name" } });
        email_client.subject = "Hello World";
        email_client.message = "<h1>Hello World</h1>";
        email_client.build_payload();
        REQUIRE(email_client.get_payload().find("Date:") != std::string::npos);
        REQUIRE(email_client.get_payload().find("To: \"name 2\" <email2@email.com>, \"name\" <email@email.com>") != std::string::npos);
        REQUIRE(email_client.get_payload().find("From: \"sender name\" <sender@email.com>") != std::string::npos);
        REQUIRE(email_client.get_payload().find("Cc: \"cc name 2\" <cc_email2@email.com>, \"cc name\" <cc_email@email.com>") != std::string::npos);
        REQUIRE(email_client.get_payload().find("Message-ID") != std::string::npos);
        REQUIRE(email_client.get_payload().find("Subject: Hello World") != std::string::npos);
        REQUIRE(email_client.get_payload().find("Content-Type: text/html; charset=UTF-8") != std::string::npos);
        REQUIRE(email_client.get_payload().find("<h1>Hello World</h1>") != std::string::npos);
    }
}
