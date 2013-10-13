#ifndef _FRMFRED2_H
#define _FRMFRED2_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

/*
 * Child Window closing procedure:
 *  frmFRED2 contains a number of child windows that can edit the mission in real time (vs. changing some settings, and
 * then clicking OK). These windows have a special way of being dismissed because of this, and can be quite confusing
 * to understand. Here is a checklist of what happens when a child window closes:
 *
 *  1. The child window is closed, and its OnClose() handler is called.
 *  2. The child window's OnClose() handler sends a message via OnChildClosed() to its parent window (frmFRED2) to say
 *     that it is closing.
 *  3. The parent window (frmFRED2) nullifies its internal pointer to the child.
 *  4. The child window's destructor is called.
 *
 *  The reason for this madness is so that we can select the child window if it is already created, instead of creating
 * a new child window every time. Future versions of wxFRED may have the ability to have multiple instances of an
 * editor type (like 2 ships editors working on two different ships), in which case this housekeeping method should
 * age well.
 */

// Child windows and dialogs:
#include "editors/frmshipseditor.h"
#include "editors/frmwingeditor.h"
#include "editors/dlgobjecteditor.h"
#include "editors/frmwaypointeditor.h"
#include "editors/dlgmissionobjectiveseditor.h"
#include "editors/dlgeventseditor.h"
#include "editors/frmteamloadouteditor.h"
#include "editors/dlgbackgroundeditor.h"
#include "editors/dlgreinforcementseditor.h"
#include "editors/dlgasteroidfieldeditor.h"
#include "editors/dlgmissionspecseditor.h"
#include "editors/frmbriefingeditor.h"
#include "editors/frmdebriefingeditor.h"
#include "editors/frmcommandbriefingeditor.h"
#include "editors/dlgfictionviewer.h"
#include "editors/dlgshieldsystemeditor.h"
#include "editors/dlgsetglobalshipflagseditor.h"
#include "editors/dlgvoiceactingmanager.h"
#include "editors/frmcampaigneditor.h"

#include "misc/dlgmissionstats.h"

#include "help/dlgaboutbox.h"
#include "help/dlgsexphelp.h"

// RAD tool generated includes:
#include "base/wxfred_base.h"

// Libs
#include <wx/wx.h>


class frmFRED2 : public fredBase::frmFRED
{
public:
	frmFRED2( const wxChar *title, int xpos, int ypos, int width, int height );
	~frmFRED2( void );

	// member variables
	//wxFREDMission* the_Mission;

protected:
	frmFRED2( void );
	frmFRED2( const frmFRED2 &otherFRED );

	// Helpers.
	void SetFredTitle( void );

	// Handlers for frmFRED events.
	//  The event table for frmFRED events (the base class) is tucked away in wxFRED_base and shouldn't be manually
	// modified.
		// Checks to see if the user has made any modifications since the last save, and prompts them if they would like to save their changes before closing.Called by File->Exit and the Close button.
	void OnClose( wxCloseEvent &event );
	void OnSize( wxSizeEvent &event );

	void OnFileNew( wxCommandEvent &event );
	void OnFileOpen( wxCommandEvent &event );
	void OnFileSave( wxCommandEvent &event );
	void OnFileSaveAs( wxCommandEvent &event );
	void OnFileRevert( wxCommandEvent &event );
	void OnFileSaveFormatFs2Open( wxCommandEvent &event );
	void OnFileSaveFormatFs2Retail( wxCommandEvent &event );
	void OnFileImportFs1Mission( wxCommandEvent &event );
	void OnFileImportFs1WeaponLoadouts( wxCommandEvent &event );
	void OnFileRunFs2( wxCommandEvent &event );
	void OnFileRecentFiles( wxCommandEvent &event );
	void OnFileExit( wxCommandEvent &event );
	void OnEditUndo( wxCommandEvent &event );
	void OnEditDelete( wxCommandEvent &event );
	void OnEditDeleteWing( wxCommandEvent &event );
	void OnEditDisableUndo( wxCommandEvent &event );

