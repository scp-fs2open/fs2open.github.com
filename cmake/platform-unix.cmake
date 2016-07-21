
INCLUDE(util)

MESSAGE(STATUS "Configuring UNIX specific things and stuff...")

target_compile_definitions(platform INTERFACE SCP_UNIX USE_OPENAL)
