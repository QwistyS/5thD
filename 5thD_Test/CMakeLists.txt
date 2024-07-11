
# Include directories for core module headers
include_directories(
    ../core/common
    core/inc
    /home/qwistys/src/5thD/third_parties/libzmq/include
    /home/qwistys/src/5thD/third_parties/sqlcipher/include
    /home/qwistys/src/5thD/third_parties/Unity/src

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