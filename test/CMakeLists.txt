
set(gtest_force_shared_crt "${MSVC_RELEASE_DEBUGGING}")

add_subdirectory(gtest)

set_target_properties(gtest PROPERTIES FOLDER "3rdparty")

add_subdirectory(src)
