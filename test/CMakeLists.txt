
set(gtest_force_shared_crt "${MSVC_USE_RUNTIME_DLL}" CACHE BOOL "Always use msvcrt.dll" FORCE)

set(GOOGLETEST_VERSION 1.10.0)
add_subdirectory(gtest)

target_compile_definitions(gtest PUBLIC "$<$<CONFIG:FastDebug>:_ITERATOR_DEBUG_LEVEL=0>")

set_target_properties(gtest PROPERTIES FOLDER "3rdparty")

add_subdirectory(src)
