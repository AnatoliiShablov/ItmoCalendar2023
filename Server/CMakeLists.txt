cmake_minimum_required(VERSION 3.5)

project(ItmoCalendar2023 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX)
    conan_cmake_configure(
            REQUIRES
            fmt/10.1.1
            zlib/1.3
            libsystemd/253.6
            libxcrypt/4.4.16
            grpc/1.54.3
            GENERATORS
            cmake_find_package
    )
endif ()

if (WIN32)
    conan_cmake_configure(
            REQUIRES
            fmt/10.1.1
            grpc/1.50.1
            GENERATORS
            cmake_find_package
    )
endif ()

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings}
)

find_package(fmt REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

add_subdirectory(proto)

add_library(utils_parse INTERFACE utils/ParseUtils.hpp)
target_include_directories(utils_parse INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_library(utils::parse ALIAS utils_parse)

add_executable(Server server.cpp)
target_link_libraries(Server PRIVATE protocols::FrontAPI fmt::fmt utils::parse)
