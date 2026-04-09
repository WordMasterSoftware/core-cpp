cmake_minimum_required(VERSION 3.28)

project(WMNextBackend LANGUAGES CXX)

include(cmake/flite.cmake)

# 导出编译数据库
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 使用 GCC 15.2.0
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.2)
    message(FATAL_ERROR "WMNextBackend requires GCC 15.2.0 or newer. Current version: ${CMAKE_CXX_COMPILER_VERSION}")
endif()

find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)

# CONFIGURE_DEPENDS 会让 CMake 在重新配置时感知新增或删除的源码文件。
file(GLOB_RECURSE WMNEXT_BACKEND_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(wmnext_backend ${WMNEXT_BACKEND_SOURCES})
add_dependencies(wmnext_backend wmnext_flite_project)

target_include_directories(wmnext_backend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(wmnext_backend
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
        websocketpp::websocketpp
        asio::asio
        libpqxx::pqxx
        redis++::redis++_static
        wmnext_flite
)

target_precompile_headers(wmnext_backend
    PRIVATE
        src/pch.hpp
)
