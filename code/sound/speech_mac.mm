#ifdef FS2_SPEECH

#import <AppKit/AppKit.h>
#import <AppKit/NSSpeechSynthesizer.h>

#include "globalincs/pstypes.h"
#include "utils/unicode.h"
#include "speech.h"

static NSSpeechSynthesizer *synth = nil;
static bool Speech_init = false;
static int voice_default_rate = 200;

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

bool speech_play(const SCP_string& text)
{
	if ( !Speech_init ) {
		return false;
	}

	if (text.empty()) {
		nprintf(("Speech", "Not playing speech because passed text is empty.\n"));
		return false;
	}

	[synth startSpeakingString:
		[NSString stringWithUTF8String:
			text.c_str()
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

	// reset voice to defaults
	[synth setObject:nil forProperty:NSSpeechResetProperty error:nil];

	// get default rate for voice
	NSNumber *voiceRate = [synth objectForProperty:NSSpeechRateProperty error:nil];
	voice_default_rate = voiceRate ? [voiceRate intValue] : 200; // median normal rate as default

	return true;
}

bool speech_set_rate(float rate_percent)
{
    if (!Speech_init) {
        return false;
    }

	CAP(rate_percent, 25.0f, 300.f);

	int rate = fl2i(voice_default_rate * (rate_percent / 100.0f));

	[synth
		setObject:[NSNumber numberWithInt:rate]
		forProperty:NSSpeechRateProperty error:nil
	];

    return true;
}

bool speech_is_speaking()
{
	if ( !Speech_init ) {
		return false;
	}

	return [synth isSpeaking];
}

SCP_vector<std::pair<int, SCP_string>> speech_enumerate_voices()
{
	NSArray *voices = [NSSpeechSynthesizer availableVoices];

	SCP_vector<std::pair<int, SCP_string>> fsoVoices;

	int voiceID = 0;
	for (NSString *voiceIdentifier in voices) {
		NSDictionary *attributes = [NSSpeechSynthesizer attributesForVoice:voiceIdentifier];
		NSString *name = [attributes objectForKey:NSVoiceName];
		fsoVoices.emplace_back(std::make_pair(voiceID++, [name UTF8String]));
	}

	return fsoVoices;
}

#endif