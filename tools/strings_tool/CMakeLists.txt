
find_package(Clang QUIET)

if (NOT Clang_FOUND)
    message(STATUS "Clang was not found, not building strings_tool")
    return()
endif()

add_executable(strings_tool EXCLUDE_FROM_ALL strings_tool.cpp)

set_target_properties(strings_tool
        PROPERTIES
        FOLDER "Tools"
)

target_link_libraries(strings_tool
        PRIVATE
        clangTooling
        clangBasic
        clangASTMatchers
        clangAST
        clangFrontend
        LLVM
        platform
        compiler
        )

# AddressSanitizer causes problems in LibTooling classes
string(REGEX REPLACE "-fsanitize=[a-zA-Z0-9,]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "-fsanitize=[a-zA-Z0-9,]+" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

find_package(Boost COMPONENTS regex)
if (Boost_FOUND)
    # Prefer to use boost::regex since the GCC implementation of std::regex is still buggy in version 7
    target_compile_definitions(strings_tool PRIVATE USE_BOOST_REGEX)
    target_link_libraries(strings_tool PRIVATE Boost::regex)
endif()
