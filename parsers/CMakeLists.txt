
function(add_parser filename)
    get_filename_component(_name "${filename}" NAME_WE)
    get_filename_component(_absolutePath "${filename}" ABSOLUTE)

    get_filename_component(_directory "${_absolutePath}" DIRECTORY)
    get_filename_component(_directoryName "${_directory}" NAME)

    if (ANTLR_FOUND)
        antlr_target(${_name} "${_absolutePath}"
            VISITOR
            OUTPUT_DIRECTORY "${_directory}/generated")
        target_sources(parsers PRIVATE "${_absolutePath}")
        source_group("${_directoryName}" FILES "${_absolutePath}")
    else()
        # Otherwise we just use the ones that are checked in
        set(ANTLR_${_name}_OUTPUTS
            ${_directory}/generated/${_name}BaseVisitor.cpp
            ${_directory}/generated/${_name}BaseVisitor.h
            ${_directory}/generated/${_name}Lexer.cpp
            ${_directory}/generated/${_name}Lexer.h
            ${_directory}/generated/${_name}Parser.cpp
            ${_directory}/generated/${_name}Parser.h
            ${_directory}/generated/${_name}Visitor.cpp
            ${_directory}/generated/${_name}Visitor.h
        )
    endif()

    target_sources(parsers PRIVATE ${ANTLR_${_name}_OUTPUTS})
    source_group("${_directoryName}\\generated" FILES ${ANTLR_${_name}_OUTPUTS})
endfunction()

add_library(parsers STATIC)

target_link_libraries(parsers PUBLIC antlr4_static)

find_package(ANTLR)

if (ANTLR_FOUND)
    # If we have an ANTLR installation then we can generate new sources
    message(STATUS "Using ANTLR to generate parser")
else()
    message(STATUS "Using generated ANTLR files")
endif()

add_parser(arg_parser/ArgumentList.g4)
add_parser(action_expression/ActionExpression.g4)

target_include_directories(parsers PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

suppress_warnings(parsers)
