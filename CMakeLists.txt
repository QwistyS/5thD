cmake_minimum_required(VERSION 3.20)
project(5thD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# support clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_definitions(BIN_PATH="${CMAKE_SOURCE_DIR}/bin/")

set(DB_PATH "${PROJECT_SOURCE_DIR}/bin/5thd.db" CACHE STRING "Path to the database file")
add_compile_definitions(DB_PATH="${DB_PATH}")

# Set the output directory for all targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
set(THIRDP ${CMAKE_SOURCE_DIR}/third_parties)
set(5thDlibs ${CMAKE_SOURCE_DIR}/bin)

# Include directories for core module headers
include_directories(
    core/common
    core
    ${THIRDP}/libzmq/include
    ${THIRDP}/sqlcipher/include
    ${THIRDP}/spdlog/include
)

add_subdirectory(${THIRDP}/Unity)

file(GLOB CORE_SRC_FILES
    core/receiver.cpp
    core/izmq.cpp
    core/5thdlogger.cpp
    core/5thdsql.cpp
    core/keys_db.cpp
    core/transmitter.cpp
    core/5thdipc_client.cpp
    core/module.cpp
    core/5thdallocator.cpp
    core/5thdipcmsg.c
)

# Set the path for the library files
if(MSVC)
    set(LIB_EXTENSION ".dll")
elseif(APPLE)
    set(LIB_EXTENSION ".dylib")
else()
    set(LIB_EXTENSION ".so")
endif()
set(ZMQ_LIB ${5thDlibs}/libzmq${LIB_EXTENSION})
set(SQLCIPHER_LIB ${5thDlibs}/libsqlcipher${LIB_EXTENSION})
set(SPDLOG_LIB ${5thDlibs}/libspdlog${LIB_EXTENSION})

add_library(zmq SHARED IMPORTED)
set_target_properties(zmq PROPERTIES IMPORTED_LOCATION ${ZMQ_LIB})

add_library(spdlog SHARED IMPORTED)
set_target_properties(spdlog PROPERTIES IMPORTED_LOCATION ${SPDLOG_LIB})

add_library(sqlcipher SHARED IMPORTED)
set_target_properties(sqlcipher PROPERTIES IMPORTED_LOCATION ${SQLCIPHER_LIB})

message("ZMQ_LIB full path: ${ZMQ_LIB}")
message("SPDLOG_LIB full path: ${SPDLOG_LIB}")
message("SQLCIPHER_LIB full path: ${SQLCIPHER_LIB}")

# Set the source files for your Qt application
set(SOURCES main.cpp)

# Add an executable target for the main application
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${CORE_SRC_FILES}
)

# Link libraries and other dependencies to the main executable
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    zmq
    sodium
    sqlcipher
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
    )
endif()

# add_subdirectory(5thD_Software_Bus)
# add_subdirectory(5thD_Peer)
add_subdirectory(5thD_Test)
