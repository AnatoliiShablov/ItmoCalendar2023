project(ServerInnerAPI)

add_library(${PROJECT_NAME} INTERFACE include/sapi/ServerInnerApi.hpp)
target_include_directories(${PROJECT_NAME} INTERFACE include)
add_library(api::server ALIAS ${PROJECT_NAME})
