
IF(MSVC)
	# Windows specific version using resource files

	set(RES_CONTENT)
	set(ARRAY_ELEMENTS)

	FOREACH(file ${file_root_def_files_files})
		set(INPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
		SET(OUTPUT "${GENERATED_SOURCE_DIR}/code/${file}")
		file(RELATIVE_PATH RELATIVE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/def_files" "${INPUT_NAME}")


		# Retrieve the path type name from the relative path
		GET_FILENAME_COMPONENT(PATH_TYPE "${RELATIVE_FILE_PATH}" DIRECTORY)
		file(TO_NATIVE_PATH "${PATH_TYPE}" PATH_TYPE)
		string(REPLACE "\\" "\\\\" PATH_TYPE "${PATH_TYPE}")
		GET_FILENAME_COMPONENT(FILE_NAME "${RELATIVE_FILE_PATH}" NAME)

		string(MAKE_C_IDENTIFIER "${RELATIVE_FILE_PATH}" FIELD_NAME)

		set(FIELD_NAME "Default_${FIELD_NAME}")
		string(TOUPPER "${FIELD_NAME}" FIELD_NAME)

		set(ARRAY_ELEMENTS "${ARRAY_ELEMENTS}\n\t{ \"${PATH_TYPE}\", \"${FILE_NAME}\" , TEXT(\"${FIELD_NAME}\") },")

		set(RES_CONTENT "${RES_CONTENT}\n${FIELD_NAME} RCDATA \"${INPUT_NAME}\"")
	ENDFOREACH(file)

	file(WRITE "${GENERATED_SOURCE_DIR}/code/default_files.rc" "${RES_CONTENT}")

	configure_file("def_files/generated_def_files-win32.h.in" "${GENERATED_SOURCE_DIR}/code/def_files/generated_def_files-win32.h")

	target_sources(code INTERFACE "${GENERATED_SOURCE_DIR}/code/default_files.rc")
else()
	# Generic version using embedfile
	SET(DEF_OUT_FILES)

	set(INCLUDE_LIST)
	set(ARRAY_ELEMENTS)

	FOREACH(file ${file_root_def_files_files})
		set(INPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
		SET(OUTPUT "${GENERATED_SOURCE_DIR}/code/${file}")
		file(RELATIVE_PATH RELATIVE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/def_files" "${INPUT_NAME}")

		# For some reason this is needed...
		GET_FILENAME_COMPONENT(DIRECTORY_PATH ${OUTPUT} DIRECTORY)
		FILE(MAKE_DIRECTORY ${DIRECTORY_PATH})

		# Retrieve the path type name from the relative path
		GET_FILENAME_COMPONENT(PATH_TYPE "${RELATIVE_FILE_PATH}" DIRECTORY)
		file(TO_NATIVE_PATH "${PATH_TYPE}" PATH_TYPE)
		GET_FILENAME_COMPONENT(FILE_NAME "${RELATIVE_FILE_PATH}" NAME)

		string(MAKE_C_IDENTIFIER "${RELATIVE_FILE_PATH}" FIELD_NAME)

		set(HEADER_FILE "${OUTPUT}.h")
		set(SOURCE_FILE "${OUTPUT}.cpp")

		set_source_files_properties("${HEADER_FILE}" "${SOURCE_FILE}" PROPERTIES GENERATED TRUE)

		set(ALL_OUTPUTS "${HEADER_FILE}" "${SOURCE_FILE}")
		set(FIELD_NAME "Default_${FIELD_NAME}")

		set(INCLUDE_LIST "${INCLUDE_LIST}\n#include \"${file}.h\"")
		set(ARRAY_ELEMENTS "${ARRAY_ELEMENTS}\n\t{ \"${PATH_TYPE}\", \"${FILE_NAME}\" , ${FIELD_NAME} , ${FIELD_NAME}_size },")

		ADD_CUSTOM_COMMAND(
			OUTPUT ${ALL_OUTPUTS}
			COMMAND embedfile "${INPUT_NAME}" "${OUTPUT}" "${FIELD_NAME}"
			DEPENDS "${INPUT_NAME}"
			COMMENT "Generating string file for ${INPUT_NAME}"
			)

		LIST(APPEND DEF_OUT_FILES ${ALL_OUTPUTS})
	ENDFOREACH(file)

	configure_file("def_files/generated_def_files-generic.h.in" "${GENERATED_SOURCE_DIR}/code/def_files/generated_def_files-generic.h")
	target_sources(code PRIVATE ${DEF_OUT_FILES})
	SOURCE_GROUP("Generated Files\\Default Files" FILES ${DEF_OUT_FILES})

endif()
