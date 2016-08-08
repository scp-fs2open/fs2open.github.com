
SET(RES_OUT_FILES)
FOREACH(file ${file_root_res_pngs})
    set(INPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    SET(OUTPUT "${GENERATED_SOURCE_DIR}/wxfred2/${file}")

    # For some reason this is needed...
    GET_FILENAME_COMPONENT(DIRECTORY_PATH ${OUTPUT} PATH)
    FILE(MAKE_DIRECTORY ${DIRECTORY_PATH})

    get_filename_component(FILENAME ${file} NAME)

    string(MAKE_C_IDENTIFIER "${FILENAME}" FIELD_NAME)

    set(HEADER_FILE "${OUTPUT}.h")
    set(SOURCE_FILE "${OUTPUT}.cpp")

    set(ALL_OUTPUTS "${HEADER_FILE}" "${SOURCE_FILE}")
    set(FIELD_NAME "${FIELD_NAME}")

    ADD_CUSTOM_COMMAND(
            OUTPUT ${ALL_OUTPUTS}
            COMMAND embedfile -wx "${INPUT_NAME}" "${OUTPUT}" "${FIELD_NAME}"
            DEPENDS ${INPUT_NAME}
            COMMENT "Generating string file for ${INPUT_NAME}"
    )

    LIST(APPEND RES_OUT_FILES ${ALL_OUTPUTS})
ENDFOREACH(file ${file_root_res_pngs})

target_sources(wxfred2 PRIVATE ${RES_OUT_FILES})
SOURCE_GROUP("Generated Files\\PNG Files" FILES ${RES_OUT_FILES})
