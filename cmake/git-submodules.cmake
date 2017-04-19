
message(STATUS "Checking for submodule updates...")
if (NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/.git")
    message("Project is no git working copy, skipping submodule check...")
    return()
endif()

find_package(Git)
if(NOT GIT_FOUND)
    message("Git could not be found, skipping submodule check...")
    return()
endif()

# Update the submodules
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        RESULT_VARIABLE UPDATE_RESULT
        OUTPUT_VARIABLE GIT_OUTPUT
        ERROR_VARIABLE GIT_OUTPUT)

if (GIT_OUTPUT)
    message(STATUS "${GIT_OUTPUT}")
endif()

if (UPDATE_RESULT)
    message("Submodule updating has failed!")
endif()
