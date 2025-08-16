cmake_minimum_required(VERSION 3.16)

project(qtfred VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(QT6_INSTALL_ROOT "" CACHE PATH
"The path to the Qt6 installation root. May be necessary on windows if the standard find_package fails to find the Qt installation.")
list(APPEND CMAKE_PREFIX_PATH "${QT6_INSTALL_ROOT}")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL)
include(source_groups.cmake)
qt_standard_project_setup()
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
INCLUDE(InstallRequiredSystemLibraries)
qt6_wrap_ui(QTFRED_UI_GENERATED ${files_UI})
source_group("UI\\Generated"            FILES ${QTFRED_UI_GENERATED})
qt_add_executable(qtfred
        ${source_files}
    ${QTFRED_UI_GENERATED}
)
set_target_properties(qtfred PROPERTIES
        AUTOUIC FALSE)
set_target_properties(qtfred PROPERTIES OUTPUT_NAME "qtfred_${FSO_BINARY_SUFFIX}")

target_compile_definitions(qtfred PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:PDB_DEBUGGING=1>")

# Undefine emit since it conflicts with our code
target_compile_definitions(qtfred PRIVATE QT_NO_EMIT)

target_compile_definitions(qtfred PUBLIC USING_THIRD_PARTY_LIBS FRED)

target_include_directories(qtfred PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(CMAKE_MAP_IMPORTED_CONFIG_FASTDEBUG Release Debug)

target_link_libraries(qtfred PUBLIC code Qt6::Widgets Qt6::OpenGL)

include(CreateLaunchers)
create_target_launcher(qtfred
    WORKING_DIRECTORY ${FSO_FREESPACE_PATH}
    ARGS ${FSO_RUN_ARGUMENTS})

INSTALL(
    TARGETS qtfred
    RUNTIME DESTINATION ${BINARY_DESTINATION}
    COMPONENT "qtFRED"
    BUNDLE DESTINATION ${BINARY_DESTINATION}
    COMPONENT "qtFRED"
)
enable_clang_tidy(qtfred)
COPY_FILES_TO_TARGET(qtfred)
if(WIN32)
# Retrieve the absolute path to qmake and then use that path to find
# the windeployqt executable
get_target_property(QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR ${QMAKE_EXE} DIRECTORY)

find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

# Run windeployqt immediately after build
add_custom_command(TARGET qtfred
    POST_BUILD
    COMMAND Qt6::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:qtfred>/$<TARGET_FILE_NAME:qtfred>"
    )
install(
        DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
        DESTINATION ${BINARY_DESTINATION}
 )
endif()

if (FSO_INSTALL_DEBUG_FILES)
    if (MSVC)
        install(FILES "$<TARGET_PDB_FILE:qtfred>"
                DESTINATION ${BINARY_DESTINATION}
                OPTIONAL)
    endif()
endif()