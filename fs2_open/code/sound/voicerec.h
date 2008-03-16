#ifndef __VOICEREC_H__
#define __VOICEREC_H__

#define GRAMMARID1      161                     // Arbitrary grammar id
#define WM_RECOEVENT    WM_USER+190             // Arbitrary user defined message for reco callback

bool VOICEREC_init(HWND hWnd, int event_id, int grammar_id, int command_resource);
void VOICEREC_deinit();
void VOICEREC_process_event(HWND hWnd);

#endif
