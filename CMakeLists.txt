cmake_minimum_required(VERSION 3.16)
project(currency_converter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.2
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(json cpr)

add_executable(currency
    src/main.cpp
    src/api.cpp
    src/cache.cpp
    src/converter.cpp
)

target_link_libraries(currency PRIVATE cpr::cpr nlohmann_json::nlohmann_json)
