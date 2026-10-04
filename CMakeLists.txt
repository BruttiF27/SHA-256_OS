cmake_minimum_required(VERSION 3.20)

project(SHA256 VERSION 1.0)

set(CMAKE_MACOSX_RPATH 1)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(-Wno-deprecated-declarations)

find_package(OpenSSL REQUIRED)

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})

# Server executable
add_executable(MainServer_sha256 src/MainServer_sha256.c include/MainServer_sha256.h)
target_link_libraries(MainServer_sha256 FileHandler Converter_sha256 CommunicationHandler ${OPENSSL_LIBRARIES})

# Client executable
add_executable(MainClient_sha256 src/MainClient_sha256.c)
target_link_libraries(MainClient_sha256 FileHandler Converter_sha256 CommunicationHandler ${OPENSSL_LIBRARIES})

# Other user-defined libraries
add_library(FileHandler src/FileHandler.c include/FileHandler.h)
add_library(Converter_sha256 src/Converter_sha256.c include/Converter_sha256.h)
add_library(CommunicationHandler src/CommunicationHandler.c include/CommunicationHandler.h)