//
// Created by Andres Jaimes on 25/06/25.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/csv_parser.h"

TEST_CASE("csv parser", "[csv]") {

    sevilla::csv_parser parser;

    SECTION("csv parser splits a simple line") {
        size_t total = parser.parse_line("one,two,three", ',');

        REQUIRE(total == 3);
        REQUIRE(parser[0] == "one");
        REQUIRE(parser[1] == "two");
        REQUIRE(parser[2] == "three");
    }

    SECTION("csv parser handles empty line") {
        size_t total = parser.parse_line("", ',');

        REQUIRE(total == 1); // often parsed as one empty field
        REQUIRE(parser[0] == "");
    }

    SECTION("csv parser handles leading/trailing commas") {
        size_t total = parser.parse_line(",a,b,", ',');

        REQUIRE(total == 4);
        REQUIRE(parser[0] == "");
        REQUIRE(parser[1] == "a");
        REQUIRE(parser[2] == "b");
        REQUIRE(parser[3] == "");
    }

    SECTION("csv parser handles consecutive separators") {
        size_t total = parser.parse_line("one,,three", ',');

        REQUIRE(total == 3);
        REQUIRE(parser[0] == "one");
        REQUIRE(parser[1] == "");
        REQUIRE(parser[2] == "three");
    }

    SECTION("csv parser handles whitespace inside fields") {
        size_t total = parser.parse_line("  a , b , c ", ',');

        REQUIRE(total == 3);
        REQUIRE(parser[0] == "  a ");
        REQUIRE(parser[1] == " b ");
        REQUIRE(parser[2] == " c ");
    }

    SECTION("csv parser handles quotes inside fields") {
        size_t total = parser.parse_line(R"("""we""","support ""quotes"" inside fields","which is ""great""")", ',');

        REQUIRE(total == 3);
        REQUIRE(parser[0] == R"("we")");
        REQUIRE(parser[1] == R"(support "quotes" inside fields)");
        REQUIRE(parser[2] == R"(which is "great")");
    }

    SECTION("csv parser with single field") {
        size_t total = parser.parse_line("onlyone", ',');

        REQUIRE(total == 1);
        REQUIRE(parser[0] == "onlyone");
    }

    SECTION("csv parser indexing out of range throws") {
        parser.parse_line("a,b,c", ',');

        REQUIRE_THROWS_AS(parser[3], std::out_of_range);
    }
}
