
include(platformChecks)
include(Cxx11)

CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/project.h.in ${GENERATED_SOURCE_DIR}/project.h)
