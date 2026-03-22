cmake_minimum_required(VERSION 3.16)

project(WMNextBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(wmnext_backend
    src/main.cpp
)

target_link_libraries(wmnext_backend
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
)
