//
// Created by Andres Jaimes on 28/06/25.
//

#include <dlfcn.h>
#include <catch2/catch_test_macros.hpp>
#include <wchar.h>

#if defined(_WIN32)
    #define LIBNAME "sevilla.dll"
#elif defined(__APPLE__)
    #define LIBNAME "libsevilla.dylib"
#else
    #define LIBNAME "libsevilla.so"
#endif

struct LoaderFixture {

    typedef const char* (*slugify_func)(const char*);
    slugify_func slugify;
    typedef const wchar_t* (*slugify_w_func)(const wchar_t*);
    slugify_w_func slugify_w;
    void* handle = nullptr;

    // load the dynamic library
    LoaderFixture() {
        handle = dlopen(LIBNAME, RTLD_NOW);
        if (handle != nullptr) {
            slugify = reinterpret_cast<slugify_func>(dlsym(handle, "sv_slugify"));
            slugify_w = reinterpret_cast<slugify_w_func>(dlsym(handle, "sv_slugify_w"));
        }
    }

    // unload the dynamic library
    ~LoaderFixture() {
        if (handle != nullptr) {
            dlclose(handle);
        }
    }
};

TEST_CASE_METHOD(LoaderFixture, "slugify c-api", "[utils][slugify][shared]") {

    SECTION("correctly converts a string") {
        const char* s = "Hello, World! - Testing, 123.";
        const char* result = slugify(s);
        REQUIRE(strcmp(result, "hello-world-testing-123") == 0);
    }

    SECTION("correctly handles a null value") {
        const char* s = nullptr;
        const char* result = slugify(s);
        REQUIRE(strcmp(result, "") == 0);
    }

}

TEST_CASE_METHOD(LoaderFixture, "slugify c-api for chart_w", "[utils][slugify][shared]") {

    SECTION("correctly converts a string") {
        const wchar_t* s = L"Hello, World! - Testing, 123.";
        const wchar_t* result = slugify_w(s);
        REQUIRE(wcscmp(result, L"hello-world-testing-123") == 0);
    }

    SECTION("correctly handles a null value") {
        const wchar_t* s = nullptr;
        const wchar_t* result = slugify_w(s);
        REQUIRE(wcscmp(result, L"") == 0);
    }

}

