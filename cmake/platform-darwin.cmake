
MESSAGE(STATUS "Doing Mac OSX specific things...")
target_compile_definitions(platform INTERFACE APPLE_APP)
SET(EXE_GUI_TYPE MACOSX_BUNDLE)

FIND_LIBRARY(COCOA_LIBRARY Cocoa)

if (NOT COCOA_LIBRARY)
    message(FATAL_ERROR "Couldn't find Cocoa!")
endif()

target_link_libraries(platform INTERFACE ${COCOA_LIBRARY})

# To support C++17 we need to target at least 10.13. maybe 10.14 for some rare features
SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")

SET(CMAKE_SKIP_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH @loader_path/../Frameworks/)

# Set the path where library dependencies are copied to
SET(LIBRAY_DESTINATION "../Frameworks")

set(PLATFORM_MAC TRUE CACHE INTERNAL "" FORCE)

# Maybe override arm64 setting if we are compiling for different architecture
# If not specified, or if it's a universal build, then this should do nothing
if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
    set(IS_ARM64 TRUE)
elseif(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
    set(IS_ARM64 FALSE)
endif()

# Generate and don't strip debug symbols
# These settings don't seem to generate debug symbols for code and Freespace2
# so those settings are set manually in code and Freespace2's CMakeLists.txt files
set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")
set(CMAKE_XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO")
set(CMAKE_XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "NO")
set(CMAKE_XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "NO")

# Generate .dSYM file
set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
