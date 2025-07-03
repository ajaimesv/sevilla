
# Sevilla project

This project contains an array of functions for common tasks that I have found my self writing over and over on different languages. It compiles to a dynamic library that can be imported by other languages, thus getting access to the functions offered here.

The project offers a C-API that can interact with `char` and `wchar_t` types. For macOS and Linux, use the `char` functions. For Windows, the `wchar_t` functions.

Functionality is structured by the following file-naming convention:
- **something.cpp** / **something.h**: contain the C++ code, classes, functions, etc.  
- **something_c_api.cpp**: C-API.

So, for example, the `http_client` functionality lives in the `http_client.cpp`, and `http_client.h` files, while the `http_client_c_api.cpp` keeps the C-API functionality to interact with other programs.

We can easily remove unwanted functionality files, because they don't tend to depend on each other.

At the moment, functionality includes:
- **cvs_parser**: A csv line parser that allows quotes within fields.
- **email_client**: Want your app to send emails?
- **http_client**: Sends remote requests, like json and form requests.
- **utils**: some generic functions like `slugify`. 

The project uses:
- [nlohmann's json](https://github.com/nlohmann/json) library to expose single-point C-API's –See this post on my website on the [command-dispatch pattern](https://andres.jaimes.net/cpp/creating-a-c-api-interface/). ([license](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT))
- [libcurl](https://curl.se) to make remote requests, and send email functionality. ([license](https://curl.se/docs/copyright.html))

For unit testing:
- [catch2](https://github.com/catchorg/Catch2) to test most functionality. ([license](https://github.com/catchorg/Catch2/blob/devel/LICENSE.txt))
- [cpp-libhttp](https://github.com/yhirose/cpp-httplib) to test remote server interactions. ([license](https://github.com/yhirose/cpp-httplib/blob/master/LICENSE))

## vcpkg

The project uses `vcpkg` to manage its dependencies. Follow these instructions to install it.

From a directory other than the project's directory:

```shell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh --disableMetrics
```

Use `bootstrap-vcpkg.bat` instead, if using Windows.

Then, configure `cmake`'s **toolchain** to let it know where `vcpkg` is located. We can set it up in CLion, Visual Studio, or via the command line.

To set it up in CLion, follow the instructions from this page, it's very simple:

- https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

To set it up from the command line, enter the following commands from the project's directory: 

```shell
cd sevilla[Makefile](build/Makefile)
mkdir build
cd build
cmake .. \ 
  -DCMAKE_TOOLCHAIN_FILE=/full/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
make
```

Notice that the build directory for the previous example is `build`, which differs from the default used by CLion and VisualStudio.

To generate a release version:
```shell
mkdir build
cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/full/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
make
```

Note: A common option for the release version directory is `build_release`.

### References
- https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

## Unit Testing

We use `catch2` for unit testing. Configuration is simple and requires the following steps:

1. Add the `catch2` dependency to `vcpkg.json`
2. Add the **Unit Tests** section to `CMakeLists.txt`
3. Write a test `.cpp` file in the `tests` directory
4. Rebuild the project. `cd` into the **build** directory, and run `ctest`. The **build** directory is called `cmake-build-debug`, if using CLion.

**Note:** When adding test files via CLion, make sure they are only added to the `<project_name>_tests` project, otherwise the test build will fail. 
