cmake_minimum_required(VERSION 3.20)
project(5thDIzmqTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB TESTS_IZMQ
    "../../core/5thdlogger.cpp"
    "../../core/izmq.cpp"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS_IZMQ})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    spdlog::spdlog 
    unity
    fifthd_zmq
)
