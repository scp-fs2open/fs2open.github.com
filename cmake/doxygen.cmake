
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	SET(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/documentation/doxygen")
	FILE(MAKE_DIRECTORY "${OUTPUT_DIR}")
	
	FILE(TO_NATIVE_PATH ${OUTPUT_DIR} OUTPUT_DIR)
	SET(OUTPUT_DIR \"${OUTPUT_DIR}\")
	
	SET(INPUTS "${CMAKE_CURRENT_SOURCE_DIR}/code" "${CMAKE_CURRENT_SOURCE_DIR}/freespace2" "${CMAKE_CURRENT_SOURCE_DIR}/fred2" "${CMAKE_CURRENT_SOURCE_DIR}/wxfred2" "${CMAKE_CURRENT_SOURCE_DIR}/qtfred")
	
	SET(INPUT_DIRS)
	FOREACH(DIR IN LISTS INPUTS)
		FILE(TO_NATIVE_PATH ${DIR} native_dir)
		
		LIST(APPEND INPUT_DIRS \"${native_dir}\")
	ENDFOREACH(DIR)
	STRING(REPLACE ";" " " INPUT_DIRS "${INPUT_DIRS}")
	
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/fs2open.Doxyfile.in
					${CMAKE_CURRENT_BINARY_DIR}/fs2open.Doxyfile
					@ONLY)
	
	add_custom_target(doxygen
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/fs2open.Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
	
	set_target_properties(doxygen
		PROPERTIES
			FOLDER "Documentation"
			EXCLUDE_FROM_ALL ON
			EXCLUDE_FROM_DEFAULT_BUILD ON
	)
endif(DOXYGEN_FOUND)
