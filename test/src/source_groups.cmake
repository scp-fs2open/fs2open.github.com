
set(source_files)

macro(add_file_folder VAR_NAME FOLDER_NAME)
    set(file_${VAR_NAME} ${ARGN})
    source_group("${FOLDER_NAME}" FILES ${file_${VAR_NAME}})
    set(source_files ${source_files} ${file_${VAR_NAME}})
endmacro(add_file_folder)

add_file_folder(root ""
    main.cpp
    test_stubs.cpp
)

add_file_folder(graphics "Globalincs"
    globalincs/test_flagset.cpp
)

add_file_folder(graphics "Graphics"
	   graphics/test_font.cpp
)

add_file_folder(menuui "menuui"
    menuui/test_intel_parse.cpp
)

add_file_folder(graphics "Parse"
    parse/test_parselo.cpp
)

add_file_folder(scripting "Scripting"
    scripting/ScriptingTestFixture.h
    scripting/ScriptingTestFixture.cpp
)

add_file_folder(scripting_api "Scripting\\\\API"
    scripting/api/bitops.cpp
)

add_file_folder(util "Util"
    util/FSTestFixture.cpp
    util/FSTestFixture.h
)
