project(Calendar)
find_package(Threads REQUIRED)

set(PROJECT_NAME_SERVICE ${PROJECT_NAME}Service)
set(PROJECT_NAME_IMPL ${PROJECT_NAME}Impl)
set(PROJECT_NAME_SERVICE_IMPL ${PROJECT_NAME_SERVICE}Impl)

add_library(${PROJECT_NAME_SERVICE_IMPL} include/service/details/${PROJECT_NAME_SERVICE_IMPL}.hpp src/${PROJECT_NAME_SERVICE_IMPL}.cpp)
target_include_directories(${PROJECT_NAME_SERVICE_IMPL} PUBLIC include)
target_link_libraries(${PROJECT_NAME_SERVICE_IMPL} PUBLIC protocols::FrontAPI service::DB)
target_link_libraries(${PROJECT_NAME_SERVICE_IMPL} PRIVATE utils::parse fmt::fmt spdlog::spdlog)
add_library(service::${PROJECT_NAME_IMPL} ALIAS ${PROJECT_NAME_SERVICE_IMPL})

add_library(${PROJECT_NAME_SERVICE} include/service/${PROJECT_NAME_SERVICE}.hpp src/${PROJECT_NAME_SERVICE}.cpp)
target_include_directories(${PROJECT_NAME_SERVICE} PUBLIC include)
target_link_libraries(${PROJECT_NAME_SERVICE} PUBLIC Threads::Threads service::DB api::server)
target_link_libraries(${PROJECT_NAME_SERVICE} PRIVATE service::${PROJECT_NAME_IMPL} fmt::fmt spdlog::spdlog)
add_library(service::${PROJECT_NAME} ALIAS ${PROJECT_NAME_SERVICE})
