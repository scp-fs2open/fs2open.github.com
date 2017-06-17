
set(gtest_force_shared_crt "${MSVC_USE_RUNTIME_DLL}" CACHE BOOL "Always use msvcrt.dll" FORCE)

add_subdirectory(gtest)

set_target_properties(gtest PROPERTIES FOLDER "3rdparty")

add_subdirectory(src)