	void OnViewToolbar( wxCommandEvent &event );
	void OnViewStatusbar( wxCommandEvent &event );
	void OnViewDisplayFilterShowShips( wxCommandEvent &event );
	void OnViewDisplayFilterShowPlayerStarts( wxCommandEvent &event );
	void OnViewDisplayFilterShowWaypoints( wxCommandEvent &event );
	void OnViewDisplayFilterShowFriendly( wxCommandEvent &event );
	void OnViewDisplayFilterShowHostile( wxCommandEvent &event );
	void OnViewHideMarkedObjects( wxCommandEvent &event );
	void OnViewShowHiddenObjects( wxCommandEvent &event );
	void OnViewShowShipModels( wxCommandEvent &event );
	void OnViewShowOutlines( wxCommandEvent &event );
	void OnViewShowShipInfo( wxCommandEvent &event );
	void OnViewShowCoordinates( wxCommandEvent &event );
	void OnViewShowGridPositions( wxCommandEvent &event );
	void OnViewShowDistances( wxCommandEvent &event );
	void OnViewShowModelPaths( wxCommandEvent &event );
	void OnViewShowModelDockPoints( wxCommandEvent &event );
	void OnViewShowGrid( wxCommandEvent &event );
	void OnViewShowHorizon( wxCommandEvent &event );
	void OnViewDoubleFineGridlines( wxCommandEvent &event );
	void OnViewAntiAliasedGridlines( wxCommandEvent &event );
	void OnViewShow3DCompass( wxCommandEvent &event );
	void OnViewShowBackground( wxCommandEvent &event );
	void OnViewViewpointCamera( wxCommandEvent &event );
	void OnViewViewpointCurrentShip( wxCommandEvent &event );
	void OnViewSaveCameraPos( wxCommandEvent &event );
	void OnViewRestoreCameraPos( wxCommandEvent &event );
	void OnViewLightingFromSuns( wxCommandEvent &event );
	
	void OnSpeedMovement( wxCommandEvent &event );
	void OnSpeedRotation( wxCommandEvent &event );
	
	void OnEditorsShips( wxCommandEvent &event );
	void OnEditorsWings( wxCommandEvent &event );
	void OnEditorsObjects( wxCommandEvent &event );
	void OnEditorsWaypointPaths( wxCommandEvent &event );
	void OnEditorsMissionObjectives( wxCommandEvent &event );
	void OnEditorsEvents( wxCommandEvent &event );
	void OnEditorsTeamLoadout( wxCommandEvent &event );
	void OnEditorsBackground( wxCommandEvent &event );
	void OnEditorsReinforcements( wxCommandEvent &event );
	void OnEditorsAsteroidField( wxCommandEvent &event );
	void OnEditorsMissionSpecs( wxCommandEvent &event );
	void OnEditorsBriefing( wxCommandEvent &event );
	void OnEditorsDebriefing( wxCommandEvent &event );
	void OnEditorsFictionViewer( wxCommandEvent &event );
	void OnEditorsShieldSystem( wxCommandEvent &event );
	void OnEditorsCommandBriefing( wxCommandEvent &event );
	void OnEditorsSetGlobalShipFlags( wxCommandEvent &event );
	void OnEditorsVoiceActingManager( wxCommandEvent &event );
	void OnEditorsCampaign( wxCommandEvent &event );
	
	void OnGroupsGroup( wxCommandEvent &event );
	void OnGroupsSetGroup( wxCommandEvent &event );
	
	void OnMiscLevelObject( wxCommandEvent &event );
	void OnMiscAlignObject( wxCommandEvent &event );
	void OnMiscMarkWing( wxCommandEvent &event );
	void OnMiscControlObject( wxCommandEvent &event );
	void OnMiscNextObject( wxCommandEvent &event );
	void OnMiscPreviousObject( wxCommandEvent &event );
	void OnMiscAdjustGrid( wxCommandEvent &event );
	void OnMiscNextSubsystem( wxCommandEvent &event );
	void OnMiscPrevSubsystem( wxCommandEvent &event );
	void OnMiscCancelSubsystem( wxCommandEvent &event );
	void OnMiscMissionStatistics( wxCommandEvent &event );
	void OnMiscErrorChecker( wxCommandEvent &event );

	void OnHelpHelpTopics( wxCommandEvent &event );
	void OnHelpAbout( wxCommandEvent &event );
	void OnHelpShowSexpHelp( wxCommandEvent &event );

