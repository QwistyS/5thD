
# Include directories for core module headers
include_directories(
    ../core/common
    core/inc
    ${FIFTHD_ZMQ_INCLUDE_DIR}
    ${FIFTHD_SQLCIPHER_INCLUDE_DIR}
    ${FIFTHD_UNITY_DIR}/src

)


if(MSVC)
    set(LIB_EXTENSION ".dll")
elseif(APPLE)
    set(LIB_EXTENSION ".dylib")
else()
    set(LIB_EXTENSION ".so")
endif()

add_subdirectory(test_izmq)
add_subdirectory(test_receiver)
add_subdirectory(test_transmitter)
