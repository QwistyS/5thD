cmake_minimum_required(VERSION 3.20)
project(5thDSoftwareBus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for core module headers
include_directories(
    ../core/
    core/inc
)

file(GLOB SOFT_BUS_FILES 
    "core/src/*.cpp"
    "../core/*.cpp"
)

# P}libzmq/includeAdd spdlog
# add_subdirectory(${THIRDP}spdlog)

# Set the source files for your Qt application
set(SOURCES soft_bus.cpp)

# Add an executable target for the main application
add_executable(${PROJECT_NAME} ${SOURCES} ${SOFT_BUS_FILES})

# Link Qt6 libraries and other dependencies to the main executable
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    libzmq
    spdlog::spdlog
)
