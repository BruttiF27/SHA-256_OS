cmake_minimum_required(VERSION 3.20)

project(SHA256 VERSION 1.0)

set(CMAKE_MACOSX_RPATH 1)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(-Wno-deprecated-declarations)

find_package(OpenSSL REQUIRED)

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})

add_executable(MainServer_sha256 src/MainServer_sha256.c
    src/Utils.h)
target_link_libraries(MainServer_sha256 ${OPENSSL_LIBRARIES})

#add_executable(MainClient_sha256.c src/MainClient_sha256.c)
#target_link_libraries(MainClient_sha256 ${OPENSSL_LIBRARIES})