cmake_minimum_required(VERSION 3.20)
project(test_ipzmq)


file(GLOB TESTS_IZMQ
    "../../core/5thdlogger.cpp"
    "../../core/5thdipcmsg.c"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS_IZMQ})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    unity
    zmq
)
