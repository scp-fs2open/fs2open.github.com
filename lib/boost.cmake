include(util)

if (WIN32)
    set(Boost_USE_STATIC_LIBS       ON)
    set(Boost_USE_MULTITHREADED     ON)

    if(MSVC_USE_RUNTIME_DLL)
        set(Boost_USE_STATIC_RUNTIME    OFF)
    else(MSVC_USE_RUNTIME_DLL)
        set(Boost_USE_STATIC_RUNTIME    ON)
    endif(MSVC_USE_RUNTIME_DLL)
elseif(PLATFORM_MAC)
    set(Boost_USE_STATIC_LIBS       ON)
endif()

find_package(Boost REQUIRED COMPONENTS thread system chrono date_time)

add_library(boost INTERFACE)

target_include_directories(boost INTERFACE "${Boost_INCLUDE_DIRS}")

# Disable automatic linking
target_compile_definitions(boost INTERFACE "BOOST_ALL_NO_LIB")

CONVERT_OLD_LIBRARIES(${Boost_LIBRARIES})
target_link_libraries(boost INTERFACE "${CONVERTED_LIBRARIES}")
