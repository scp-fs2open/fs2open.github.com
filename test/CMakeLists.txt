
set(gtest_force_shared_crt "${FSO_MSVC_RUNTIME_DLL}" CACHE BOOL "Always use msvcrt.dll" FORCE)

set(GOOGLETEST_VERSION 1.10.0)
add_subdirectory(gtest)

set_target_properties(gtest PROPERTIES FOLDER "3rdparty")

add_subdirectory(src)
