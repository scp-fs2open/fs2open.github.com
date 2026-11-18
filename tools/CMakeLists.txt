# These are used during the build so we always need to add them
ADD_SUBDIRECTORY(embedfile)
ADD_SUBDIRECTORY(fieldscan)

# Now add the optional tools
if (FSO_BUILD_TOOLS)
    ADD_SUBDIRECTORY(strings_tool)
endif ()
