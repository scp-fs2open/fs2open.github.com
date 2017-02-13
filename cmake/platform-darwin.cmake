
MESSAGE(STATUS "Doing Mac OSX specific things...")
target_compile_definitions(platform INTERFACE APPLE_APP)
SET(EXE_GUI_TYPE MACOSX_BUNDLE)

FIND_LIBRARY(COCOA_LIBRARY Cocoa)

if (NOT COCOA_LIBRARY)
    message(FATAL_ERROR "Couldn't find Cocoa!")
endif()

target_link_libraries(platform INTERFACE ${COCOA_LIBRARY})

SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")

SET(CMAKE_SKIP_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH @loader_path/../Frameworks/)

# Set the path where library dependencies are copied to
SET(LIBRAY_DESTINATION "../Frameworks")

set(PLATFORM_MAC TRUE CACHE INTERNAL "" FORCE)
