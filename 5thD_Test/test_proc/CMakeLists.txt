cmake_minimum_required(VERSION 3.20)
project(test_proc)


file(GLOB TESTS_PROC
    "../../core/5thdlogger.cpp"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS_PROC})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    unity
)
