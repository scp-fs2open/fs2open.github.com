
MESSAGE(STATUS "Doing configuration specific to Apple Clang...")

# TODO: Actually add Mac specific flags here...

# Suppress specific warning
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fsigned-char -Wno-unknown-pragmas")

# Omit "conversion from string literal to 'char *' is deprecated" warnings.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-char-subscripts")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")

set(CMAKE_CXX_FLAGS_RELEASE "-Wno-unused-variable")

set(CMAKE_CXX_FLAGS_DEBUG "-Wextra -Wshadow -Wno-unused-parameter")
