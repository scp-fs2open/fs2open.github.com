
SET(MD5_SOURCES
        md5.cpp
        md5.h)

ADD_LIBRARY(md5 STATIC ${MD5_SOURCES})

target_include_directories(md5 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Disable warnings if building from source
suppress_warnings(md5)

set_target_properties(md5 PROPERTIES FOLDER "3rdparty")
target_link_libraries(md5 PUBLIC compiler)
