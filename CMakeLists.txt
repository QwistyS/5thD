# CMakeLists.txt

cmake_minimum_required(VERSION 3.5)
project(5thD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for core module headers
include_directories(core/inc)

add_subdirectory(cppzmq)
# Add Unity as a submodule
add_subdirectory(Unity)

# Find spdlog package
add_subdirectory(spdlog)
# Find OpenSSl
find_package(OpenSSL REQUIRED)
# Find Qt stuff
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Set the source files for your Qt application
set(SOURCES main.cpp)

# Add all .cpp files from core/src directory
file(GLOB CORE_SRC_FILES "core/src/*.cpp")

# Add an executable target for the main application
add_executable(${PROJECT_NAME} ${SOURCES} ${CORE_SRC_FILES})

# Find the Qt6 package

# Link Qt6 libraries to the main executable
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    spdlog::spdlog
    cppzmq
    OpenSSL::SSL
)

