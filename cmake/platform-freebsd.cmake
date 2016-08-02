
target_compile_definitions(platform INTERFACE SCP_BSD)

find_library(EXECINFO_LIB execinfo)

if (NOT EXECINFO_LIB)
    message(FATAL_ERROR "Couldn't find libexecinfo! Please install it and rerun the configuration.")
else()
    target_link_libraries(platform INTERFACE "${EXECINFO_LIB}")
endif()