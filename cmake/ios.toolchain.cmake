# ios.toolchain.cmake

# Set the target system name and version
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_SYSTEM_VERSION 14.0) # Change to the minimum iOS version you support
set(CMAKE_OSX_SYSROOT iphoneos)

# Set the path to your Qt installation
set(QT_ROOT "/path/to/your/qt")

# Specify the toolchain and platform
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")

# Set the architectures to build for
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# Code signing settings
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "YOUR_TEAM_ID")

# Bundle settings
set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")

# Set the default path for built executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include the Qt iOS toolchain
include(${QT_ROOT}/ios/mkspecs/features/toolchain.cmake)
