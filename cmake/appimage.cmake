
SET(BINARY_DESTINATION "bin")
SET(LIBRAY_DESTINATION "lib")

configure_file("${CMAKE_CURRENT_LIST_DIR}/AppRun.in" "${CMAKE_CURRENT_BINARY_DIR}/AppRun.gen" @ONLY)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AppRun-$<CONFIG>"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/AppRun.gen")

install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/AppRun-$<CONFIG>" DESTINATION "." RENAME "AppRun"
        COMPONENT "Freespace2")


configure_file("${CMAKE_CURRENT_LIST_DIR}/AppImage.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/AppImage.desktop.gen" @ONLY)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AppImage-$<CONFIG>.desktop"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/AppImage.desktop.gen")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/AppImage-$<CONFIG>.desktop" DESTINATION "." RENAME "fso.desktop"
        COMPONENT "Freespace2")

install(FILES "${CMAKE_SOURCE_DIR}/freespace2/resources/app_icon.png" DESTINATION "."
        COMPONENT "Freespace2")
