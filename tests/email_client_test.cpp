//
// Created by Andres Jaimes on 27/06/25.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/email_client.h"

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
        REQUIRE(result == "Launchpad McQuack <abc@abc.com>");
    }

    SECTION("build_display_email must format an email without brackets, and without name") {
        const std::string result = sevilla::email_client::build_display_email("abc@abc.com", "");
        REQUIRE(result == "<abc@abc.com>");
    }

    SECTION("build_display_email must format an email with brackets") {
        const std::string result = sevilla::email_client::build_display_email("<abc@abc.com>", "Launchpad McQuack");
        REQUIRE(result == "Launchpad McQuack <abc@abc.com>");
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
        const std::string result = sevilla::email_client::convert_to_crlf("Hello\nWonderful\nWorld!");
        REQUIRE(result == "Hello\r\nWonderful\r\nWorld!\r\n");
    }

    SECTION("convert_to_crlf must replace leave CRLF chars unchanged") {
        const std::string result = sevilla::email_client::convert_to_crlf("Hello\r\nWorld!");
        REQUIRE(result == "Hello\r\nWorld!\r\n");
    }

    SECTION("convert_to_crlf must replace leave last CRLF chars unchanged") {
        const std::string result = sevilla::email_client::convert_to_crlf("Hello\r\nWorld!\r\n");
        REQUIRE(result == "Hello\r\nWorld!\r\n");
    }

    SECTION("convert_to_crlf must handle empty strings") {
        const std::string result = sevilla::email_client::convert_to_crlf("");
        REQUIRE(result == "\r\n");
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
