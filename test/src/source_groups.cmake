
set(source_files)

add_file_folder(""
    main.cpp
    test_stubs.cpp
)

add_file_folder("Actions"
)

add_file_folder("Actions\\\\Expression"
	actions/expression/test_ExpressionParser.cpp
)

add_file_folder("CFile"
    cfile/cfile.cpp
)

add_file_folder("Globalincs"
    globalincs/test_flagset.cpp
    globalincs/test_safe_strings.cpp
    globalincs/test_version.cpp
)

add_file_folder("Graphics"
	   graphics/test_font.cpp
)

add_file_folder("Math"
    math/test_vecmat.cpp
)

add_file_folder("menuui"
    menuui/test_intel_parse.cpp
)

add_file_folder("mod"
    mod/test_mod_table.cpp
)

add_file_folder("model"
    model/test_modelread.cpp
)

add_file_folder("Parse"
    parse/test_parselo.cpp
    parse/test_replace.cpp
)

add_file_folder("Pilotfile"
    pilotfile/plr.cpp
)

add_file_folder("Scripting"
    scripting/ade_args.cpp
    scripting/doc_parser.cpp
    scripting/require.cpp
    scripting/script_state.cpp
    scripting/ScriptingTestFixture.h
    scripting/ScriptingTestFixture.cpp
)

add_file_folder("Scripting\\\\API"
    scripting/api/async.cpp
    scripting/api/base.cpp
    scripting/api/bitops.cpp
    scripting/api/enums.cpp
    scripting/api/hookvars.cpp
)

add_file_folder("Scripting\\\\Lua"
    scripting/lua/Args.cpp
    scripting/lua/Convert.cpp
    scripting/lua/Function.cpp
    scripting/lua/Reference.cpp
    scripting/lua/Table.cpp
    scripting/lua/TestUtil.h
    scripting/lua/Thread.cpp
    scripting/lua/Util.cpp
    scripting/lua/Value.cpp
)

add_file_folder("Test Util"
    util/FSTestFixture.cpp
    util/FSTestFixture.h
    util/test_util.h
)

add_file_folder("Utils"
    utils/HeapAllocatorTest.cpp
)

add_file_folder("Weapon"
    weapon/weapons.cpp
)
