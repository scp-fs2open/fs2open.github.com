
SET(VOICE_REC_FILES)
IF(FSO_USE_VOICEREC)
	ADD_DEFINITIONS(-DFS2_VOICER)
	
	SET(SOUND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sound")
	SET(VOICE_REC_FILES
		"${GENERATED_SOURCE_DIR}/code/sound/grammar.h"
		"${GENERATED_SOURCE_DIR}/code/sound/phrases.cfg")
	
	ADD_CUSTOM_COMMAND(
		OUTPUT ${VOICE_REC_FILES}
		COMMAND "${SOUND_DIR}/gc.exe" /o "${GENERATED_SOURCE_DIR}/code/sound/phrases.cfg" /h "${GENERATED_SOURCE_DIR}/code/sound/grammar.h" "${SOUND_DIR}/phrases.xml"
		DEPENDS "${SOUND_DIR}/phrases.xml"
		COMMENT "Compiling voice recognition files"
		)

	SOURCE_GROUP("Generated Files\\Voice Recognition files" FILES ${VOICE_REC_FILES})
ENDIF(FSO_USE_VOICEREC)
