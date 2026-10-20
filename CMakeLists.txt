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
target_link_libraries(MainServer_sha256 
    SharedMemoryUtils JobScheduler MessageQueueUtils SemaphoreUtils FileHandler Converter_sha256 RequestDataCollector ${OPENSSL_LIBRARIES})

# Main client executable
add_executable(MainClient_sha256 src/MainClient_sha256.c)
target_link_libraries(MainClient_sha256 
    SharedMemoryUtils JobScheduler MessageQueueUtils SemaphoreUtils FileHandler Converter_sha256 ${OPENSSL_LIBRARIES})

# Secondary client executable
add_executable(AdminClient src/AdminClient.c)
target_link_libraries(AdminClient MessageQueueUtils)

# Other user-defined libraries
add_library(Converter_sha256 src/Converter_sha256.c include/Converter_sha256.h)
add_library(SharedMemoryUtils src/SharedMemoryUtils.c include/SharedMemoryUtils.h)
add_library(JobScheduler src/JobScheduler.c include/JobScheduler.h)
add_library(MessageQueueUtils src/MessageQueueUtils.c include/MessageQueueUtils.h)
add_library(SemaphoreUtils src/SemaphoreUtils.c include/SemaphoreUtils.h)
add_library(FileHandler src/FileHandler.c include/FileHandler.h)
add_library(RequestDataCollector src/RequestDataCollector.c include/RequestDataCollector.h)