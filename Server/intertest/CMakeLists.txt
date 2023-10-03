project(intertests)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

conan_cmake_configure(
        REQUIRES
        gtest/1.14.0
        GENERATORS
        cmake_find_package
)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings}
)

find_package(GTest REQUIRED)

add_executable(intertests intertests.cpp)
target_link_libraries(intertests PRIVATE service::DB service::Calendar GTest::GTest spdlog::spdlog protocols::FrontAPI)