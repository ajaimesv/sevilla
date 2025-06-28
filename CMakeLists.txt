cmake_minimum_required(VERSION 3.31)

project(sevilla)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
        src/c_api.cpp
        src/c_api.h
        src/csv_parser.cpp
        src/csv_parser.h
        src/csv_parser_c_api.cpp
        src/http_client.cpp
        src/http_client.h
        src/http_client_c_api.cpp
        src/email_client.cpp
        src/email_client.h
        src/email_client_c_api.cpp
        src/utils.cpp
        src/utils.h
        src/utils_c_api.cpp
)

# Static version, for unit testing
add_library(sevilla_static STATIC ${SOURCES})
set_target_properties(sevilla_static PROPERTIES OUTPUT_NAME "sevilla")

# Shared version
add_library(sevilla_shared SHARED ${SOURCES})
set_target_properties(sevilla_shared PROPERTIES OUTPUT_NAME "sevilla")

# Find the curl package
find_package(CURL REQUIRED)

# Link the curl library to our library
target_link_libraries(sevilla_static PRIVATE CURL::libcurl)
target_link_libraries(sevilla_shared PRIVATE CURL::libcurl)

# -------------------------------------
# Unit tests
# -------------------------------------
enable_testing()

find_package(Catch2 CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

add_executable(sevilla_tests
        tests/csv_parser_test.cpp
        tests/http_client_test.cpp
        tests/email_client_test.cpp
        tests/utils_test.cpp
        tests/utils_c_api_test.cpp
)

target_link_libraries(sevilla_tests PRIVATE
        Catch2::Catch2WithMain
        httplib::httplib
        sevilla_static
)

# Register the tests
include(CTest)
include(Catch)
catch_discover_tests(sevilla_tests)
