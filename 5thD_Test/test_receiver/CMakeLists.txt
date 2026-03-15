cmake_minimum_required(VERSION 3.20)
project(5thDreceiverTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB TESTS_RECEIVER
    "../../core/5thdlogger.cpp"
    "../../core/izmq.cpp"
    "../../core/5thdipcmsg.c"
    "../../core/receiver.cpp"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS_RECEIVER})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    spdlog::spdlog 
    fifthd_sodium
    unity
    fifthd_zmq
)