	// Handlers for child dialogs & frames
	bool ChildIsOpen( const wxWindowID child_id );
	void OnChildClosed( wxWindow *child );

	// Child windows and dialogs
	friend class frmShipsEditor;
	friend class frmWingEditor;
	friend class dlgObjectEditor;
	friend class frmWaypointEditor;
	friend class dlgMissionObjectivesEditor;
	friend class dlgEventsEditor;
	friend class frmTeamLoadoutEditor;
	friend class dlgBackgroundEditor;
	friend class dlgReinforcementsEditor;
	friend class dlgAsteroidFieldEditor;
	friend class dlgMissionSpecsEditor;
	friend class frmBriefingEditor;
	friend class frmDebriefingEditor;
	friend class frmCommandBriefingEditor;
	friend class dlgFictionViewer;
	friend class dlgShieldSystemEditor;
	friend class dlgSetGlobalShipFlagsEditor;
	friend class dlgVoiceActingManager;
	friend class frmCampaignEditor;
	friend class dlgMissionStats;
	friend class dlgAboutBox;
	friend class dlgSexpHelp;

	enum FREDWindowID
	{
		// Tools
		ID_TOOL_NEWOBJECTLIST = 2000,

		// Editors
		ID_frmShipsEditor,
		ID_frmWingEditor,
		ID_dlgObjectEditor,
		ID_frmWaypointEditor,
		ID_dlgMissionObjectivesEditor,
		ID_dlgEventsEditor,
		ID_frmTeamLoadoutEditor,
		ID_dlgBackgroundEditor,
		ID_dlgReinforcementsEditor,
		ID_dlgAsteroidFieldEditor,
		ID_dlgMissionSpecsEditor,
		ID_frmBriefingEditor,
		ID_frmDebriefingEditor,
		ID_frmCommandBriefingEditor,
		ID_dlgFictionViewer,
		ID_dlgShieldSystemEditor,
		ID_dlgSetGlobalShipFlagsEditor,
		ID_dlgVoiceActingManager,
		ID_frmCampaignEditor,

		// Misc.
		ID_dlgMissionStats,

		// Help frames and dialogs
		ID_dlgAboutBox,
		ID_dlgSexpHelp
	};

private:
//	void InitMods();
	// Widgets
	wxChoice* cbNewObject;
	wxStatusBar* sbFRED;

	// Frames and Dialogs
	frmShipsEditor* frmShipsEditor_p;
	frmWingEditor* frmWingEditor_p;
	dlgObjectEditor* dlgObjectEditor_p;
	frmWaypointEditor* frmWaypointEditor_p;
	dlgMissionObjectivesEditor* dlgMissionObjectivesEditor_p;
	dlgEventsEditor* dlgEventsEditor_p;
	frmTeamLoadoutEditor* frmTeamLoadoutEditor_p;
	dlgBackgroundEditor* dlgBackgroundEditor_p;
	dlgReinforcementsEditor* dlgReinforcementsEditor_p;
	dlgAsteroidFieldEditor* dlgAsteroidFieldEditor_p;
	dlgMissionSpecsEditor* dlgMissionSpecsEditor_p;
	frmBriefingEditor* frmBriefingEditor_p;
	frmDebriefingEditor* frmDebriefingEditor_p;
	frmCommandBriefingEditor* frmCommandBriefingEditor_p;
	dlgFictionViewer* dlgFictionViewer_p;
	dlgShieldSystemEditor* dlgShieldSystemEditor_p;
	dlgSetGlobalShipFlagsEditor* dlgSetGlobalShipFlagsEditor_p;
	dlgVoiceActingManager* dlgVoiceActingManager_p;
	frmCampaignEditor* frmCampaignEditor_p;
	dlgMissionStats* dlgMissionStats_p;
	dlgAboutBox* dlgAboutBox_p;
	dlgSexpHelp* dlgSexpHelp_p;

	// Viewports and Rendering
//	wxGLContext mission_state;
//	wxGLCanvas viewport;

	// member variables
	wxChar version[32];
	wxString currentFilename;
	const wxString fredName;
};
#endif