//
// Created by Andres Jaimes on 28/06/25.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/utils.h"

TEST_CASE("slugify", "[utils][slugify]") {

    SECTION("correctly converts a string") {
        std::string s = "Hello, World! - Testing, 123.";
        std::string result = sevilla::slugify(s);
        REQUIRE(result == "hello-world-testing-123");
    }

    SECTION("correctly handles an empty string") {
        std::string s = "";
        std::string result = sevilla::slugify(s);
        REQUIRE(result == "");
    }

    SECTION("correctly handles a string of spaces") {
        std::string s = "   ";
        std::string result = sevilla::slugify(s);
        REQUIRE(result == "");
    }

}

TEST_CASE("trim operations", "[utils][trim]") {

    SECTION("correctly ltrims a string") {
        std::string s = "  \t \n Hello, World!  ";
        std::string result = sevilla::ltrim(s);
        REQUIRE(result == "Hello, World!  ");
    }

    SECTION("correctly rtrims a string") {
        std::string s = "  Hello, World!\n \t ";
        std::string result = sevilla::rtrim(s);
        REQUIRE(result == "  Hello, World!");
    }

    SECTION("correctly trims a string") {
        std::string s = " \t  \n Hello, World! \n \t ";
        std::string result = sevilla::trim(s);
        REQUIRE(result == "Hello, World!");
    }

    SECTION("correctly handles an empty string") {
        std::string s = "";
        std::string result = sevilla::trim(s);
        REQUIRE(result == "");
    }

    SECTION("correctly handles an string of spaces") {
        std::string s = "   ";
        std::string result = sevilla::trim(s);
        REQUIRE(result == "");
    }

}
