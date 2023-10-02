project(FrontAPI)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

if (UNIX)
    conan_cmake_configure(
            REQUIRES
            zlib/1.3
            libsystemd/253.6
            libxcrypt/4.4.16
            grpc/1.50.1
            GENERATORS
            cmake_find_package
    )
endif ()

if (WIN32)
    conan_cmake_configure(
            REQUIRES
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

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)


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
