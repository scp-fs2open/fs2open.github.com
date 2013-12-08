/*
 * Code created by Thomas Whittaker (Random Tiger) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#ifdef _WIN32
#ifndef FS2_VOICER

#if NDEBUG
	#pragma message( "WARNING: You have not compiled voice recognition into this build (use FS2_VOICER)" )
#endif // NDEBUG

#else // to end-of-file ...

#ifdef LAUNCHER
#include "stdafx.h"
#endif	//LAUNCHER



#include <sphelper.h>                           // Contains definitions of SAPI functions
#include <stdio.h>

#include "voicerec.h"
#include "grammar.h"

// FreeSpace includes
#include "hud/hudsquadmsg.h"
#include "io/keycontrol.h"
#include "playerman/player.h"
#include "ship/ship.h"

CComPtr<ISpRecoGrammar>         p_grammarObject; // Pointer to our grammar object
CComPtr<ISpRecoContext>         p_recogContext;  // Pointer to our recognition context
CComPtr<ISpRecognizer>			p_recogEngine;   // Pointer to our recognition engine instance

const bool DEBUG_ON = false;

bool VOICEREC_init(HWND hWnd, int event_id, int grammar_id, int command_resource)
{
    HRESULT hr = S_OK;
    CComPtr<ISpAudio> cpAudio;

    while ( 1 )
    {
        // create a recognition engine
        hr = p_recogEngine.CoCreateInstance(CLSID_SpSharedRecognizer);
        if ( FAILED( hr ) )
        {
			MessageBox(hWnd,"Failed to create a recognition engine\n","Error",MB_OK);
			printf("Failed to create a recognition engine\n");
            break;
        }
       
        // create the command recognition context
        hr = p_recogEngine->CreateRecoContext( &p_recogContext );
        if ( FAILED( hr ) )
        {
			MessageBox(hWnd,"Failed to create the command recognition context\n","Error",MB_OK);
			printf("Failed to create the command recognition context\n");
            break;
        }

        // Let SR know that window we want it to send event information to, and using
        // what message
        hr = p_recogContext->SetNotifyWindowMessage( hWnd, event_id, 0, 0 );
        if ( FAILED( hr ) )
        {
			MessageBox(hWnd,"Failed to SetNotifyWindowMessage\n","Error",MB_OK);
            break;
        }

	    // Tell SR what types of events interest us.  Here we only care about command
        // recognition.
        hr = p_recogContext->SetInterest( SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION) );
        if ( FAILED( hr ) )
        {
			MessageBox(hWnd,"Failed to set events\n","Error",MB_OK);
            break;
        }

        // Load our grammar, which is the compiled form of simple.xml bound into this executable as a
        // user defined ("SRGRAMMAR") resource type.
        hr = p_recogContext->CreateGrammar(grammar_id, &p_grammarObject);
        if (FAILED(hr))
        {
			MessageBox(hWnd,"Failed to load grammar\n","Error",MB_OK);
            break;
        }

        hr = p_grammarObject->LoadCmdFromResource(NULL, MAKEINTRESOURCEW(command_resource),
                                                 L"SRGRAMMAR", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                                 SPLO_DYNAMIC);
        if ( FAILED( hr ) )
        {
			MessageBox(hWnd,"Failed to load resource\n","Error",MB_OK);
            break;
        }

        // Set rules to active, we are now listening for commands
        hr = p_grammarObject->SetRuleState(NULL, NULL, SPRS_ACTIVE );
        if ( FAILED( hr ) )
        {
			MessageBox(hWnd,"Failed to set listening for commands\n","Error",MB_OK);
            break;
        }

        break;
    }

    // if we failed and have a partially setup SAPI, close it all down
    if ( FAILED( hr ) )
    {
        VOICEREC_deinit();
    }

    return ( hr == S_OK);
}

void VOICEREC_deinit()
{
    // Release grammar, if loaded
    if ( p_grammarObject )
    {
        p_grammarObject.Release();
    }
    // Release recognition context, if created
    if ( p_recogContext )
    {
        p_recogContext->SetNotifySink(NULL);
        p_recogContext.Release();
    }
    // Release recognition engine instance, if created
	if ( p_recogEngine )
	{
		p_recogEngine.Release();
	}
}

void VOICEREC_execute_command(ISpPhrase *pPhrase, HWND hWnd);
void VOICEREC_process_event(HWND hWnd)
{
    CSpEvent event;  // Event helper class

    // Loop processing events while there are any in the queue
    while (event.GetFrom(p_recogContext) == S_OK)
    {
        // Look at recognition event only
        switch (event.eEventId)
        {
            case SPEI_RECOGNITION:
					VOICEREC_execute_command(event.RecoResult(), hWnd);
                break;

        }
    }
}

// Its not enough to update this, phrases.xml must have this info as well
char *wing_names[] = 
{
		"Alpha",
		"Beta",	
		"Gamma",	
		"Delta",
		"Epsilon",
		"Zeta",
		"Eta",
		"Theta",
		"Iota",
		"Kappa",
		"Lambda",
		"Mu",
		"Nu",
		"Xi",
		"Omicron",
		"Pi",
		"Rho",
		"Sigma",
		"Tau",
		"Upsilon",
		"Phi",
		"Chi",
		"Psi",
		"Omega",
};

/******************************************************************************
* ExecuteCommand *
*----------------*
*   Description:
*       Called to Execute commands that have been identified by the speech engine.
*
******************************************************************************/
extern int button_function(int n);
extern void hud_squadmsg_msg_all_fighters();
extern void hud_squadmsg_shortcut( int command );

