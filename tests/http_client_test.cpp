//
// Created by Andres Jaimes on 26/06/25.
//

#include <catch2/catch_test_macros.hpp>
#include <httplib.h>
#include "../src/http_client.h"

TEST_CASE("encoding functions", "[http_client][encoding]") {

    sevilla::http_client http_client;

    SECTION("encode must correctly encode a string") {
        std::string input = "http://hello.world?a=1&b=some spaces";
        const std::string result = sevilla::http_client::encode(input);

        REQUIRE(result == "http%3A%2F%2Fhello.world%3Fa%3D1%26b%3Dsome%20spaces");
    }

    SECTION("encode_map must correctly encode a map") {
        std::map<std::string, std::string> input = {
            { "id", "abc-123#def" },
            { "url", "http://hello.world?a=1&b=some spaces" },
            { "name", "Joe Doe" },
        };
        const std::string result = sevilla::http_client::encode_map(input);

        REQUIRE(result == "id=abc-123%23def&name=Joe%20Doe&url=http%3A%2F%2Fhello.world%3Fa%3D1%26b%3Dsome%20spaces");
    }

}


TEST_CASE("query_params function", "[http_client][query_params]") {

    sevilla::http_client http_client;

    SECTION("query_params must update the instance's url") {
        std::map<std::string, std::string> query_params = {
            { "id", "abc-123#def" },
            { "url", "http://hello.world?a=1&b=some spaces" },
            { "name", "Joe Doe" },
        };
        http_client.url = "https://test.com/path";
        http_client.query_params = query_params;
        http_client.add_query_params();

        REQUIRE(http_client.url == "https://test.com/path?id=abc-123%23def&name=Joe%20Doe&url=http%3A%2F%2Fhello.world%3Fa%3D1%26b%3Dsome%20spaces");
    }

    SECTION("query_params must update the instance's url with existing params") {
        std::map<std::string, std::string> query_params = {
            { "id", "abc-123#def" },
            { "url", "http://hello.world?a=1&b=some spaces" },
            { "name", "Joe Doe" },
        };
        http_client.url = "https://test.com/path?s=2";
        http_client.query_params = query_params;
        http_client.add_query_params();

        REQUIRE(http_client.url == "https://test.com/path?s=2&id=abc-123%23def&name=Joe%20Doe&url=http%3A%2F%2Fhello.world%3Fa%3D1%26b%3Dsome%20spaces");
    }

    SECTION("query_params must update the instance's url that ends in '&'") {
        std::map<std::string, std::string> query_params = {
            { "id", "abc-123#def" },
            { "url", "http://hello.world?a=1&b=some spaces" },
            { "name", "Joe Doe" },
        };
        http_client.url = "https://test.com/path?s=2&";
        http_client.query_params = query_params;
        http_client.add_query_params();

        REQUIRE(http_client.url == "https://test.com/path?s=2&id=abc-123%23def&name=Joe%20Doe&url=http%3A%2F%2Fhello.world%3Fa%3D1%26b%3Dsome%20spaces");
    }

    SECTION("query_params must update the instance's url when url is empty") {
        std::map<std::string, std::string> query_params = {
            { "id", "abc-123#def" },
            { "url", "http://hello.world?a=1&b=some spaces" },
            { "name", "Joe Doe" },
        };
        http_client.query_params = query_params;
        http_client.add_query_params();

        REQUIRE(http_client.url == "?id=abc-123%23def&name=Joe%20Doe&url=http%3A%2F%2Fhello.world%3Fa%3D1%26b%3Dsome%20spaces");
    }

    SECTION("query_params must leave untouched the url if there are no query params") {
        std::map<std::string, std::string> query_params = { };
        http_client.url = "https://test.com/path?s=2";
        http_client.query_params = query_params;
        http_client.add_query_params();

        REQUIRE(http_client.url == "https://test.com/path?s=2");
    }

}

/*
 * Local server to test the make_request function.
 */
