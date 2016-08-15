
include(source_groups.cmake)

add_executable(unittests ${source_files})
target_link_libraries(unittests PRIVATE gtest)
target_link_libraries(unittests PRIVATE code)

target_include_directories(unittests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(unittests PROPERTIES FOLDER "tests")

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../test_data" TEST_DATA_PATH)
string(REPLACE "\\" "\\\\" TEST_DATA_PATH "${TEST_DATA_PATH}")
target_compile_definitions(unittests PRIVATE "TEST_DATA_PATH=\"${TEST_DATA_PATH}\"")