//extern void hud_squadmsg_ship_command();
extern int Msg_instance;;
extern int Msg_shortcut_command;
extern int Squad_msg_mode;

char VOICEREC_lastCommand[30];

void VOICEREC_execute_command(ISpPhrase *pPhrase, HWND hWnd)
{
    SPPHRASE *pElements;

    // Get the phrase elements, one of which is the rule id we specified in
    // the grammar.  Switch on it to figure out which command was recognized.
    if (SUCCEEDED(pPhrase->GetPhrase(&pElements)))
    {     
		if(DEBUG_ON)
		{
			WCHAR *pwszText;
			pPhrase->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &pwszText, NULL);
			MessageBoxW(NULL,pwszText,NULL,MB_OK);	
		}
		
        switch ( pElements->Rule.ulId )
        {
			case VID_ShipName: 
			{
				int wingType = pElements->pProperties->vValue.ulVal;
				int shipNum = pElements->pProperties->pNextSibling->vValue.ulVal;

				char shipName[NAME_LENGTH];
				wing_bash_ship_name(shipName, wing_names[wingType], shipNum);

				Msg_instance = ship_name_lookup(shipName);

				// Can't issue commands to yourself or to nobody
				if(Msg_instance < 0 || Msg_instance == Player_obj->instance)
				{
					break;
				}

				if(!(Player->flags & PLAYER_FLAGS_MSG_MODE))
				{
					hud_squadmsg_toggle();
				}

				hud_squadmsg_do_mode(SM_MODE_SHIP_COMMAND);
				break;
			}
			case VID_WingName: 
			{
				int wingType  = pElements->pProperties->vValue.ulVal;

				Msg_instance  = wing_lookup(wing_names[wingType]);
				if (Msg_instance < 0)
					break;

				if(!(Player->flags & PLAYER_FLAGS_MSG_MODE))
				{
					hud_squadmsg_toggle();
				}

				hud_squadmsg_do_mode(SM_MODE_WING_COMMAND);
				break;
			}

			case VID_Action:
			{
				int action = pElements->pProperties->vValue.ulVal;					

				// If menu is up
				if(Player->flags & PLAYER_FLAGS_MSG_MODE)
				{
					switch(action)
					{					
					case VID_DestoryTarget: Msg_shortcut_command = ATTACK_TARGET_ITEM;			break;
					case VID_DisableTarget:	Msg_shortcut_command = DISABLE_TARGET_ITEM;			break;
					case VID_DisarmTarget:	Msg_shortcut_command = DISARM_TARGET_ITEM;			break;
					case VID_DestroySubsys:	Msg_shortcut_command = DISABLE_SUBSYSTEM_ITEM;		break;
					case VID_ProtectTarget:	Msg_shortcut_command = PROTECT_TARGET_ITEM;			break;
					case VID_IgnoreTarget:	Msg_shortcut_command = IGNORE_TARGET_ITEM;			break;
					case VID_FormWing:		Msg_shortcut_command = FORMATION_ITEM;				break;
					case VID_CoverMe:		Msg_shortcut_command = COVER_ME_ITEM;				break;
					case VID_EngageEnemy:	Msg_shortcut_command = ENGAGE_ENEMY_ITEM;			break;
					case VID_Depart:		Msg_shortcut_command = DEPART_ITEM;					break;
					default:				Msg_shortcut_command = -1;							break;

					}

					if(Msg_instance == MESSAGE_ALL_FIGHTERS || Squad_msg_mode == SM_MODE_ALL_FIGHTERS )
					{
					//	nprintf(("warning", "VOICER hud_squadmsg_send_to_all_fighters\n"));
						hud_squadmsg_send_to_all_fighters(Msg_shortcut_command);
					}
					else if(Squad_msg_mode == SM_MODE_SHIP_COMMAND)
					{
					//	nprintf(("warning", "VOICER msg ship %d\n", Msg_instance));
						hud_squadmsg_send_ship_command( Msg_instance, Msg_shortcut_command, 1 );
					}
					else if(Squad_msg_mode == SM_MODE_WING_COMMAND)
					{
					//	nprintf(("warning", "VOICER msg wing %d\n", Msg_instance));
						hud_squadmsg_send_wing_command( Msg_instance, Msg_shortcut_command, 1 );
					}
					else if(Squad_msg_mode == SM_MODE_REINFORCEMENTS )
					{
					}

					hud_squadmsg_toggle();
				}
				else
				{
					switch(action)
					{					
					case VID_DestoryTarget: button_function(ATTACK_MESSAGE);				break;
					case VID_DisableTarget:	button_function(DISABLE_MESSAGE);				break;
					case VID_DisarmTarget:	button_function(DISARM_MESSAGE);				break;
					case VID_DestroySubsys:	button_function(ATTACK_SUBSYSTEM_MESSAGE);		break;
					case VID_ProtectTarget:	button_function(PROTECT_MESSAGE);				break;
					case VID_IgnoreTarget:	button_function(IGNORE_MESSAGE);				break;
					case VID_FormWing:		button_function(FORM_MESSAGE);					break;
					case VID_CoverMe:		button_function(COVER_MESSAGE);					break;
					case VID_EngageEnemy:	button_function(ENGAGE_MESSAGE);				break;
					case VID_Depart:		button_function(WARP_MESSAGE);					break;
					}
				}
		
				break;
			}

			// These commands run no matter what, and will even bring up the menu
			case VID_TopMenu:
			{
				int action = pElements->pProperties->vValue.ulVal;
				bool msgWindow = false;
				if (Player->flags & PLAYER_FLAGS_MSG_MODE)
				{
					msgWindow = true;
				}

				// If the command window is not up, or it is and its a cancel request toggle
				if((msgWindow && action == VID_Cancel) || (!msgWindow && action != VID_Cancel))
				{
					hud_squadmsg_toggle();
				}
				
				switch(action)
				{
					case VID_Ships: 
						hud_squadmsg_do_mode( SM_MODE_SHIP_SELECT );
						break;

					case VID_Wings:
						hud_squadmsg_do_mode( SM_MODE_WING_SELECT );
						break;

					case VID_AllFighters: 
					case VID_AllWings:
					//	Msg_instance = MESSAGE_ALL_FIGHTERS;
					//	Squad_msg_mode == SM_MODE_ALL_FIGHTERS
						hud_squadmsg_msg_all_fighters();

					//	if(Msg_shortcut_command == -1)
					//	{
					//		hud_squadmsg_do_mode( SM_MODE_ALL_FIGHTERS ); 
					//	}
						break;

					case VID_Reinforcements: 
						hud_squadmsg_do_mode( SM_MODE_REINFORCEMENTS );
						break;

					case VID_SupportShip:
						hud_squadmsg_do_mode( SM_MODE_REPAIR_REARM );
						break;

					case VID_AbortSupport:
						hud_squadmsg_do_mode( SM_MODE_REPAIR_REARM_ABORT );
						break;

					case VID_More:
						break;
				}

				break;
			}


	
	
	/*	
		
		
	hud_squadmsg_do_mode( SM_MODE_SHIP_COMMAND );				// and move to a new mode
	hud_squadmsg_do_mode( SM_MODE_WING_COMMAND );				// and move to a new mode
hud_squadmsg_do_mode( SM_MODE_SHIP_COMMAND );

*/

        }
        // Free the pElements memory which was allocated for us
        ::CoTaskMemFree(pElements);
    }

}

#endif // VOICER
#endif // _WIN32
