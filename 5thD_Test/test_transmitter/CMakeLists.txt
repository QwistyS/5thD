cmake_minimum_required(VERSION 3.20)
project(test_transmitter)

file(GLOB TESTS_TRANSMITTER
    "../../core/5thdlogger.cpp"
    "../../core/izmq.cpp"
    "../../core/5thdipcmsg.c"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS_TRANSMITTER})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    unity
    zmq
)
