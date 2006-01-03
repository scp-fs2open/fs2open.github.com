/*
 * Code created by Thomas Whittaker (Random Tiger) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#ifdef _WIN32
#ifndef FS2_VOICER

#if NDEBUG
	#pragma message( "WARNING: You have not voice recognition into this build (use FS2_VOICER)" )
#endif // NDEBUG

#else // to end-of-file ...

#ifdef LAUNCHER
#include "stdafx.h"
#endif	//LAUNCHER



#include <sphelper.h>                           // Contains definitions of SAPI functions
#include <stdio.h>

#include "voicerec.h"
#include "grammar.h"

// Freespace includes
#include "hud\hudsquadmsg.h"
#include "io\keycontrol.h"
#include "playerman/player.h"

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

/******************************************************************************
* ExecuteCommand *
*----------------*
*   Description:
*       Called to Execute commands that have been identified by the speech engine.
*
******************************************************************************/
extern int button_function(int n);

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

				// Goto Action menu
			//	hud_squadmsg_do_mode( SM_MODE_SHIP_COMMAND );
				break;
			}
			case VID_WingName: 
			{
				int WingType = pElements->pProperties->vValue.ulVal;

		//		hud_squadmsg_do_mode( SM_MODE_SHIP_COMMAND );

				break;
			}

			case VID_Action:
			{
				int action = pElements->pProperties->vValue.ulVal;

				switch(action)
				{
					
					case VID_DestoryTarget: 				
						button_function(ATTACK_MESSAGE); 
						break;
					case VID_DisableTarget:
						button_function(DISABLE_MESSAGE); 
						break;
					case VID_DisarmTarget:
						button_function(DISARM_MESSAGE); 
						break;

					case VID_DestroySubsys:
						button_function(ATTACK_SUBSYSTEM_MESSAGE); 
						break;

					case VID_ProtectTarget:
						button_function(PROTECT_MESSAGE); 
						break;

					case VID_IgnoreTarget:
						button_function(IGNORE_MESSAGE); 
						break;

					case VID_FormWing:
						button_function(FORM_MESSAGE); 
						break;

					case VID_CoverMe:
						button_function(COVER_MESSAGE); 
						break;

					case VID_EngageEnemy:
						button_function(ENGAGE_MESSAGE); 
						break;

					case VID_Depart:
						button_function(WARP_MESSAGE); 
						break;
					break;
				}
				break;
			}

			case VID_TopMenu:
			{
				int action = pElements->pProperties->vValue.ulVal;

				// If the command window is not up, or it is and its a cancel request toggle
				if((Player->flags & PLAYER_FLAGS_MSG_MODE && action == VID_Cancel) ||
					(!(Player->flags & PLAYER_FLAGS_MSG_MODE) && action != VID_Cancel))
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
						hud_squadmsg_do_mode( SM_MODE_ALL_FIGHTERS ); 
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