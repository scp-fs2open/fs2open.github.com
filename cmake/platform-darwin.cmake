
MESSAGE(STATUS "Doing Mac OSX specific things...")
SET(EXE_GUI_TYPE MACOSX_BUNDLE)

FIND_LIBRARY(COCOA_LIBRARY Cocoa)

if (NOT COCOA_LIBRARY)
    message(FATAL_ERROR "Couldn't find Cocoa!")
endif()

target_link_libraries(platform INTERFACE ${COCOA_LIBRARY})

set(PLATFORM_MAC TRUE)
