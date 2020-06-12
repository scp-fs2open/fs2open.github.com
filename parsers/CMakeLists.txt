
add_library(parsers STATIC)

target_link_libraries(parsers PUBLIC antlr4_static)

find_package(ANTLR)

if (ANTLR_FOUND)
    # If we have an ANTLR installation then we can generate new sources
    message(STATUS "Using ANTLR to generate parser")
    antlr_target(ArgumentList "${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/ArgumentList.g4"
            VISITOR
            OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated")
    target_sources(parsers PRIVATE "arg_parser/ArgumentList.g4")
    source_group("arg_parser" FILES "arg_parser\\ArgumentList.g4")
else()
    # Otherwise we just use the ones that are checked in
    message(STATUS "Using generated ANTLR files")
    set(ANTLR_ArgumentList_OUTPUTS
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListBaseVisitor.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListBaseVisitor.h
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListLexer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListLexer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListParser.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListParser.h
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListVisitor.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/arg_parser/generated/ArgumentListVisitor.h
            )
endif()

target_sources(parsers PRIVATE ${ANTLR_ArgumentList_OUTPUTS})
source_group("arg_parser\\generated" FILES ${ANTLR_ArgumentList_OUTPUTS})

target_include_directories(parsers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

suppress_warnings(parsers)