void run_http_server() {
    httplib::Server svr;

    svr.Get("/get", [](const httplib::Request &req, httplib::Response &res) {
        std::string value = req.get_param_value("value");
        res.status = 200;
        std::ostringstream oss;
        oss << "Sent: " << value;
        res.set_content(oss.str(), "text/plain");
    });

    svr.Get("/auth", [](const httplib::Request &req, httplib::Response &res) {
        std::string token = req.get_header_value("Authorization");
        res.status = 200;
        res.set_content("Authorization: " + token, "text/plain");
    });

    svr.Get("/status-500", [](const httplib::Request &req, httplib::Response &res) {
        res.status = 500;
        res.set_content("Simulated code: 500", "text/plain");
    });

    svr.Get("/slow-response", [](const httplib::Request &req, httplib::Response &res) {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        res.status = 200;
        res.set_content("Slow response", "text/plain");
    });

    svr.Post("/post", [](const httplib::Request &req, httplib::Response &res) {
        std::string content_type = req.get_header_value("Content-Type");
        std::string body = req.body;
        res.status = 200;
        res.set_content("Sent: " + body + " using: " + content_type, "text/plain");
    });

    // Special endpoint to stop the server
    svr.Get("/stop", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Server stopping...", "text/plain");
        res.status = 200;
        // Stop in a thread to avoid leaving the client hanging
        std::thread([&svr]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            svr.stop();
        }).detach(); // Stop after response is sent
    });

    svr.listen("127.0.0.1", 16435);
}

/*
 * Fixture to handle the lifespan of the web server.
 */
struct WebServerFixture {

    std::thread server_thread;

    // start the server
    WebServerFixture() {
        server_thread = std::thread(run_http_server);
        // Give the server a moment to start up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // stop the server
    ~WebServerFixture() {
        sevilla::http_client http_client;
        // call the stop endpoint
        http_client.url = "http://127.0.0.1:16435/stop";
        http_client.make_request();
        // wait for the server to stop
        if (server_thread.joinable()) server_thread.join();
    }
};

TEST_CASE_METHOD(WebServerFixture, "make_request function", "[http_client][make_request]") {

    sevilla::http_client http_client;

    SECTION("make a simple get request") {
        http_client.url = "http://127.0.0.1:16435/get?value=3";
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_OK);
        REQUIRE(http_client.status_code == 200);
        REQUIRE(http_client.response_body == "Sent: 3");
    }

    SECTION("make a simple post request") {
        http_client.url = "http://127.0.0.1:16435/post";
        http_client.method = "POST";
        http_client.headers["Content-Type"] = "application/x-www-form-urlencoded";
        http_client.form_params["a"] = "1";
        http_client.form_params["b"] = "2";
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_OK);
        REQUIRE(http_client.status_code == 200);
        REQUIRE(http_client.response_body == "Sent: a=1&b=2 using: application/x-www-form-urlencoded");
    }

    SECTION("make a post request with no form params") {
        http_client.url = "http://127.0.0.1:16435/post";
        http_client.method = "POST";
        http_client.headers["Content-Type"] = "application/json";
        http_client.request_body = R"({ "hello": "world" })";
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_OK);
        REQUIRE(http_client.status_code == 200);
        REQUIRE(http_client.response_body == R"(Sent: { "hello": "world" } using: application/json)");
    }

    SECTION("handle a remote error response") {
        http_client.url = "http://127.0.0.1:16435/status-500";
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_OK);
        REQUIRE(http_client.status_code == 500);
        REQUIRE(http_client.response_body == "Simulated code: 500");
    }

    SECTION("send basic auth") {
        http_client.url = "http://127.0.0.1:16435/auth";
        http_client.auth_basic_username = "user";
        http_client.auth_basic_password = "password";
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_OK);
        REQUIRE(http_client.status_code == 200);
        REQUIRE(http_client.response_body == "Authorization: Basic dXNlcjpwYXNzd29yZA==");
    }

    SECTION("send bearer auth") {
        http_client.url = "http://127.0.0.1:16435/auth";
        http_client.auth_bearer_token = "token";
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_OK);
        REQUIRE(http_client.status_code == 200);
        REQUIRE(http_client.response_body == "Authorization: Bearer token");
    }

    SECTION("handle a timeout because of a slow server response") {
        http_client.url = "http://127.0.0.1:16435/slow-response";
        http_client.max_timeout = 100; // 100 milliseconds
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_OPERATION_TIMEDOUT);
        REQUIRE(http_client.error_message == "Timeout was reached");
    }

    SECTION("handle a connection timeout") {
        http_client.url = "http://127.0.0.1:4321/"; // invalid port
        http_client.connection_timeout = 100; // 100 milliseconds
        http_client.make_request();

        REQUIRE(http_client.error == CURLE_COULDNT_CONNECT);
        REQUIRE(http_client.error_message == "Could not connect to server");
    }

}
