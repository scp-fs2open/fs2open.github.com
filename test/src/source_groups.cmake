
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

add_file_folder(cfile "CFile"
    cfile/cfile.cpp
)

add_file_folder(graphics "Globalincs"
    globalincs/test_flagset.cpp
    globalincs/test_safe_strings.cpp
    globalincs/test_version.cpp
)

add_file_folder(graphics "Graphics"
	   graphics/test_font.cpp
)

add_file_folder(menuui "menuui"
    menuui/test_intel_parse.cpp
)

add_file_folder(mod "mod"
    mod/test_mod_table.cpp
)

add_file_folder(graphics "Parse"
    parse/test_parselo.cpp
)

add_file_folder(scripting "Scripting"
    scripting/ade_args.cpp
    scripting/ScriptingTestFixture.h
    scripting/ScriptingTestFixture.cpp
)

add_file_folder(scripting_api "Scripting\\\\API"
    scripting/api/base.cpp
    scripting/api/bitops.cpp
    scripting/api/enums.cpp
)

add_file_folder(scripting_api "Scripting\\\\Lua"
    scripting/lua/Args.cpp
    scripting/lua/Convert.cpp
    scripting/lua/Function.cpp
    scripting/lua/Reference.cpp
    scripting/lua/Table.cpp
    scripting/lua/TestUtil.h
    scripting/lua/Util.cpp
    scripting/lua/Value.cpp
)

add_file_folder(util "Test Util"
    util/FSTestFixture.cpp
    util/FSTestFixture.h
    util/test_util.h
)

add_file_folder(utils "Utils"
    utils/HeapAllocatorTest.cpp
)

add_file_folder(weapon "Weapon"
    weapon/weapons.cpp
)
