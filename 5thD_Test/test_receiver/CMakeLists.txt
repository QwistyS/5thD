cmake_minimum_required(VERSION 3.20)
project(test_receiver)

file(GLOB TESTS_RECEIVER
    "../../core/5thdlogger.cpp"
    "../../core/izmq.cpp"
    "../../core/5thdipcmsg.c"
    "../../core/receiver.cpp"
    "../../core/5thdipcmsg.c"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS_RECEIVER})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    sodium
    unity
    zmq
)
