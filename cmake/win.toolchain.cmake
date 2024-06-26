# Toolchain file for cross-compiling to Windows using MinGW-w64
SET(CMAKE_SYSTEM_NAME Windows)

# Define the compiler to use
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# Define the path to the MinGW-w64 libraries
SET(CMAKE_FIND_ROOT_PATH /usr/local/opt/mingw-w64)

# Adjust the search paths for finding libraries and headers
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
