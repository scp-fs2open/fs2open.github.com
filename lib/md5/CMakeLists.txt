
SET(MD5_SOURCES
        md5.cpp
        md5.h)

ADD_LIBRARY(md5 STATIC ${MD5_SOURCES})

target_include_directories(md5 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(md5 PROPERTIES FOLDER "3rdparty")
