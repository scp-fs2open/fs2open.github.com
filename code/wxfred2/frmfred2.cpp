/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

// Note: wxFormBuilder 3.3beta has a tendancy to use the ADJUST_MINSIZE macro (no longer existing)
//  Just do a search&replace of "|wxADJUST_MINSIZE" and "|wxTE_LINEWRAP" to "" to delete all instances, and it'll work fine


#include "frmfred2.h"
#include "res/fred_app.xpm"
#include "res/fred_debug.xpm"

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

#include "base/wxfred_base.h"

#include <globalincs/version.h>
#include <globalincs/pstypes.h>

#include <wx/msgdlg.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>

// Public:
frmFRED2::frmFRED2( const wxChar *title, int xpos, int ypos, int width, int height )
	: fredBase::frmFRED( NULL, wxID_ANY, wxEmptyString, wxPoint(xpos, ypos), wxSize(width, height) ),
		currentFilename("Untitled"), fredName("FreeSpace 2 Mission Editor"),
		frmShipsEditor_p(NULL), frmWingEditor_p(NULL), dlgObjectEditor_p(NULL),
		frmWaypointEditor_p(NULL), dlgMissionObjectivesEditor_p(NULL), dlgEventsEditor_p(NULL),
		frmTeamLoadoutEditor_p(NULL), dlgBackgroundEditor_p(NULL), dlgReinforcementsEditor_p(NULL),
		dlgAsteroidFieldEditor_p(NULL), dlgMissionSpecsEditor_p(NULL), frmBriefingEditor_p(NULL),
		frmDebriefingEditor_p(NULL), frmCommandBriefingEditor_p(NULL), dlgFictionViewer_p(NULL),
		dlgShieldSystemEditor_p(NULL), dlgSetGlobalShipFlagsEditor_p(NULL), dlgVoiceActingManager_p(NULL),
		frmCampaignEditor_p(NULL), dlgMissionStats_p(NULL), dlgAboutBox_p(NULL), dlgSexpHelp_p(NULL)
{
	// Title bar setup
	// z64: somewhat roundabout way to include the FS2 Open version number in the form title.
	
	wxSprintf( version, "v%i.%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD );
	this->SetFredTitle();

	// New Object Selection ListBox
	cbNewObject = new wxChoice( tbrFRED, ID_TOOL_NEWOBJECTLIST, wxDefaultPosition, wxSize(193,-1), 0, NULL, wxCB_SORT );
	// TODO: Auto-size the choice box to the longest string in the list.
	// TODO: Grab the actual list of object classes
	cbNewObject->Append(_T("GTF Ulysses"));
	cbNewObject->SetSelection(0);

	tbrFRED->AddControl( cbNewObject );
	tbrFRED->Realize();

	// Status Bar
	sbFRED = this->CreateStatusBar( 5, wxSTB_DEFAULT_STYLE, wxID_ANY, "wxFRED2" );

	// The Viewport
	wxBoxSizer* bSizerView = new wxBoxSizer( wxVERTICAL );
	wxGLCanvas *TheViewport = new wxGLCanvas(this, wxID_ANY, NULL, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
	bSizerView->Add( TheViewport );
	this->SetSizer( bSizerView );
	this->Layout();

	// Set the icon to use for the taskbar and titlebar of this frame
#ifdef NDEBUG
	this->SetIcon(wxIcon(fred_app_xpm));
#else
	this->SetIcon(wxIcon(fred_debug_xpm));
#endif
}

frmFRED2::~frmFRED2( void )
{
	if( sbFRED != NULL )
	{
		delete sbFRED;
	}
}

// Protected:
// Helpers.
/*
 *  Sets the string on the titlebar of frmFRED2.
 *  wxChar version[] and wxString fredName are initialized during the constructor, and shouldn't be altered elsewhere.
 */
void frmFRED2::SetFredTitle( void )
{
	this->SetTitle( currentFilename + " - " + version + " - " + fredName );
}

// Handlers for frmFRED events.
/*
 *  Checks to see if the user has made any modifications since the last save, and prompts them if they would like to
 * save their changes before closing.
 *  Called by File->Exit and the Close button.
 */
void frmFRED2::OnClose( wxCloseEvent &event )
{
// TODO: Implement OnClose
	this->Destroy();
}

void frmFRED2::OnSize( wxSizeEvent &event )
{
// TODO: Implement OnSize
}

void frmFRED2::OnFileNew( wxCommandEvent &event )
{
// TODO: Implement OnFileNew
}

void frmFRED2::OnFileOpen( wxCommandEvent &event )
{
	wxFileDialog *dlg =
		new wxFileDialog( this, "Open FreeSpace 2 Mission", wxGetCwd(), wxEmptyString, "FreeSpace2 Missions (*.fs2)|*.fs2| All Files (*.*)|*.*");

	if( dlg->ShowModal() != wxID_OK )
	{
		// User didn't click OK
		return;
	}

	currentFilename = dlg->GetFilename();
	// TODO: actually (attempt) to open the file
	SetFredTitle();
}

void frmFRED2::OnFileSave( wxCommandEvent &event )
{
// TODO: Implement OnFileSave
}

void frmFRED2::OnFileSaveAs( wxCommandEvent &event )
{
// TODO: Implement OnFileSaveAs
}

void frmFRED2::OnFileRevert( wxCommandEvent &event )
{
// TODO: Implement OnFileRevert
}

void frmFRED2::OnFileSaveFormatFs2Open( wxCommandEvent &event )
{
// TODO: Implement OnFileSaveFormatFs2Open
}

void frmFRED2::OnFileSaveFormatFs2Retail( wxCommandEvent &event )
{
// TODO: Implement OnFileSaveFormatFs2Retail
}

void frmFRED2::OnFileImportFs1Mission( wxCommandEvent &event )
{
// TODO: Implement OnFileImportFs1Mission
}

void frmFRED2::OnFileImportFs1WeaponLoadouts( wxCommandEvent &event )
{
// TODO: Implement OnFileImportFs1WeaponLoadouts
}

void frmFRED2::OnFileRunFs2( wxCommandEvent &event )
{
// TODO: Implement OnFileRunFs2
}

void frmFRED2::OnFileRecentFiles( wxCommandEvent &event )
{
// TODO: Implement OnFileRecentFiles
}

void frmFRED2::OnFileExit( wxCommandEvent &event )
{
// TODO: Implement OnFileExit
	this->Close();
}

void frmFRED2::OnEditUndo( wxCommandEvent &event )
{
// TODO: Implement OnEditUndo
}

void frmFRED2::OnEditDelete( wxCommandEvent &event )
{
// TODO: Implement OnEditDelete
}

void frmFRED2::OnEditDeleteWing( wxCommandEvent &event )
{
// TODO: Implement OnEditDeleteWing
}

void frmFRED2::OnEditDisableUndo( wxCommandEvent &event )
{
// TODO: Implement OnEditDisableUndo
}

void frmFRED2::OnViewToolbar( wxCommandEvent &event )
{
	if( event.IsChecked() == false )
	{
		// Control is cleared: User does not want to view the status bar
		tbrFRED->Hide();
	}
	else
	{
		// Else, control is filled: User wants to view the status bar
		tbrFRED->Show();
	}
}

void frmFRED2::OnViewStatusbar( wxCommandEvent &event )
{
	if( event.IsChecked() == false )
	{
		// Control is cleared: User does not want to view the status bar
		sbFRED->Hide();
	}
	else
	{
		// Else, control is filled: User wants to view the status bar
		sbFRED->Show();
	}
}

void frmFRED2::OnViewDisplayFilterShowShips( wxCommandEvent &event )
{
// TODO: Implement OnViewDisplayFilterShowShips
}

void frmFRED2::OnViewDisplayFilterShowPlayerStarts( wxCommandEvent &event )
{
// TODO: Implement OnViewDisplayFilterShowPlayerStarts
}

void frmFRED2::OnViewDisplayFilterShowWaypoints( wxCommandEvent &event )
{
// TODO: Implement OnViewDisplayFilterShowWaypoints
}

void frmFRED2::OnViewDisplayFilterShowFriendly( wxCommandEvent &event )
{
// TODO: Implement OnViewDisplayFilterShowFriendly
}

void frmFRED2::OnViewDisplayFilterShowHostile( wxCommandEvent &event )
{
// TODO: Implement OnViewDisplayFilterShowHostile
}

void frmFRED2::OnViewHideMarkedObjects( wxCommandEvent &event )
{
// TODO: Implement OnViewHideMarkedObjects
}

void frmFRED2::OnViewShowHiddenObjects( wxCommandEvent &event )
{
// TODO: Implement OnViewShowHiddenObjects
}

void frmFRED2::OnViewShowShipModels( wxCommandEvent &event )
{
// TODO: Implement OnViewShowShipModels
}

void frmFRED2::OnViewShowOutlines( wxCommandEvent &event )
{
// TODO: Implement OnViewShowOutlines
}

void frmFRED2::OnViewShowShipInfo( wxCommandEvent &event )
{
// TODO: Implement OnViewShowShipInfo
}

void frmFRED2::OnViewShowCoordinates( wxCommandEvent &event )
{
// TODO: Implement OnViewShowCoordinates
}

void frmFRED2::OnViewShowGridPositions( wxCommandEvent &event )
{
// TODO: Implement OnViewShowGridPositions
}

void frmFRED2::OnViewShowDistances( wxCommandEvent &event )
{
// TODO: Implement OnViewShowDistances
}

void frmFRED2::OnViewShowModelPaths( wxCommandEvent &event )
{
// TODO: Implement OnViewShowModelPaths
}

void frmFRED2::OnViewShowModelDockPoints( wxCommandEvent &event )
{
// TODO: Implement OnViewShowModelDockPoints
}

void frmFRED2::OnViewShowGrid( wxCommandEvent &event )
{
// TODO: Implement OnViewShowGrid
}

void frmFRED2::OnViewShowHorizon( wxCommandEvent &event )
{
// TODO: Implement OnViewShowHorizon
}

void frmFRED2::OnViewDoubleFineGridlines( wxCommandEvent &event )
{
// TODO: Implement OnViewDoubleFineGridlines
}

void frmFRED2::OnViewAntiAliasedGridlines( wxCommandEvent &event )
{
// TODO: Implement OnViewAntiAliasedGridlines
}

void frmFRED2::OnViewShow3DCompass( wxCommandEvent &event )
{
// TODO: Implement OnViewShow3DCompass
}

void frmFRED2::OnViewShowBackground( wxCommandEvent &event )
{
// TODO: Implement OnViewShowBackground
}

void frmFRED2::OnViewViewpointCamera( wxCommandEvent &event )
{
// TODO: Implement OnViewViewpointCamera
}

void frmFRED2::OnViewViewpointCurrentShip( wxCommandEvent &event )
{
// TODO: Implement OnViewViewpointCurrentShip
}

void frmFRED2::OnViewSaveCameraPos( wxCommandEvent &event )
{
// TODO: Implement OnViewSaveCameraPos
}

void frmFRED2::OnViewRestoreCameraPos( wxCommandEvent &event )
{
// TODO: Implement OnViewRestoreCameraPos
}

void frmFRED2::OnViewLightingFromSuns( wxCommandEvent &event )
{
// TODO: Implement OnViewLightingFromSuns
}

void frmFRED2::OnSpeedMovement( wxCommandEvent &event )
{
// TODO: Implement OnViewSpeedMovement
}

void frmFRED2::OnSpeedRotation( wxCommandEvent &event )
{
// TODO: Implement OnSpeedRotation
}

void frmFRED2::OnEditorsShips( wxCommandEvent &event )
{
	if( frmShipsEditor_p == NULL )
	{
		frmShipsEditor_p = new class frmShipsEditor( this, ID_frmShipsEditor );
	}

	frmShipsEditor_p->Show();
	frmShipsEditor_p->SetFocus();
}

void frmFRED2::OnEditorsWings( wxCommandEvent &event )
{
	if( frmWingEditor_p == NULL )
	{
		frmWingEditor_p = new class frmWingEditor( this, ID_frmWingEditor );
	}

	frmWingEditor_p->Show();
	frmWingEditor_p->SetFocus();
}

void frmFRED2::OnEditorsObjects( wxCommandEvent &event )
{
	if( dlgObjectEditor_p == NULL )
	{
		dlgObjectEditor_p = new class dlgObjectEditor( this, ID_dlgObjectEditor );
	}

	dlgObjectEditor_p->Show();
	dlgObjectEditor_p->SetFocus();
}

void frmFRED2::OnEditorsWaypointPaths( wxCommandEvent &event )
{
	if( frmWaypointEditor_p == NULL )
	{
		frmWaypointEditor_p = new class frmWaypointEditor( this, ID_frmWaypointEditor );
	}

	frmWaypointEditor_p->Show();
	frmWaypointEditor_p->SetFocus();
}

void frmFRED2::OnEditorsMissionObjectives( wxCommandEvent &event )
{
	if( dlgMissionObjectivesEditor_p == NULL )
	{
		dlgMissionObjectivesEditor_p = new class dlgMissionObjectivesEditor( this, ID_dlgMissionObjectivesEditor );
	}

	dlgMissionObjectivesEditor_p->Show();
	dlgMissionObjectivesEditor_p->SetFocus();
}

void frmFRED2::OnEditorsEvents( wxCommandEvent &event )
{
	if( dlgEventsEditor_p == NULL )
	{
		dlgEventsEditor_p = new class dlgEventsEditor( this, ID_dlgEventsEditor );
	}

	dlgEventsEditor_p->Show();
	dlgEventsEditor_p->SetFocus();
}

void frmFRED2::OnEditorsTeamLoadout( wxCommandEvent &event )
{
	if( frmTeamLoadoutEditor_p == NULL )
	{
		frmTeamLoadoutEditor_p = new class frmTeamLoadoutEditor( this, ID_frmTeamLoadoutEditor );
	}

	frmTeamLoadoutEditor_p->Show();
	frmTeamLoadoutEditor_p->SetFocus();
}

void frmFRED2::OnEditorsBackground( wxCommandEvent &event )
{
	if( dlgBackgroundEditor_p == NULL )
	{
		dlgBackgroundEditor_p = new class dlgBackgroundEditor( this, ID_dlgBackgroundEditor );
	}

	dlgBackgroundEditor_p->Show();
	dlgBackgroundEditor_p->SetFocus();
}

void frmFRED2::OnEditorsReinforcements( wxCommandEvent &event )
{
	if( dlgReinforcementsEditor_p == NULL )
	{
		dlgReinforcementsEditor_p = new class dlgReinforcementsEditor( this, ID_dlgReinforcementsEditor );
	}

	dlgReinforcementsEditor_p->Show();
	dlgReinforcementsEditor_p->SetFocus();
}

void frmFRED2::OnEditorsAsteroidField( wxCommandEvent &event )
{
	if( dlgAsteroidFieldEditor_p == NULL )
	{
		dlgAsteroidFieldEditor_p = new class dlgAsteroidFieldEditor( this, ID_dlgAsteroidFieldEditor );
	}

	dlgAsteroidFieldEditor_p->Show();
	dlgAsteroidFieldEditor_p->SetFocus();
}

void frmFRED2::OnEditorsMissionSpecs( wxCommandEvent &event )
{
	if( dlgMissionSpecsEditor_p == NULL )
	{
		dlgMissionSpecsEditor_p = new class dlgMissionSpecsEditor( this, ID_dlgMissionSpecsEditor );
	}

	dlgMissionSpecsEditor_p->Show();
	dlgMissionSpecsEditor_p->SetFocus();
}

void frmFRED2::OnEditorsBriefing( wxCommandEvent &event )
{
	if( frmBriefingEditor_p == NULL )
	{
		frmBriefingEditor_p = new class frmBriefingEditor( this, ID_frmBriefingEditor );
	}

	frmBriefingEditor_p->Show();
	frmBriefingEditor_p->SetFocus();
}

void frmFRED2::OnEditorsDebriefing( wxCommandEvent &event )
{
	if( frmDebriefingEditor_p == NULL )
	{
		frmDebriefingEditor_p = new class frmDebriefingEditor( this, ID_frmDebriefingEditor );
	}

	frmDebriefingEditor_p->Show();
	frmDebriefingEditor_p->SetFocus();
}

void frmFRED2::OnEditorsCommandBriefing( wxCommandEvent &event )
{
	if( frmCommandBriefingEditor_p == NULL )
	{
		frmCommandBriefingEditor_p = new class frmCommandBriefingEditor( this, ID_frmCommandBriefingEditor );
	}

	frmCommandBriefingEditor_p->Show();
	frmCommandBriefingEditor_p->SetFocus();
}

void frmFRED2::OnEditorsFictionViewer( wxCommandEvent &event )
{
	if( dlgFictionViewer_p == NULL )
	{
		dlgFictionViewer_p = new class dlgFictionViewer( this, ID_dlgFictionViewer );
	}

	dlgFictionViewer_p->Show();
	dlgFictionViewer_p->SetFocus();
}

void frmFRED2::OnEditorsShieldSystem( wxCommandEvent &event )
{
if( dlgShieldSystemEditor_p == NULL )
	{
		dlgShieldSystemEditor_p = new class dlgShieldSystemEditor( this, ID_dlgShieldSystemEditor );
	}

	dlgShieldSystemEditor_p->Show();
	dlgShieldSystemEditor_p->SetFocus();
}

void frmFRED2::OnEditorsSetGlobalShipFlags( wxCommandEvent &event )
{
if( dlgSetGlobalShipFlagsEditor_p == NULL )
	{
		dlgSetGlobalShipFlagsEditor_p = new class dlgSetGlobalShipFlagsEditor( this, ID_dlgSetGlobalShipFlagsEditor );
	}

	dlgSetGlobalShipFlagsEditor_p->Show();
	dlgSetGlobalShipFlagsEditor_p->SetFocus();
}

void frmFRED2::OnEditorsVoiceActingManager( wxCommandEvent &event )
{
if( dlgVoiceActingManager_p == NULL )
	{
		dlgVoiceActingManager_p = new class dlgVoiceActingManager( this, ID_dlgVoiceActingManager );
	}

	dlgVoiceActingManager_p->Show();
	dlgVoiceActingManager_p->SetFocus();
}

void frmFRED2::OnEditorsCampaign( wxCommandEvent &event )
{
if( frmCampaignEditor_p == NULL )
	{
		frmCampaignEditor_p = new class frmCampaignEditor( this, ID_frmCampaignEditor );
	}

	frmCampaignEditor_p->Show();
	frmCampaignEditor_p->SetFocus();
}

void frmFRED2::OnGroupsGroup( wxCommandEvent &event )
{
// TODO: Implement OnGroupsGroup
}

void frmFRED2::OnGroupsSetGroup( wxCommandEvent &event )
{
// TODO: Implement OnGroupsSetGroup
}

void frmFRED2::OnMiscLevelObject( wxCommandEvent &event )
{
// TODO: Implement OnMiscLevelObject
}

void frmFRED2::OnMiscAlignObject( wxCommandEvent &event )
{
// TODO: Implement OnMiscAlignObject
}

void frmFRED2::OnMiscMarkWing( wxCommandEvent &event )
{
// TODO: Implement OnMiscMarkWing
}

void frmFRED2::OnMiscControlObject( wxCommandEvent &event )
{
// TODO: Implement OnMiscControlObject
}

void frmFRED2::OnMiscNextObject( wxCommandEvent &event )
{
// TODO: Implement OnMiscNextObject
}

void frmFRED2::OnMiscPreviousObject( wxCommandEvent &event )
{
// TODO: Implement OnMiscPreviousObject
}

void frmFRED2::OnMiscAdjustGrid( wxCommandEvent &event )
{
// TODO: Implement OnMiscAdjustGrid
}

void frmFRED2::OnMiscNextSubsystem( wxCommandEvent &event )
{
// TODO: Implement OnMiscNextSubsystem
}

void frmFRED2::OnMiscPrevSubsystem( wxCommandEvent &event )
{
// TODO: Implement OnMiscPrevSubsystem
}

void frmFRED2::OnMiscCancelSubsystem( wxCommandEvent &event )
{
// TODO: Implement OnMiscCancelSubsystem
}

void frmFRED2::OnMiscMissionStatistics( wxCommandEvent &event )
{
if( dlgMissionStats_p == NULL )
	{
		dlgMissionStats_p = new class dlgMissionStats( this, ID_dlgMissionStats );
	}

	dlgMissionStats_p->Show();
	dlgMissionStats_p->SetFocus();
}

void frmFRED2::OnMiscErrorChecker( wxCommandEvent &event )
{
// TODO: Implement OnMiscErrorChecker
}

void frmFRED2::OnHelpHelpTopics( wxCommandEvent &event )
{
// TODO: Implement OnMiscHelpHelpTopics
}

void frmFRED2::OnHelpAbout( wxCommandEvent &event )
{
if( dlgAboutBox_p == NULL )
	{
		dlgAboutBox_p = new class dlgAboutBox( this, ID_dlgAboutBox );
	}

	dlgAboutBox_p->Show();
	dlgAboutBox_p->SetFocus();
}

void frmFRED2::OnHelpShowSexpHelp( wxCommandEvent &event )
{
if( dlgSexpHelp_p == NULL )
	{
		dlgSexpHelp_p = new class dlgSexpHelp( this, ID_dlgSexpHelp );
	}

	dlgSexpHelp_p->Show();
	dlgSexpHelp_p->SetFocus();
}

// Handlers for child dialogs & frames
bool frmFRED2::ChildIsOpen( const wxWindowID child_id )
{
	switch( child_id )
	{
		case ID_frmShipsEditor:
			return ( frmShipsEditor_p != NULL ) ? frmShipsEditor_p->IsShown() : false;

		case ID_frmWingEditor:
			return ( frmWingEditor_p != NULL ) ? frmWingEditor_p->IsShown() : false;

		case ID_dlgObjectEditor:
			return ( dlgObjectEditor_p != NULL ) ? dlgObjectEditor_p->IsShown() : false;

		case ID_frmWaypointEditor:
			return ( frmWaypointEditor_p != NULL ) ? frmWaypointEditor_p->IsShown() : false;

		case ID_dlgMissionObjectivesEditor:
			return ( dlgMissionObjectivesEditor_p != NULL ) ? dlgMissionObjectivesEditor_p->IsShown() : false;

		case ID_dlgEventsEditor:
			return ( dlgEventsEditor_p != NULL ) ? dlgEventsEditor_p->IsShown() : false;

		case ID_frmTeamLoadoutEditor:
			return ( frmTeamLoadoutEditor_p != NULL ) ? frmTeamLoadoutEditor_p->IsShown() : false;

		case ID_dlgBackgroundEditor:
			return ( dlgBackgroundEditor_p != NULL ) ? dlgBackgroundEditor_p->IsShown() : false;

		case ID_dlgReinforcementsEditor:
			return ( dlgReinforcementsEditor_p != NULL ) ? dlgReinforcementsEditor_p->IsShown() : false;

		case ID_dlgAsteroidFieldEditor:
			return ( dlgAsteroidFieldEditor_p != NULL ) ? dlgAsteroidFieldEditor_p->IsShown() : false;

		case ID_dlgMissionSpecsEditor:
			return ( dlgMissionSpecsEditor_p != NULL ) ? dlgMissionSpecsEditor_p->IsShown() : false;

		case ID_frmBriefingEditor:
			return ( frmBriefingEditor_p != NULL ) ? frmBriefingEditor_p->IsShown() : false;

		case ID_frmDebriefingEditor:
			return ( frmDebriefingEditor_p != NULL ) ? frmDebriefingEditor_p->IsShown() : false;

		case ID_frmCommandBriefingEditor:
			return ( frmCommandBriefingEditor_p != NULL ) ? frmCommandBriefingEditor_p->IsShown() : false;

		case ID_dlgFictionViewer:
			return ( dlgFictionViewer_p != NULL ) ? dlgFictionViewer_p->IsShown() : false;

		case ID_dlgShieldSystemEditor:
			return ( dlgShieldSystemEditor_p != NULL ) ? dlgShieldSystemEditor_p->IsShown() : false;

		case ID_dlgSetGlobalShipFlagsEditor:
			return ( dlgSetGlobalShipFlagsEditor_p != NULL ) ? dlgSetGlobalShipFlagsEditor_p->IsShown() : false;

		case ID_dlgVoiceActingManager:
			return ( dlgVoiceActingManager_p != NULL ) ? dlgVoiceActingManager_p->IsShown() : false;

		case ID_frmCampaignEditor:
			return ( frmCampaignEditor_p != NULL ) ? frmCampaignEditor_p->IsShown() : false;

		case ID_dlgMissionStats:
			return ( dlgMissionStats_p != NULL ) ? dlgMissionStats_p->IsShown() : false;

		case ID_dlgAboutBox:
			return ( dlgAboutBox_p != NULL ) ? dlgAboutBox_p->IsShown() : false;

		case ID_dlgSexpHelp:
			return ( dlgSexpHelp_p != NULL ) ? dlgSexpHelp_p->IsShown() : false;

		default:
			// Maybe return debug dialog. "Unknown child id"
			return false;
	}
}

void frmFRED2::OnChildClosed( wxWindow* const child )
{
	wxWindowID child_id = child->GetId();

	switch( child_id )
	{
		case ID_frmShipsEditor:
			frmShipsEditor_p = NULL;
			break;

		case ID_frmWingEditor:
			frmWingEditor_p = NULL;
			break;

		case ID_dlgObjectEditor:
			dlgObjectEditor_p = NULL;
			break;

		case ID_frmWaypointEditor:
			frmWaypointEditor_p = NULL;
			break;

		case ID_dlgMissionObjectivesEditor:
			dlgMissionObjectivesEditor_p = NULL;
			break;

		case ID_dlgEventsEditor:
			dlgEventsEditor_p = NULL;
			break;

		case ID_frmTeamLoadoutEditor:
			frmTeamLoadoutEditor_p = NULL;
			break;

		case ID_dlgBackgroundEditor:
			dlgBackgroundEditor_p = NULL;
			break;

		case ID_dlgReinforcementsEditor:
			dlgReinforcementsEditor_p = NULL;
			break;

		case ID_dlgAsteroidFieldEditor:
			dlgAsteroidFieldEditor_p = NULL;
			break;

		case ID_dlgMissionSpecsEditor:
			dlgMissionSpecsEditor_p = NULL;
			break;

		case ID_frmBriefingEditor:
			frmBriefingEditor_p = NULL;
			break;

		case ID_frmDebriefingEditor:
			frmDebriefingEditor_p = NULL;
			break;

		case ID_frmCommandBriefingEditor:
			frmCommandBriefingEditor_p = NULL;
			break;

		case ID_dlgFictionViewer:
			dlgFictionViewer_p = NULL;
			break;

		case ID_dlgShieldSystemEditor:
			dlgShieldSystemEditor_p = NULL;
			break;

		case ID_dlgSetGlobalShipFlagsEditor:
			dlgSetGlobalShipFlagsEditor_p = NULL;
			break;

		case ID_dlgVoiceActingManager:
			dlgVoiceActingManager_p = NULL;
			break;

		case ID_frmCampaignEditor:
			frmCampaignEditor_p = NULL;
			break;

		case ID_dlgMissionStats:
			dlgMissionStats_p = NULL;
			break;

		case ID_dlgAboutBox:
			dlgAboutBox_p = NULL;
			break;

		case ID_dlgSexpHelp:
			dlgSexpHelp_p = NULL;
			break;

		default:
			// wxFRED must close in order to prevent anything from exploding.
			// TODO: determine if any changes were made prior to the crash. If so, try to save the changes so the user's work won't be lost
			wxChar message[255];
			wxSprintf( message, "Child ID: %i \n\nwxFRED must now close, due to an unknown child window closing.\nPlease inform an SCP member of the Child ID number listed above.", child_id );
			wxMessageBox( message, _T("Unknown child window closed"));
			Close();
	}
}
