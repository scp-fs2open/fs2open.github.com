CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( OpenDDL-Parser )
SET ( OPENDDL_PARSER_VERSION_MAJOR 0 )
SET ( OPENDDL_PARSER_VERSION_MINOR 1 )
SET ( OPENDDL_PARSER_VERSION_PATCH 0 )
SET ( OPENDDL_PARSER_VERSION ${OPENDDL_PARSER_VERSION_MAJOR}.${OPENDDL_PARSER_VERSION_MINOR}.${OPENDDL_PARSER_VERSION_PATCH} )
SET ( PROJECT_VERSION "${OPENDDL_PARSER_VERSION}" )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
    find_package(Threads)
else()
    add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
endif()

add_definitions( -DOPENDDLPARSER_BUILD )
add_definitions( -D_VARIADIC_MAX=10 )
add_definitions( -DGTEST_HAS_PTHREAD=0 )

if( WIN32 AND NOT CYGWIN )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc" )  # Force to always compile with W4
  if( CMAKE_CXX_FLAGS MATCHES "/W[0-4]" )
    string( REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
  else()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
  endif()
elseif( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic ${OPENDDL_CXXFLAGS}")
elseif ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic ${OPENDDL_CXXFLAGS} -Wwrite-strings")
endif()

# Include the doc component.
FIND_PACKAGE( doxygen )
IF ( DOXYGEN_FOUND )
    CONFIGURE_FILE( doc/openddlparser_doc.in doc/doxygenfile @ONLY )
    ADD_CUSTOM_TARGET( doc ALL ${DOXYGEN_EXECUTABLE} doc/doxygenfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM )
ENDIF ( DOXYGEN_FOUND )

SET ( openddl_parser_src
    code/OpenDDLCommon.cpp
    code/OpenDDLExport.cpp
    code/OpenDDLParser.cpp
    code/OpenDDLStream.cpp
    code/DDLNode.cpp
    code/Value.cpp
    include/openddlparser/OpenDDLCommon.h
    include/openddlparser/OpenDDLExport.h
    include/openddlparser/OpenDDLParser.h
    include/openddlparser/OpenDDLParserUtils.h
    include/openddlparser/OpenDDLStream.h
    include/openddlparser/DDLNode.h
    include/openddlparser/Value.h
    include/openddlparser/TPoolAllocator.h
)
 
SOURCE_GROUP( code            FILES ${openddl_parser_src} )

ADD_LIBRARY( openddl_parser STATIC
	${openddl_parser_src}
)

target_include_directories(openddl_parser PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(openddl_parser
    PROPERTIES
    FOLDER "3rdparty/openddl-parser"
)

target_link_libraries(openddl_parser PUBLIC compiler)