
if(WIN32)
    # Handling of windows resources+

    set(subpath resources/win)

    set(RESOURCE_FILES
        ${subpath}/freespace.rc
        ${subpath}/version.rc2
    )

    set(ICONS
        ${subpath}/app_icon.ico
        ${subpath}/app_icon_glow.ico
        ${subpath}/dbg_icon.ico
        ${subpath}/goal_com.bmp
        ${subpath}/goal_fail.bmp
        ${subpath}/goal_inc.bmp
        ${subpath}/goal_none.bmp
        ${subpath}/goal_ord.bmp
        ${subpath}/V_app.ico
        ${subpath}/V_debug.ico
        ${subpath}/V_sse-d.ico
        ${subpath}/V_sse.ico
    )

    set(RESOURCES
        ${RESOURCE_FILES}
        ${ICONS}
    )

    target_sources(Freespace2 PRIVATE ${RESOURCES})

    source_group("Resources" FILES ${RESOURCE_FILES})
    source_group("Resources\\Icons" FILES ${ICONS})

    SET_SOURCE_FILES_PROPERTIES(${subpath}/freespace.rc PROPERTIES COMPILE_DEFINITIONS "_VC08")

    IF(FSO_INSTRUCTION_SET STREQUAL "SSE2" OR FSO_INSTRUCTION_SET STREQUAL "AVX")
    	set_property(SOURCE ${subpath}/freespace.rc APPEND_STRING PROPERTY COMPILE_DEFINITIONS ";_SSE2")
    ENDIF()

elseif(APPLE)
    # Handling of apple resources
    set(subpath resources/mac)

    set_target_properties(Freespace2 PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/${subpath}/Info.plist)

    # Copy everything from the Resources directory
    add_custom_command(TARGET Freespace2 POST_BUILD
        COMMAND cp -a "${CMAKE_CURRENT_SOURCE_DIR}/${subpath}/Resources" "$<TARGET_FILE_DIR:Freespace2>/../Resources"
        COMMENT "Copying resources into bundle..."
    )

    #configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${subpath}/fixup_bundle.cmake.in"
    #    "${CMAKE_CURRENT_BINARY_DIR}/fixup_bundle.cmake"
    #   @ONLY)

    #file(GENERATE
    #    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/fixup_bundle-$<CONFIG>.cmake"
    #    INPUT "${CMAKE_CURRENT_BINARY_DIR}/fixup_bundle.cmake")

    add_custom_command(TARGET Freespace2 POST_BUILD
		COMMAND cp -a "${FSO_MAC_FRAMEWORKS}" "$<TARGET_FILE_DIR:Freespace2>/../Frameworks"
        #COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/fixup_bundle-$<CONFIG>.cmake"
        COMMENT "Copying frameworks into bundle..."
    )
else()
    # No special resource handling required, add rules for new platforms here
endif()
