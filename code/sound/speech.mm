#ifdef FS2_SPEECH

#import <AppKit/AppKit.h>
#import <AppKit/NSSpeechSynthesizer.h>

#include "globalincs/pstypes.h"
#include "utils/unicode.h"


static NSSpeechSynthesizer *synth = nil;
static bool Speech_init = false;


bool speech_init()
{
	if (Speech_init) {
		return true;
	}

	synth = [[NSSpeechSynthesizer alloc] init];

	Speech_init = true;

	return true;
}

void speech_deinit()
{
	if ( !Speech_init ) {
		return;
	}

	[synth release];
	synth = nil;

	Speech_init = false;
}

bool speech_play(const char *text)
{
	if ( !Speech_init ) {
		return false;
	}

	if ( !text || !strlen(text) ) {
		nprintf(("Speech", "Not playing speech because passed text is null.\n"));
		return false;
	}

	SCP_string work_buffer;

	bool saw_dollar = false;
	for (auto ch : unicode::codepoint_range(text)) {
		if (ch == UNICODE_CHAR('$')) {
			// Skip $ escape sequences which appear in briefing text
			saw_dollar = true;
			continue;
		} else if (saw_dollar) {
			saw_dollar = false;
			continue;
		}

		unicode::encode(ch, std::back_inserter(work_buffer));
	}

	if (work_buffer.empty()) {
		return false;
	}

	[synth startSpeakingString:
		[NSString stringWithUTF8String:
			work_buffer.c_str()
		]
	];

	return true;
}

bool speech_pause()
{
	if ( !Speech_init ) {
		return false;
	}

	[synth pauseSpeakingAtBoundary:NSSpeechImmediateBoundary];

	return true;
}

bool speech_resume()
{
	if ( !Speech_init ) {
		return false;
	}

	[synth continueSpeaking];

	return true;
}

bool speech_stop()
{
	if ( !Speech_init ) {
		return false;
	}

	[synth stopSpeaking];

	return true;
}

bool speech_set_volume(unsigned short volume)
{
	if ( !Speech_init ) {
		return false;
	}

	float vol = volume * 0.01f;

	[synth
		setObject: [NSNumber numberWithFloat:vol]
		forProperty:NSSpeechVolumeProperty error:nil
	];

	return true;
}

bool speech_set_voice(int voice)
{
	if ( !Speech_init ) {
		return false;
	}

	NSArray *voices = [NSSpeechSynthesizer availableVoices];

	if ( (voice < 0) || (voice >= [voices count]) ) {
		nprintf(("Speech", "Attempting to set voice to invalid value (%d)\n", voice));
		return false;
	}

	[synth setVoice: [voices objectAtIndex:voice]];

	return true;
}

bool speech_is_speaking()
{
	if ( !Speech_init ) {
		return false;
	}

	return [synth isSpeaking];
}

SCP_vector<SCP_string> speech_enumerate_voices()
{
	NSArray *voices = [NSSpeechSynthesizer availableVoices];

	SCP_vector<SCP_string> fsoVoices;

	for (NSString *voiceIdentifier in voices) {
		NSDictionary *attributes = [NSSpeechSynthesizer attributesForVoice:voiceIdentifier];
		NSString *name = [attributes objectForKey:NSVoiceName];

		fsoVoices.push_back([name UTF8String]);
	}

	return fsoVoices;
}

#endif