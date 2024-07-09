cmake_minimum_required(VERSION 3.20)
project(5thD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# set(DB_PATH "${PROJECT_SOURCE_DIR}/bin/5thd.db" CACHE STRING "Path to the database file")
# add_compile_definitions(DB_PATH="${DB_PATH}")

# Set the output directory for all targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
set(THIRDP "third_parties/")

# Include directories for core module headers
include_directories(
    core/common
    core
)

set(5thDlibs /home/qwistys/src/5thD/bin)


add_subdirectory(${THIRDP}/spdlog)

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

add_subdirectory(5thD_Software_Bus)
add_subdirectory(5thD_Peer)
