
# Include directories for core module headers
include_directories(
    ../core/common
    core/inc
    /home/qwistys/src/5thD/third_parties/libzmq/include
    /home/qwistys/src/5thD/third_parties/sqlcipher/include
    /home/qwistys/src/5thD/third_parties/Unity/src

)

add_subdirectory(test_izmq)
add_subdirectory(test_receiver)
add_subdirectory(test_transmitter)
add_subdirectory(test_proc)