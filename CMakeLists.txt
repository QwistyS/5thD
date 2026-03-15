cmake_minimum_required(VERSION 3.20)
project(5thD)

# support clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# set(DB_PATH "${PROJECT_SOURCE_DIR}/bin/5thd.db" CACHE STRING "Path to the database file")
# add_compile_definitions(DB_PATH="${DB_PATH}")

# Set the output directory for all targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
set(THIRDP "${CMAKE_CURRENT_SOURCE_DIR}/third_parties")

set(FIFTHD_THIRD_PARTY_DIR "${THIRDP}")
set(FIFTHD_PREBUILT_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin" CACHE PATH
    "Directory containing prebuilt libzmq/sqlcipher shared libraries")
set(5thDlibs "${FIFTHD_PREBUILT_LIB_DIR}")

set(FIFTHD_ZMQ_INCLUDE_HINT "${FIFTHD_THIRD_PARTY_DIR}/libzmq/include")
set(FIFTHD_SQLCIPHER_INCLUDE_HINT "${FIFTHD_THIRD_PARTY_DIR}/sqlcipher/include")
set(FIFTHD_SODIUM_INCLUDE_HINT "${FIFTHD_THIRD_PARTY_DIR}/libsodium/src/libsodium/include")
set(FIFTHD_UNITY_DIR "${FIFTHD_THIRD_PARTY_DIR}/Unity")

if(MSVC)
    set(FIFTHD_SHARED_LIB_EXT ".dll")
elseif(APPLE)
    set(FIFTHD_SHARED_LIB_EXT ".dylib")
else()
    set(FIFTHD_SHARED_LIB_EXT ".so")
endif()

find_path(FIFTHD_ZMQ_INCLUDE_DIR
    NAMES zmq.h
    HINTS "${FIFTHD_ZMQ_INCLUDE_HINT}")
find_path(FIFTHD_SQLCIPHER_INCLUDE_DIR
    NAMES sqlcipher/sqlite3.h
    HINTS "${FIFTHD_SQLCIPHER_INCLUDE_HINT}")
find_path(FIFTHD_SODIUM_INCLUDE_DIR
    NAMES sodium.h
    HINTS "${FIFTHD_SODIUM_INCLUDE_HINT}")

find_library(FIFTHD_ZMQ_LIBRARY
    NAMES zmq libzmq
    HINTS "${FIFTHD_PREBUILT_LIB_DIR}")
find_library(FIFTHD_SQLCIPHER_LIBRARY
    NAMES sqlcipher libsqlcipher
    HINTS "${FIFTHD_PREBUILT_LIB_DIR}")
find_library(FIFTHD_SODIUM_LIBRARY
    NAMES sodium libsodium
    HINTS "${FIFTHD_PREBUILT_LIB_DIR}")

# Include directories for core module headers
include_directories(
    core/common
    core
    ${CMAKE_CURRENT_SOURCE_DIR}/../extern/stuff/inc
)

if(EXISTS "${FIFTHD_THIRD_PARTY_DIR}/spdlog/CMakeLists.txt")
    add_subdirectory("${FIFTHD_THIRD_PARTY_DIR}/spdlog")
else()
    find_package(spdlog CONFIG QUIET)
    if(NOT TARGET spdlog::spdlog)
        message(STATUS "spdlog submodule is not initialized and spdlog package was not found.")
    endif()
endif()

set(FIFTHD_HAS_UNITY OFF)
if(EXISTS "${FIFTHD_UNITY_DIR}/CMakeLists.txt")
    add_subdirectory("${FIFTHD_UNITY_DIR}")
    set(FIFTHD_HAS_UNITY ON)
else()
    message(STATUS "Unity submodule is not initialized; skipping 5thD tests.")
endif()

if(FIFTHD_ZMQ_INCLUDE_DIR AND FIFTHD_ZMQ_LIBRARY AND NOT TARGET fifthd_zmq)
    add_library(fifthd_zmq UNKNOWN IMPORTED GLOBAL)
    set_target_properties(fifthd_zmq PROPERTIES
        IMPORTED_LOCATION "${FIFTHD_ZMQ_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FIFTHD_ZMQ_INCLUDE_DIR}")
endif()

if(FIFTHD_SQLCIPHER_INCLUDE_DIR AND FIFTHD_SQLCIPHER_LIBRARY AND NOT TARGET fifthd_sqlcipher)
    add_library(fifthd_sqlcipher UNKNOWN IMPORTED GLOBAL)
    set_target_properties(fifthd_sqlcipher PROPERTIES
        IMPORTED_LOCATION "${FIFTHD_SQLCIPHER_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FIFTHD_SQLCIPHER_INCLUDE_DIR}")
endif()

if(FIFTHD_SODIUM_LIBRARY AND NOT TARGET fifthd_sodium)
    add_library(fifthd_sodium UNKNOWN IMPORTED GLOBAL)
    set_target_properties(fifthd_sodium PROPERTIES
        IMPORTED_LOCATION "${FIFTHD_SODIUM_LIBRARY}")
    if(FIFTHD_SODIUM_INCLUDE_DIR)
        set_target_properties(fifthd_sodium PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${FIFTHD_SODIUM_INCLUDE_DIR}")
    endif()
endif()

file(GLOB CORE_SRC_FILES 
)

# Set the source files for your Qt application
set(SOURCES main.cpp)

# Add an executable target for the main application
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${CORE_SRC_FILES}
    )

# Link libraries and other dependencies to the main executable
if(APPLE)
    target_link_libraries(${PROJECT_NAME} 
        "-framework Cocoa" 
        "-framework IOKit" 
        "-framework CoreFoundation"
    )
else()
    target_link_libraries(${PROJECT_NAME} 
    )
endif()

set(FIFTHD_CAN_BUILD_NETWORK_TARGETS ON)
if(NOT TARGET spdlog::spdlog)
    set(FIFTHD_CAN_BUILD_NETWORK_TARGETS OFF)
endif()
foreach(_required_target fifthd_zmq fifthd_sqlcipher fifthd_sodium)
    if(NOT TARGET ${_required_target})
        set(FIFTHD_CAN_BUILD_NETWORK_TARGETS OFF)
    endif()
endforeach()

if(FIFTHD_CAN_BUILD_NETWORK_TARGETS)
    add_subdirectory(5thD_Software_Bus)
    add_subdirectory(5thD_Peer)
else()
    message(STATUS
        "Skipping 5thD_Software_Bus and 5thD_Peer (missing spdlog/zmq/sqlcipher/libsodium dependencies).")
endif()

if(FIFTHD_HAS_UNITY AND FIFTHD_CAN_BUILD_NETWORK_TARGETS)
    add_subdirectory(5thD_Test)
endif()
