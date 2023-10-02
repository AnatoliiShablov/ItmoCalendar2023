project(FrontAPI)

add_library(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../../API/FrontAPI.proto")
target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf gRPC::grpc++)
if (UNIX)
    find_package(libsystemd REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE libsystemd::libsystemd)
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/protocols")

protobuf_generate(TARGET ${PROJECT_NAME}
        IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../API"
        LANGUAGE cpp
        PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/protocols")

protobuf_generate(TARGET ${PROJECT_NAME}
        IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../API"
        LANGUAGE grpc
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
        PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
        PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/protocols")

add_library(protocols::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} LIBRARY)
