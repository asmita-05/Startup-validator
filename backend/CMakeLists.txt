cmake_minimum_required(VERSION 3.14)
project(StartupValidator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find packages ─────────────────────────────
find_package(CURL REQUIRED)

# ── Include FetchContent for Crow & nlohmann/json ──
include(FetchContent)

FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG        v1.0+5
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(crow nlohmann_json)

# ── Build the executable ──────────────────────
add_executable(startup_validator main.cpp)

target_include_directories(startup_validator PRIVATE
    ${crow_SOURCE_DIR}/include
    ${nlohmann_json_SOURCE_DIR}/include
)

target_link_libraries(startup_validator PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Crow needs Asio (header-only, bundled)
find_package(Threads REQUIRED)
target_link_libraries(startup_validator PRIVATE Threads::Threads)

# Optional: set output directory
set_target_properties(startup_validator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
