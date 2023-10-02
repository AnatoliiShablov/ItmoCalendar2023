project(DB)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

conan_cmake_configure(
        REQUIRES
        sqlite_orm/1.8.2
        GENERATORS
        cmake_find_package
)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings}
)

find_package(SqliteOrm REQUIRED)

set(PROJECT_NAME_SERVICE ${PROJECT_NAME}Service)

add_library(${PROJECT_NAME_SERVICE} include/service/${PROJECT_NAME_SERVICE}.hpp src/${PROJECT_NAME_SERVICE}.cpp)
target_include_directories(${PROJECT_NAME_SERVICE} PUBLIC include)
target_link_libraries(${PROJECT_NAME_SERVICE} PUBLIC api::server)
target_link_libraries(${PROJECT_NAME_SERVICE} PRIVATE sqlite_orm::sqlite_orm fmt::fmt spdlog::spdlog)
add_library(service::${PROJECT_NAME} ALIAS ${PROJECT_NAME_SERVICE})
