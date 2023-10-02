cmake_minimum_required(VERSION 3.5)

project(ItmoCalendar2023 LANGUAGES CXX)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

conan_cmake_configure(
        REQUIRES
        fmt/10.1.1
        GENERATORS
        cmake_find_package
)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings}
)

find_package(fmt REQUIRED)

add_subdirectory(ServerInnerAPI)
add_subdirectory(DBService)
add_subdirectory(CalendarService)
add_subdirectory(NotifierService)

add_subdirectory(proto)

add_library(utils_parse INTERFACE utils/ParseUtils.hpp)
target_include_directories(utils_parse INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_library(utils::parse ALIAS utils_parse)

add_executable(Server server.cpp)
target_link_libraries(Server PRIVATE protocols::FrontAPI fmt::fmt utils::parse service::DB service::Calendar service::Notifier)
