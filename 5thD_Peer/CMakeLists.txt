cmake_minimum_required(VERSION 3.20)
project(5thDPeer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for core module headers
include_directories(
    ../core/
    core/inc/
)

file(GLOB PEER_FILES
    "core/src/*.cpp"
    "../core/*.cpp"
)

# Set the source files for your Qt application
set(SOURCES peer.cpp)

# Add an executable target for the main application
add_executable(${PROJECT_NAME} ${SOURCES} ${PEER_FILES})

# Link Qt6 libraries and other dependencies to the main executable
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    libzmq
    OpenSSL::SSL
    miniupnpc::miniupnpc
    CURL::libcurl
    spdlog::spdlog
)
