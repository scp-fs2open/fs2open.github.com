///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxfred_base.h"

#include "../res/constx.png.h"
#include "../res/constxy.png.h"
#include "../res/constxz.png.h"
#include "../res/consty.png.h"
#include "../res/constyz.png.h"
#include "../res/constz.png.h"
#include "../res/fred_splash.xpm"
#include "../res/orbitsel.png.h"
#include "../res/play.xpm"
#include "../res/rotlocal.png.h"
#include "../res/select.png.h"
#include "../res/selectlist.png.h"
#include "../res/selectlock.png.h"
#include "../res/selectmove.png.h"
#include "../res/selectrot.png.h"
#include "../res/showdist.png.h"
#include "../res/wingdisband.png.h"
#include "../res/wingform.png.h"
#include "../res/zoomext.png.h"
#include "../res/zoomsel.png.h"

///////////////////////////////////////////////////////////////////////////
using namespace fredBase;

BEGIN_EVENT_TABLE( frmFRED, wxFrame )
	EVT_CLOSE( frmFRED::_wxFB_OnClose )
	EVT_MENU( ID_mnuFileNew, frmFRED::_wxFB_OnFileNew )
	EVT_MENU( ID_mnuFileOpen, frmFRED::_wxFB_OnFileOpen )
	EVT_MENU( ID_mnuFileSave, frmFRED::_wxFB_OnFileSave )
	EVT_MENU( ID_mnuFileSaveAs, frmFRED::_wxFB_OnFileSaveAs )
	EVT_MENU( ID_mnuFileRevert, frmFRED::_wxFB_OnFileRevert )
	EVT_MENU( ID_mnuFileSaveFormatFs2Open, frmFRED::_wxFB_OnFileSaveFormatFs2Open )
	EVT_MENU( ID_mnuFileSaveFormatFs2Retail, frmFRED::_wxFB_OnFileSaveFormatFs2Retail )
	EVT_MENU( ID_mnuFileImportFs1Mission, frmFRED::_wxFB_OnFileImportFs1Mission )
	EVT_MENU( ID_mnuFileImportWeaponLoadouts, frmFRED::_wxFB_OnFileImportWeaponLoadouts )
	EVT_MENU( ID_mnuFileRunFreespace, frmFRED::_wxFB_OnFileRunFreespace )
	EVT_MENU( ID_mnuFileRecentFiles, frmFRED::_wxFB_OnFileRecentFiles )
	EVT_MENU( ID_mnuFileExit, frmFRED::_wxFB_OnFileExit )
	EVT_MENU( ID_mnuEditUndo, frmFRED::_wxFB_OnEditUndo )
	EVT_MENU( ID_mnuEditDelete, frmFRED::_wxFB_OnEditDelete )
	EVT_MENU( ID_mnuEditDeleteWing, frmFRED::_wxFB_OnEditDeleteWing )
	EVT_MENU( ID_mnuEditDisableUndo, frmFRED::_wxFB_OnEditDisableUndo )
	EVT_MENU( ID_mnuViewToolbar, frmFRED::_wxFB_OnViewToolbar )
	EVT_MENU( ID_mnuViewStatusBar, frmFRED::_wxFB_OnViewStatusBar )
	EVT_MENU( ID_mnuViewDispayFilterShowShips, frmFRED::_wxFB_OnViewDisplayFilterShowShips )
	EVT_MENU( ID_mnuViewDisplayFilterShowPlayerStarts, frmFRED::_wxFB_OnViewDisplayFilterShowPlayerStarts )
	EVT_MENU( ID_mnuViewDisplayFilterShowWaypoints, frmFRED::_wxFB_OnViewDisplayFilterShowWaypoints )
	EVT_MENU( ID_mnuViewDisplayFilterShowFriendly, frmFRED::_wxFB_OnViewDisplayFilterShowFriendly )
	EVT_MENU( ID_mnuViewDisplayFilterShowHostile, frmFRED::_wxFB_OnViewDisplayFilterShowHostile )
	EVT_MENU( ID_mnuViewHideMarkedObjects, frmFRED::_wxFB_OnViewHideMarkedObjects )
	EVT_MENU( ID_mnuViewShowHiddenObjects, frmFRED::_wxFB_OnViewShowHiddenObjects )
	EVT_MENU( ID_mnuViewShowShipModels, frmFRED::_wxFB_OnViewShowShipModels )
	EVT_MENU( ID_mnuViewShowOutlines, frmFRED::_wxFB_OnViewShowOutlines )
	EVT_MENU( ID_mnuViewShowShipInfo, frmFRED::_wxFB_OnViewShowShipInfo )
	EVT_MENU( ID_mnuViewShowCoordinates, frmFRED::_wxFB_OnViewShowCoordinates )
	EVT_MENU( ID_mnuViewShowGridPositions, frmFRED::_wxFB_OnViewShowGridPositions )
	EVT_MENU( ID_ViewShowDistances, frmFRED::_wxFB_OnViewShowDistances )
	EVT_MENU( ID_mnuViewShowModelPaths, frmFRED::_wxFB_OnViewShowModelPaths )
	EVT_MENU( ID_mnuViewShowModelDockPoints, frmFRED::_wxFB_OnViewShowModelDockPoints )
	EVT_MENU( ID_mnuViewShowGrid, frmFRED::_wxFB_OnViewShowGrid )
	EVT_MENU( ID_mnuViewShowHorizon, frmFRED::_wxFB_OnViewShowHorizon )
	EVT_MENU( ID_mnuViewDoubleFineGridlines, frmFRED::_wxFB_OnViewDoubleFineGridlines )
	EVT_MENU( ID_mnuViewAntiAliasedGridlines, frmFRED::_wxFB_OnViewAntiAliasedGridlines )
	EVT_MENU( ID_mnuViewShow3DCompass, frmFRED::_wxFB_OnViewShow3DCompass )
	EVT_MENU( ID_mnuViewShowBackground, frmFRED::_wxFB_OnViewShowBackground )
	EVT_MENU( ID_ViewViewpointCamera, frmFRED::_wxFB_OnViewViewpointCamera )
	EVT_MENU( ID_mnuViewViewpointCurrentShip, frmFRED::_wxFB_OnViewViewpointCurrentShip )
	EVT_MENU( ID_mnuViewSaveCameraPos, frmFRED::_wxFB_OnViewSaveCameraPos )
	EVT_MENU( ID_mnuViewRestoreCameraPos, frmFRED::_wxFB_OnViewRestoreCameraPos )
	EVT_MENU( ID_mnuViewLightingFromSuns, frmFRED::_wxFB_OnViewLightingFromSuns )
	EVT_MENU( ID_mnuSpeedMovementX1, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedMovementX2, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedMovementX3, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedMovementX5, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedMovementX8, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedMovementX10, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedMovementX50, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedMovementX100, frmFRED::_wxFB_OnSpeedMovement )
	EVT_MENU( ID_mnuSpeedRotationX1, frmFRED::_wxFB_OnSpeedRotation )
	EVT_MENU( ID_mnuSpeedRotationX5, frmFRED::_wxFB_OnSpeedRotation )
	EVT_MENU( ID_mnuSpeedRotationX12, frmFRED::_wxFB_OnSpeedRotation )
	EVT_MENU( ID_mnuSpeedRotationX25, frmFRED::_wxFB_OnSpeedRotation )
	EVT_MENU( ID_mnuSpeedRotationX50, frmFRED::_wxFB_OnSpeedRotation )
	EVT_MENU( ID_mnuEditorsShips, frmFRED::_wxFB_OnEditorsShips )
	EVT_MENU( ID_mnuEditorsWings, frmFRED::_wxFB_OnEditorsWings )
	EVT_MENU( ID_mnuEditorsObjects, frmFRED::_wxFB_OnEditorsObjects )
	EVT_MENU( ID_mnuEditorsWaypointPaths, frmFRED::_wxFB_OnEditorsWaypointPaths )
	EVT_MENU( ID_mnuEditorsMissionObjectives, frmFRED::_wxFB_OnEditorsMissionObjectives )
	EVT_MENU( ID_mnuEditorsEvents, frmFRED::_wxFB_OnEditorsEvents )
	EVT_MENU( ID_mnuEditorsTeamLoadout, frmFRED::_wxFB_OnEditorsTeamLoadout )
	EVT_MENU( ID_mnuEditorsBackground, frmFRED::_wxFB_OnEditorsBackground )
	EVT_MENU( ID_mnuEditorsReinforcements, frmFRED::_wxFB_OnEditorsReinforcements )
	EVT_MENU( ID_mnuEditorsAsteroidField, frmFRED::_wxFB_OnEditorsAsteroidField )
	EVT_MENU( ID_mnuEditorsMissionSpecs, frmFRED::_wxFB_OnEditorsMissionSpecs )
	EVT_MENU( ID_mnuEditorsBriefing, frmFRED::_wxFB_OnEditorsBriefing )
	EVT_MENU( ID_mnuEditorsDebriefing, frmFRED::_wxFB_OnEditorsDebriefing )
	EVT_MENU( ID_mnuEditorsCommandBriefing, frmFRED::_wxFB_OnEditorsCommandBriefing )
	EVT_MENU( ID_mnuEditorsFictionViewer, frmFRED::_wxFB_OnEditorsFictionViewer )
	EVT_MENU( ID_mnuEditorsShieldSystem, frmFRED::_wxFB_OnEditorsShieldSystem )
	EVT_MENU( ID_mnuEditorsSetGlobalShipFlags, frmFRED::_wxFB_OnEditorsSetGlobalShipFlags )
	EVT_MENU( ID_mnuEditorsVoiceActingManager, frmFRED::_wxFB_OnEditorsVoiceActingManager )
	EVT_MENU( ID_mnuEditorsCampaign, frmFRED::_wxFB_OnEditorsCampaign )
	EVT_MENU( ID_mnuGroupsGroup1, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup2, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup3, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup4, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup5, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup6, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup7, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup8, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsGroup9, frmFRED::_wxFB_OnGroupsGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup1, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup2, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup3, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup4, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup5, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup6, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup7, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup8, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuGroupsSetGroupGroup9, frmFRED::_wxFB_OnGroupsSetGroup )
	EVT_MENU( ID_mnuMiscLevelObject, frmFRED::_wxFB_OnMiscLevelObject )
	EVT_MENU( ID_mnuMiscAlignObject, frmFRED::_wxFB_OnMiscAlignObject )
	EVT_MENU( ID_mnuMiscMarkWing, frmFRED::_wxFB_OnMiscMarkWing )
	EVT_MENU( ID_mnuMiscControlObject, frmFRED::_wxFB_OnMiscControlObject )
	EVT_MENU( ID_mnuMiscNextObject, frmFRED::_wxFB_OnMiscNextObject )
	EVT_MENU( ID_mnuMiscPreviousObject, frmFRED::_wxFB_OnMiscPreviousObject )
	EVT_MENU( ID_mnuMiscAdjustGrid, frmFRED::_wxFB_OnMiscAdjustGrid )
	EVT_MENU( ID_mnuMiscNextSubsystem, frmFRED::_wxFB_OnMiscNextSubsystem )
	EVT_MENU( ID_mnuMiscPrevSubsystem, frmFRED::_wxFB_OnMiscPrevSubsystem )
	EVT_MENU( ID_mnuMiscCancelSubsystem, frmFRED::_wxFB_OnMiscCancelSubsystem )
	EVT_MENU( ID_mnuMiscMissionStatistics, frmFRED::_wxFB_OnMiscMissionStatistics )
	EVT_MENU( ID_mnuMiscErrorChecker, frmFRED::_wxFB_OnMiscErrorChecker )
	EVT_MENU( ID_mnuHelpHelpTopics, frmFRED::_wxFB_OnHelpHelpTopics )
	EVT_MENU( ID_mnuHelpShowSexpHelp, frmFRED::_wxFB_OnHelpShowSexpHelp )
	EVT_MENU( ID_mnuHelpAbout, frmFRED::_wxFB_OnHelpAbout )
	EVT_TOOL( ID_optSelect, frmFRED::_wxFB_OnSelect )
	EVT_TOOL( ID_optSelectMove, frmFRED::_wxFB_OnSelectMove )
	EVT_TOOL( ID_optSelectRotate, frmFRED::_wxFB_OnSelectRotate )
	EVT_TOOL( ID_chkRotateLocally, frmFRED::_wxFB_OnRotateLocally )
	EVT_TOOL( ID_optConstraintX, frmFRED::_wxFB_OnConstraintX )
	EVT_TOOL( ID_optConstraintY, frmFRED::_wxFB_OnConstraintY )
	EVT_TOOL( ID_optConstraintZ, frmFRED::_wxFB_OnConstraintZ )
	EVT_TOOL( ID_optConstraintXZ, frmFRED::_wxFB_OnConstraintXZ )
	EVT_TOOL( ID_optConstraintYZ, frmFRED::_wxFB_OnConstraintYZ )
	EVT_TOOL( ID_optConstraintXY, frmFRED::_wxFB_OnConstraintXY )
	EVT_TOOL( ID_btnSelectionList, frmFRED::_wxFB_OnSelectionList )
	EVT_TOOL( ID_chkSelectionLock, frmFRED::_wxFB_OnSelectionLock )
	EVT_TOOL( ID_btnWingForm, frmFRED::_wxFB_OnWingForm )
	EVT_TOOL( wxID_ANY, frmFRED::_wxFB_OnWingDisband )
	EVT_TOOL( ID_btnZoomSelected, frmFRED::_wxFB_OnZoomSelected )
	EVT_TOOL( ID_btnZoomExtents, frmFRED::_wxFB_OnZoomExtents )
	EVT_TOOL( ID_chkShowDistances, frmFRED::_wxFB_OnShowDistances )
	EVT_TOOL( ID_chkOrbitSelected, frmFRED::_wxFB_OnOrbitSelected )
END_EVENT_TABLE()

frmFRED::frmFRED( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 800,300 ), wxDefaultSize );
	
	mbrFRED = new wxMenuBar( 0 );
	mnuFile = new wxMenu();
	wxMenuItem* mnuFileNew;
	mnuFileNew = new wxMenuItem( mnuFile, ID_mnuFileNew, wxString( wxT("New") ) + wxT('\t') + wxT("Ctrl+N"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileNew );
	
	wxMenuItem* mnuFileOpen;
	mnuFileOpen = new wxMenuItem( mnuFile, ID_mnuFileOpen, wxString( wxT("Open...") ) + wxT('\t') + wxT("Ctrl-O"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileOpen );
	
	wxMenuItem* mnuFileSave;
	mnuFileSave = new wxMenuItem( mnuFile, ID_mnuFileSave, wxString( wxT("Save") ) + wxT('\t') + wxT("Ctrl+S"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSave );
	
	wxMenuItem* mnuFileSaveAs;
	mnuFileSaveAs = new wxMenuItem( mnuFile, ID_mnuFileSaveAs, wxString( wxT("Save As...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSaveAs );
	
	wxMenuItem* mnuFileRevert;
	mnuFileRevert = new wxMenuItem( mnuFile, ID_mnuFileRevert, wxString( wxT("Revert") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileRevert );
	
	mnuFile->AppendSeparator();
	
	mnuFileSaveFormat = new wxMenu();
	wxMenuItem* mnuFileSaveFormatFs2Open;
	mnuFileSaveFormatFs2Open = new wxMenuItem( mnuFileSaveFormat, ID_mnuFileSaveFormatFs2Open, wxString( wxT("FS2 open") ) , wxEmptyString, wxITEM_RADIO );
	mnuFileSaveFormat->Append( mnuFileSaveFormatFs2Open );
	
	wxMenuItem* mnuFileSaveFormatFs2Retail;
	mnuFileSaveFormatFs2Retail = new wxMenuItem( mnuFileSaveFormat, ID_mnuFileSaveFormatFs2Retail, wxString( wxT("FS2 retail") ) , wxEmptyString, wxITEM_RADIO );
	mnuFileSaveFormat->Append( mnuFileSaveFormatFs2Retail );
	
	mnuFile->Append( -1, wxT("Save Format"), mnuFileSaveFormat );
	
	mnuFileImport = new wxMenu();
	wxMenuItem* mnuFileImportFs1Mission;
	mnuFileImportFs1Mission = new wxMenuItem( mnuFileImport, ID_mnuFileImportFs1Mission, wxString( wxT("FS1 mission...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFileImport->Append( mnuFileImportFs1Mission );
	
	wxMenuItem* mnuFileImportWeaponLoadouts;
	mnuFileImportWeaponLoadouts = new wxMenuItem( mnuFileImport, ID_mnuFileImportWeaponLoadouts, wxString( wxT("FS1 weapon loadouts...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFileImport->Append( mnuFileImportWeaponLoadouts );
	
	mnuFile->Append( -1, wxT("Import"), mnuFileImport );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileRunFreespace;
	mnuFileRunFreespace = new wxMenuItem( mnuFile, ID_mnuFileRunFreespace, wxString( wxT("Run Freespace") ) + wxT('\t') + wxT("Alt+R"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileRunFreespace );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileRecentFiles;
	mnuFileRecentFiles = new wxMenuItem( mnuFile, ID_mnuFileRecentFiles, wxString( wxT("Recent File List") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileRecentFiles );
	mnuFileRecentFiles->Enable( false );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileExit;
	mnuFileExit = new wxMenuItem( mnuFile, ID_mnuFileExit, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileExit );
	
	mbrFRED->Append( mnuFile, wxT("File") ); 
	
	mnuEdit = new wxMenu();
	wxMenuItem* mnuEditUndo;
	mnuEditUndo = new wxMenuItem( mnuEdit, ID_mnuEditUndo, wxString( wxT("Undo") ) + wxT('\t') + wxT("Ctrl+Z"), wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditUndo );
	
	wxMenuItem* mnuEditDelete;
	mnuEditDelete = new wxMenuItem( mnuEdit, ID_mnuEditDelete, wxString( wxT("Delete") ) + wxT('\t') + wxT("Del"), wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditDelete );
	
	wxMenuItem* mnuEditDeleteWing;
	mnuEditDeleteWing = new wxMenuItem( mnuEdit, ID_mnuEditDeleteWing, wxString( wxT("Delete Wing") ) + wxT('\t') + wxT("Ctrl+Del"), wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditDeleteWing );
	
	mnuEdit->AppendSeparator();
	
	wxMenuItem* mnuEditDisableUndo;
	mnuEditDisableUndo = new wxMenuItem( mnuEdit, ID_mnuEditDisableUndo, wxString( wxT("Disable Undo") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditDisableUndo );
	
	mbrFRED->Append( mnuEdit, wxT("Edit") ); 
	
	mnuView = new wxMenu();
	wxMenuItem* mnuViewToolbar;
	mnuViewToolbar = new wxMenuItem( mnuView, ID_mnuViewToolbar, wxString( wxT("Toolbar") ) , wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewToolbar );
	mnuViewToolbar->Check( true );
	
	wxMenuItem* mnuViewStatusBar;
	mnuViewStatusBar = new wxMenuItem( mnuView, ID_mnuViewStatusBar, wxString( wxT("Status Bar") ) , wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewStatusBar );
	mnuViewStatusBar->Check( true );
	
	mnuView->AppendSeparator();
	
	mnuViewDisplayFiter = new wxMenu();
	wxMenuItem* mnuViewDisplayFilterShowShips;
	mnuViewDisplayFilterShowShips = new wxMenuItem( mnuViewDisplayFiter, ID_mnuViewDispayFilterShowShips, wxString( wxT("Show Ships") ) , wxEmptyString, wxITEM_CHECK );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowShips );
	
	wxMenuItem* mnuViewDisplayFilterShowPlayerStarts;
	mnuViewDisplayFilterShowPlayerStarts = new wxMenuItem( mnuViewDisplayFiter, ID_mnuViewDisplayFilterShowPlayerStarts, wxString( wxT("Show Player Starts") ) , wxEmptyString, wxITEM_CHECK );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowPlayerStarts );
	
	wxMenuItem* mnuViewDisplayFilterShowWaypoints;
	mnuViewDisplayFilterShowWaypoints = new wxMenuItem( mnuViewDisplayFiter, ID_mnuViewDisplayFilterShowWaypoints, wxString( wxT("Show Waypoints") ) , wxEmptyString, wxITEM_CHECK );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowWaypoints );
	
	mnuViewDisplayFiter->AppendSeparator();
	
	wxMenuItem* mnuViewDisplayFilterShowFriendly;
	mnuViewDisplayFilterShowFriendly = new wxMenuItem( mnuViewDisplayFiter, ID_mnuViewDisplayFilterShowFriendly, wxString( wxT("Show Friendly") ) , wxEmptyString, wxITEM_CHECK );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowFriendly );
	
	wxMenuItem* mnuViewDisplayFilterShowHostile;
	mnuViewDisplayFilterShowHostile = new wxMenuItem( mnuViewDisplayFiter, ID_mnuViewDisplayFilterShowHostile, wxString( wxT("Show Hostile") ) , wxEmptyString, wxITEM_CHECK );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowHostile );
	
	mnuView->Append( -1, wxT("Display Filter"), mnuViewDisplayFiter );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewHideMarkedObjects;
	mnuViewHideMarkedObjects = new wxMenuItem( mnuView, ID_mnuViewHideMarkedObjects, wxString( wxT("Hide Marked Objects") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewHideMarkedObjects );
	
	wxMenuItem* mnuViewShowHiddenObjects;
	mnuViewShowHiddenObjects = new wxMenuItem( mnuView, ID_mnuViewShowHiddenObjects, wxString( wxT("Show Hidden Objects") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowHiddenObjects );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewShowShipModels;
	mnuViewShowShipModels = new wxMenuItem( mnuView, ID_mnuViewShowShipModels, wxString( wxT("Show Ship Models") ) + wxT('\t') + wxT("Shift+Alt+M"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowShipModels );
	
	wxMenuItem* mnuViewShowOutlines;
	mnuViewShowOutlines = new wxMenuItem( mnuView, ID_mnuViewShowOutlines, wxString( wxT("Show Outlines") ) + wxT('\t') + wxT("Shift+Alt+O"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowOutlines );
	
	wxMenuItem* mnuViewShowShipInfo;
	mnuViewShowShipInfo = new wxMenuItem( mnuView, ID_mnuViewShowShipInfo, wxString( wxT("Show Ship Info") ) + wxT('\t') + wxT("Shift+Alt+I"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowShipInfo );
	
	wxMenuItem* mnuViewShowCoordinates;
	mnuViewShowCoordinates = new wxMenuItem( mnuView, ID_mnuViewShowCoordinates, wxString( wxT("Show Coordinates") ) + wxT('\t') + wxT("Shift+Alt+C"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowCoordinates );
	
	wxMenuItem* mnuViewShowGridPositions;
	mnuViewShowGridPositions = new wxMenuItem( mnuView, ID_mnuViewShowGridPositions, wxString( wxT("Show Grid Positions") ) + wxT('\t') + wxT("Shift+Alt+P"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowGridPositions );
	
	wxMenuItem* mnuViewShowDistances;
	mnuViewShowDistances = new wxMenuItem( mnuView, ID_ViewShowDistances, wxString( wxT("Show Distances") ) + wxT('\t') + wxT("D"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowDistances );
	
	wxMenuItem* mnuViewShowModelPaths;
	mnuViewShowModelPaths = new wxMenuItem( mnuView, ID_mnuViewShowModelPaths, wxString( wxT("Show Model Paths") ) , wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowModelPaths );
	
	wxMenuItem* mnuViewShowModelDockPoints;
	mnuViewShowModelDockPoints = new wxMenuItem( mnuView, ID_mnuViewShowModelDockPoints, wxString( wxT("Show Model Dock Points") ) , wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowModelDockPoints );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewShowGrid;
	mnuViewShowGrid = new wxMenuItem( mnuView, ID_mnuViewShowGrid, wxString( wxT("Show Grid") ) + wxT('\t') + wxT("Shift+Alt+G"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowGrid );
	
	wxMenuItem* mnuViewShowHorizon;
	mnuViewShowHorizon = new wxMenuItem( mnuView, ID_mnuViewShowHorizon, wxString( wxT("Show Horizon") ) + wxT('\t') + wxT("Shift+Alt+H"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowHorizon );
	
	wxMenuItem* mnuViewDoubleFineGridlines;
	mnuViewDoubleFineGridlines = new wxMenuItem( mnuView, ID_mnuViewDoubleFineGridlines, wxString( wxT("Double Fine Gridlines") ) , wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewDoubleFineGridlines );
	
	wxMenuItem* mnuViewAntiAliasedGridlines;
	mnuViewAntiAliasedGridlines = new wxMenuItem( mnuView, ID_mnuViewAntiAliasedGridlines, wxString( wxT("Anti-Aliased Gridlines") ) , wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewAntiAliasedGridlines );
	
	wxMenuItem* mnuViewShow3DCompass;
	mnuViewShow3DCompass = new wxMenuItem( mnuView, ID_mnuViewShow3DCompass, wxString( wxT("Show 3-D Compass") ) + wxT('\t') + wxT("Shift+Alt+3"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShow3DCompass );
	
	wxMenuItem* mnuViewShowBackground;
	mnuViewShowBackground = new wxMenuItem( mnuView, ID_mnuViewShowBackground, wxString( wxT("Show Background") ) + wxT('\t') + wxT("Shift+Alt+B"), wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewShowBackground );
	
	mnuView->AppendSeparator();
	
	mnuViewViewpoint = new wxMenu();
	wxMenuItem* mnuViewViewpointCamera;
	mnuViewViewpointCamera = new wxMenuItem( mnuViewViewpoint, ID_ViewViewpointCamera, wxString( wxT("Camera") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewViewpoint->Append( mnuViewViewpointCamera );
	
	wxMenuItem* mnuViewViewpointCurrentShip;
	mnuViewViewpointCurrentShip = new wxMenuItem( mnuViewViewpoint, ID_mnuViewViewpointCurrentShip, wxString( wxT("Current Ship") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewViewpoint->Append( mnuViewViewpointCurrentShip );
	
	mnuView->Append( -1, wxT("Viewpoint\tShift+V"), mnuViewViewpoint );
	
	wxMenuItem* mnuViewSaveCameraPos;
	mnuViewSaveCameraPos = new wxMenuItem( mnuView, ID_mnuViewSaveCameraPos, wxString( wxT("Save Camera Pos") ) + wxT('\t') + wxT("Ctrl+P"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewSaveCameraPos );
	
	wxMenuItem* mnuViewRestoreCameraPos;
	mnuViewRestoreCameraPos = new wxMenuItem( mnuView, ID_mnuViewRestoreCameraPos, wxString( wxT("Restore Camera Pos") ) + wxT('\t') + wxT("Ctrl+R"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewRestoreCameraPos );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewLightingFromSuns;
	mnuViewLightingFromSuns = new wxMenuItem( mnuView, ID_mnuViewLightingFromSuns, wxString( wxT("Lighting From Suns") ) , wxEmptyString, wxITEM_CHECK );
	mnuView->Append( mnuViewLightingFromSuns );
	
	mbrFRED->Append( mnuView, wxT("View") ); 
	
	mnuSpeed = new wxMenu();
	mnuSpeedMovement = new wxMenu();
	wxMenuItem* mnuSpeedMovementX1;
	mnuSpeedMovementX1 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX1, wxString( wxT("x1") ) + wxT('\t') + wxT("1"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX1 );
	
	wxMenuItem* mnuSpeedMovementX2;
	mnuSpeedMovementX2 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX2, wxString( wxT("x2") ) + wxT('\t') + wxT("2"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX2 );
	
	wxMenuItem* mnuSpeedMovementX3;
	mnuSpeedMovementX3 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX3, wxString( wxT("x3") ) + wxT('\t') + wxT("3"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX3 );
	
	wxMenuItem* mnuSpeedMovementX5;
	mnuSpeedMovementX5 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX5, wxString( wxT("x5") ) + wxT('\t') + wxT("4"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX5 );
	
	wxMenuItem* mnuSpeedMovementX8;
	mnuSpeedMovementX8 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX8, wxString( wxT("x8") ) + wxT('\t') + wxT("5"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX8 );
	
	wxMenuItem* mnuSpeedMovementX10;
	mnuSpeedMovementX10 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX10, wxString( wxT("x10") ) + wxT('\t') + wxT("6"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX10 );
	
	wxMenuItem* mnuSpeedMovementX50;
	mnuSpeedMovementX50 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX50, wxString( wxT("x50") ) + wxT('\t') + wxT("7"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX50 );
	
	wxMenuItem* mnuSpeedMovementX100;
	mnuSpeedMovementX100 = new wxMenuItem( mnuSpeedMovement, ID_mnuSpeedMovementX100, wxString( wxT("x100") ) + wxT('\t') + wxT("8"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX100 );
	
	mnuSpeed->Append( -1, wxT("Movement"), mnuSpeedMovement );
	
	mnuSpeedRotation = new wxMenu();
	wxMenuItem* mnuSpeedRotationX1;
	mnuSpeedRotationX1 = new wxMenuItem( mnuSpeedRotation, ID_mnuSpeedRotationX1, wxString( wxT("x1") ) + wxT('\t') + wxT("Shift+1"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX1 );
	
	wxMenuItem* mnuSpeedRotationX5;
	mnuSpeedRotationX5 = new wxMenuItem( mnuSpeedRotation, ID_mnuSpeedRotationX5, wxString( wxT("x5") ) + wxT('\t') + wxT("Shift+2"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX5 );
	
	wxMenuItem* mnuSpeedRotationX12;
	mnuSpeedRotationX12 = new wxMenuItem( mnuSpeedRotation, ID_mnuSpeedRotationX12, wxString( wxT("x12") ) + wxT('\t') + wxT("Shift+3"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX12 );
	
	wxMenuItem* mnuSpeedRotationX25;
	mnuSpeedRotationX25 = new wxMenuItem( mnuSpeedRotation, ID_mnuSpeedRotationX25, wxString( wxT("x25") ) + wxT('\t') + wxT("Shift+4"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX25 );
	
	wxMenuItem* mnuSpeedRotationX50;
	mnuSpeedRotationX50 = new wxMenuItem( mnuSpeedRotation, ID_mnuSpeedRotationX50, wxString( wxT("x50") ) + wxT('\t') + wxT("Shift+5"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX50 );
	
	mnuSpeed->Append( -1, wxT("Rotation"), mnuSpeedRotation );
	
	mbrFRED->Append( mnuSpeed, wxT("Speed") ); 
	
	mnuEditors = new wxMenu();
	wxMenuItem* mnuEditorsShips;
	mnuEditorsShips = new wxMenuItem( mnuEditors, ID_mnuEditorsShips, wxString( wxT("Ships") ) + wxT('\t') + wxT("Shift+S"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsShips );
	
	wxMenuItem* mnuEditorsWings;
	mnuEditorsWings = new wxMenuItem( mnuEditors, ID_mnuEditorsWings, wxString( wxT("Wings") ) + wxT('\t') + wxT("Shift+W"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsWings );
	
	wxMenuItem* mnuEditorsObjects;
	mnuEditorsObjects = new wxMenuItem( mnuEditors, ID_mnuEditorsObjects, wxString( wxT("Objects") ) + wxT('\t') + wxT("Shift+O"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsObjects );
	
	wxMenuItem* mnuEditorsWaypointPaths;
	mnuEditorsWaypointPaths = new wxMenuItem( mnuEditors, ID_mnuEditorsWaypointPaths, wxString( wxT("Waypoint Paths") ) + wxT('\t') + wxT("Shift+Y"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsWaypointPaths );
	
	wxMenuItem* mnuEditorsMissionObjectives;
	mnuEditorsMissionObjectives = new wxMenuItem( mnuEditors, ID_mnuEditorsMissionObjectives, wxString( wxT("Mission Objectives") ) + wxT('\t') + wxT("Shift+G"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsMissionObjectives );
	
	wxMenuItem* mnuEditorsEvents;
	mnuEditorsEvents = new wxMenuItem( mnuEditors, ID_mnuEditorsEvents, wxString( wxT("Events") ) + wxT('\t') + wxT("Shift+E"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsEvents );
	
	wxMenuItem* mnuEditorsTeamLoadout;
	mnuEditorsTeamLoadout = new wxMenuItem( mnuEditors, ID_mnuEditorsTeamLoadout, wxString( wxT("Team Loadout") ) + wxT('\t') + wxT("Shift+P"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsTeamLoadout );
	
	wxMenuItem* mnuEditorsBackground;
	mnuEditorsBackground = new wxMenuItem( mnuEditors, ID_mnuEditorsBackground, wxString( wxT("Background") ) + wxT('\t') + wxT("Shift+I"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsBackground );
	
	wxMenuItem* mnuEditorsReinforcements;
	mnuEditorsReinforcements = new wxMenuItem( mnuEditors, ID_mnuEditorsReinforcements, wxString( wxT("Reinforcements") ) + wxT('\t') + wxT("Shift+R"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsReinforcements );
	
	wxMenuItem* mnuEditorsAsteroidField;
	mnuEditorsAsteroidField = new wxMenuItem( mnuEditors, ID_mnuEditorsAsteroidField, wxString( wxT("Asteroid Field") ) + wxT('\t') + wxT("Shift+A"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsAsteroidField );
	
	wxMenuItem* mnuEditorsMissionSpecs;
	mnuEditorsMissionSpecs = new wxMenuItem( mnuEditors, ID_mnuEditorsMissionSpecs, wxString( wxT("Mission Specs") ) + wxT('\t') + wxT("Shift+N"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsMissionSpecs );
	
	wxMenuItem* mnuEditorsBriefing;
	mnuEditorsBriefing = new wxMenuItem( mnuEditors, ID_mnuEditorsBriefing, wxString( wxT("Briefing") ) + wxT('\t') + wxT("Shift+B"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsBriefing );
	
	wxMenuItem* mnuEditorsDebriefing;
	mnuEditorsDebriefing = new wxMenuItem( mnuEditors, ID_mnuEditorsDebriefing, wxString( wxT("Debriefing") ) + wxT('\t') + wxT("Shift+D"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsDebriefing );
	
	wxMenuItem* mnuEditorsCommandBriefing;
	mnuEditorsCommandBriefing = new wxMenuItem( mnuEditors, ID_mnuEditorsCommandBriefing, wxString( wxT("Command Briefing") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsCommandBriefing );
	
	wxMenuItem* mnuEditorsFictionViewer;
	mnuEditorsFictionViewer = new wxMenuItem( mnuEditors, ID_mnuEditorsFictionViewer, wxString( wxT("Fiction Viewer") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsFictionViewer );
	
	wxMenuItem* mnuEditorsShieldSystem;
	mnuEditorsShieldSystem = new wxMenuItem( mnuEditors, ID_mnuEditorsShieldSystem, wxString( wxT("Shield System") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsShieldSystem );
	
	wxMenuItem* mnuEditorsSetGlobalShipFlags;
	mnuEditorsSetGlobalShipFlags = new wxMenuItem( mnuEditors, ID_mnuEditorsSetGlobalShipFlags, wxString( wxT("Set Global Ship Flags") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsSetGlobalShipFlags );
	
	wxMenuItem* mnuEditorsVoiceActingManager;
	mnuEditorsVoiceActingManager = new wxMenuItem( mnuEditors, ID_mnuEditorsVoiceActingManager, wxString( wxT("Voice Manager") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsVoiceActingManager );
	
	mnuEditors->AppendSeparator();
	
	wxMenuItem* mnuEditorsCampaign;
	mnuEditorsCampaign = new wxMenuItem( mnuEditors, ID_mnuEditorsCampaign, wxString( wxT("Campaign") ) + wxT('\t') + wxT("Shift+C"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsCampaign );
	
	mbrFRED->Append( mnuEditors, wxT("Editors") ); 
	
	mnuGroups = new wxMenu();
	wxMenuItem* mnuGroupsGroup1;
	mnuGroupsGroup1 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup1, wxString( wxT("Group 1") ) + wxT('\t') + wxT("Ctrl+1"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup1 );
	
	wxMenuItem* mnuGroupsGroup2;
	mnuGroupsGroup2 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup2, wxString( wxT("Group 2") ) + wxT('\t') + wxT("Ctrl+2"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup2 );
	
	wxMenuItem* mnuGroupsGroup3;
	mnuGroupsGroup3 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup3, wxString( wxT("Group 3") ) + wxT('\t') + wxT("Ctrl+3"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup3 );
	
	wxMenuItem* mnuGroupsGroup4;
	mnuGroupsGroup4 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup4, wxString( wxT("Group 4") ) + wxT('\t') + wxT("Ctrl+4"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup4 );
	
	wxMenuItem* mnuGroupsGroup5;
	mnuGroupsGroup5 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup5, wxString( wxT("Group 5") ) + wxT('\t') + wxT("Ctrl+5"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup5 );
	
	wxMenuItem* mnuGroupsGroup6;
	mnuGroupsGroup6 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup6, wxString( wxT("Group 6") ) + wxT('\t') + wxT("Ctrl+6"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup6 );
	
	wxMenuItem* mnuGroupsGroup7;
	mnuGroupsGroup7 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup7, wxString( wxT("Group 7") ) + wxT('\t') + wxT("Ctrl+7"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup7 );
	
	wxMenuItem* mnuGroupsGroup8;
	mnuGroupsGroup8 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup8, wxString( wxT("Group 8") ) + wxT('\t') + wxT("Ctrl+8"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup8 );
	
	wxMenuItem* mnuGroupsGroup9;
	mnuGroupsGroup9 = new wxMenuItem( mnuGroups, ID_mnuGroupsGroup9, wxString( wxT("Group 9") ) + wxT('\t') + wxT("Ctrl+9"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup9 );
	
	mnuGroupsSetGroup = new wxMenu();
	wxMenuItem* mnuGroupsSetGroupGroup1;
	mnuGroupsSetGroupGroup1 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup1, wxString( wxT("Group 1") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup1 );
	
	wxMenuItem* mnuGroupsSetGroupGroup2;
	mnuGroupsSetGroupGroup2 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup2, wxString( wxT("Group 2") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup2 );
	
	wxMenuItem* mnuGroupsSetGroupGroup3;
	mnuGroupsSetGroupGroup3 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup3, wxString( wxT("Group 3") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup3 );
	
	wxMenuItem* mnuGroupsSetGroupGroup4;
	mnuGroupsSetGroupGroup4 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup4, wxString( wxT("Group 4") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup4 );
	
	wxMenuItem* mnuGroupsSetGroupGroup5;
	mnuGroupsSetGroupGroup5 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup5, wxString( wxT("Group 5") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup5 );
	
	wxMenuItem* mnuGroupsSetGroupGroup6;
	mnuGroupsSetGroupGroup6 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup6, wxString( wxT("Group 6") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup6 );
	
	wxMenuItem* mnuGroupsSetGroupGroup7;
	mnuGroupsSetGroupGroup7 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup7, wxString( wxT("Group 7") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup7 );
	
	wxMenuItem* mnuGroupsSetGroupGroup8;
	mnuGroupsSetGroupGroup8 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup8, wxString( wxT("Group 8") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup8 );
	
	wxMenuItem* mnuGroupsSetGroupGroup9;
	mnuGroupsSetGroupGroup9 = new wxMenuItem( mnuGroupsSetGroup, ID_mnuGroupsSetGroupGroup9, wxString( wxT("Group 9") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup9 );
	
	mnuGroups->Append( -1, wxT("Set Group"), mnuGroupsSetGroup );
	
	mbrFRED->Append( mnuGroups, wxT("Groups") ); 
	
	mnuMisc = new wxMenu();
	wxMenuItem* mnuMiscLevelObject;
	mnuMiscLevelObject = new wxMenuItem( mnuMisc, ID_mnuMiscLevelObject, wxString( wxT("Level Object") ) + wxT('\t') + wxT("L"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscLevelObject );
	
	wxMenuItem* mnuMiscAlignObject;
	mnuMiscAlignObject = new wxMenuItem( mnuMisc, ID_mnuMiscAlignObject, wxString( wxT("Align Object") ) + wxT('\t') + wxT("Ctrl+L"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscAlignObject );
	
	wxMenuItem* mnuMiscMarkWing;
	mnuMiscMarkWing = new wxMenuItem( mnuMisc, ID_mnuMiscMarkWing, wxString( wxT("Mark Wing") ) + wxT('\t') + wxT("W"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscMarkWing );
	
	wxMenuItem* mnuMiscControlObject;
	mnuMiscControlObject = new wxMenuItem( mnuMisc, ID_mnuMiscControlObject, wxString( wxT("Control Object") ) + wxT('\t') + wxT("T"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscControlObject );
	
	wxMenuItem* mnuMiscNextObject;
	mnuMiscNextObject = new wxMenuItem( mnuMisc, ID_mnuMiscNextObject, wxString( wxT("Next Object") ) + wxT('\t') + wxT("Tab"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscNextObject );
	
	wxMenuItem* mnuMiscPreviousObject;
	mnuMiscPreviousObject = new wxMenuItem( mnuMisc, ID_mnuMiscPreviousObject, wxString( wxT("Prev Object") ) + wxT('\t') + wxT("Ctrl+Tab"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscPreviousObject );
	
	wxMenuItem* mnuMiscAdjustGrid;
	mnuMiscAdjustGrid = new wxMenuItem( mnuMisc, ID_mnuMiscAdjustGrid, wxString( wxT("Adjust Grid") ) , wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscAdjustGrid );
	
	wxMenuItem* mnuMiscNextSubsystem;
	mnuMiscNextSubsystem = new wxMenuItem( mnuMisc, ID_mnuMiscNextSubsystem, wxString( wxT("Next Subsystem") ) + wxT('\t') + wxT("K"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscNextSubsystem );
	
	wxMenuItem* mnuMiscPrevSubsystem;
	mnuMiscPrevSubsystem = new wxMenuItem( mnuMisc, ID_mnuMiscPrevSubsystem, wxString( wxT("Prev Subsystem") ) + wxT('\t') + wxT("Shift+K"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscPrevSubsystem );
	
	wxMenuItem* mnuMiscCancelSubsystem;
	mnuMiscCancelSubsystem = new wxMenuItem( mnuMisc, ID_mnuMiscCancelSubsystem, wxString( wxT("Cancel Subsystem") ) + wxT('\t') + wxT("Alt+K"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscCancelSubsystem );
	
	wxMenuItem* mnuMiscMissionStatistics;
	mnuMiscMissionStatistics = new wxMenuItem( mnuMisc, ID_mnuMiscMissionStatistics, wxString( wxT("Mission Statistics") ) + wxT('\t') + wxT("Ctrl+Shift+D"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscMissionStatistics );
	
	mnuMisc->AppendSeparator();
	
	wxMenuItem* mnuMiscErrorChecker;
	mnuMiscErrorChecker = new wxMenuItem( mnuMisc, ID_mnuMiscErrorChecker, wxString( wxT("Error Checker") ) + wxT('\t') + wxT("Shift+H"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscErrorChecker );
	
	mbrFRED->Append( mnuMisc, wxT("Misc") ); 
	
	mnuHelp = new wxMenu();
	wxMenuItem* mnuHelpHelpTopics;
	mnuHelpHelpTopics = new wxMenuItem( mnuHelp, ID_mnuHelpHelpTopics, wxString( wxT("Help Topics") ) + wxT('\t') + wxT("F1"), wxEmptyString, wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpHelpTopics );
	
	wxMenuItem* mnuHelpShowSexpHelp;
	mnuHelpShowSexpHelp = new wxMenuItem( mnuHelp, ID_mnuHelpShowSexpHelp, wxString( wxT("Show SEXP Help") ) , wxEmptyString, wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpShowSexpHelp );
	
	mnuHelp->AppendSeparator();
	
	wxMenuItem* mnuHelpAbout;
	mnuHelpAbout = new wxMenuItem( mnuHelp, ID_mnuHelpAbout, wxString( wxT("About wxFRED2...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpAbout );
	
	mbrFRED->Append( mnuHelp, wxT("Help") ); 
	
	this->SetMenuBar( mbrFRED );
	
	tbrFRED = this->CreateToolBar( wxTB_HORIZONTAL, wxID_ANY );
	tbrFRED->SetToolSeparation( 0 );
	tbrFRED->SetToolPacking( 0 );
	tbrFRED->AddTool( ID_optSelect, wxEmptyString, select_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Select (S)"), wxT("Select objects only."), NULL ); 
	
	tbrFRED->AddTool( ID_optSelectMove, wxEmptyString, selectmove_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Select and Move (M)"), wxT("Select and move selected objects."), NULL ); 
	
	tbrFRED->AddTool( ID_optSelectRotate, wxT("Select and Rotate"), selectrot_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Select and Rotate (R)"), wxT("Select and rotate selected objects."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->AddTool( ID_chkRotateLocally, wxEmptyString, rotlocal_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Rotate Locally (X)"), wxT("Enable/disable local rotation for the selected group."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->AddTool( ID_optConstraintX, wxEmptyString, constx_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("X Constraint (`)"), wxT("Constrain actions to the global X axis."), NULL ); 
	
	tbrFRED->AddTool( ID_optConstraintY, wxEmptyString, consty_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Y Constraint (`)"), wxT("Constrain actions to the global Y axis."), NULL ); 
	
	tbrFRED->AddTool( ID_optConstraintZ, wxEmptyString, constz_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Z Constraint (`)"), wxT("Constrain actions to the global Z axis."), NULL ); 
	
	tbrFRED->AddTool( ID_optConstraintXZ, wxEmptyString, constxz_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("XZ Constraint (`)"), wxT("Constrain actions to the global XZ plane."), NULL ); 
	
	tbrFRED->AddTool( ID_optConstraintYZ, wxEmptyString, constyz_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("YZ Constraint (`)"), wxT("Constrain actions to the global YZ plane."), NULL ); 
	
	tbrFRED->AddTool( ID_optConstraintXY, wxEmptyString, constxy_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("XY Constraint (`)"), wxT("Constrain actions to the global XY plane."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->AddTool( ID_btnSelectionList, wxEmptyString, selectlist_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Selection List (H)"), wxT("Select object(s) from a list."), NULL ); 
	
	tbrFRED->AddTool( ID_chkSelectionLock, wxEmptyString, selectlock_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Selection Lock (L)"), wxT("Lock the current selection from changes."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->AddTool( ID_btnWingForm, wxEmptyString, wingform_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Form Wing (Ctrl+W)"), wxT("Adds the current selection to a wing."), NULL ); 
	
	tbrFRED->AddTool( wxID_ANY, wxEmptyString, wingdisband_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Disband Wing (Ctrl+D)"), wxT("Removes the current selection from (any) wing."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->AddTool( ID_btnZoomSelected, wxT("tool"), zoomsel_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Zoom Selected (Alt+Z)"), wxT("Zoom to view current selection."), NULL ); 
	
	tbrFRED->AddTool( ID_btnZoomExtents, wxT("tool"), zoomext_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Zoom Extents (Shift+Z)"), wxT("Zoom to view all objects in the mission."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->AddTool( ID_chkShowDistances, wxEmptyString, showdist_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Show Distances (D)"), wxT("Show the distances between all selected objects."), NULL ); 
	
	tbrFRED->AddTool( ID_chkOrbitSelected, wxT("tool"), orbitsel_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Rotate about Selection (Ctrl+V)"), wxT("Rotate the vieport camera about the current selection."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->Realize(); 
	
	
	this->Centre( wxBOTH );
}

frmFRED::~frmFRED()
{
}

BEGIN_EVENT_TABLE( frmShipsEditor, wxFrame )
	EVT_CLOSE( frmShipsEditor::_wxFB_OnClose )
END_EVENT_TABLE()

frmShipsEditor::frmShipsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 485,600 ), wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	mbrShipsEditor = new wxMenuBar( 0 );
	selectShip = new wxMenu();
	mbrShipsEditor->Append( selectShip, wxT("Select Ship") ); 
	
	this->SetMenuBar( mbrShipsEditor );
	
	wxBoxSizer* bSizer93;
	bSizer93 = new wxBoxSizer( wxVERTICAL );
	
	wxGridBagSizer* gbSizer4;
	gbSizer4 = new wxGridBagSizer( 0, 0 );
	gbSizer4->SetFlexibleDirection( wxBOTH );
	gbSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblShipName = new wxStaticText( this, wxID_ANY, wxT("Ship Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipName->Wrap( -1 );
	gbSizer4->Add( lblShipName, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtShipName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtShipName->SetMaxLength( 0 ); 
	gbSizer4->Add( txtShipName, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblWing = new wxStaticText( this, wxID_ANY, wxT("Wing:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWing->Wrap( -1 );
	gbSizer4->Add( lblWing, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtWing = new wxTextCtrl( this, wxID_ANY, wxT("None"), wxDefaultPosition, wxDefaultSize, 0 );
	txtWing->Enable( false );
	
	gbSizer4->Add( txtWing, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblShipClass = new wxStaticText( this, wxID_ANY, wxT("Ship Class:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipClass->Wrap( -1 );
	gbSizer4->Add( lblShipClass, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboShipClassChoices;
	cboShipClass = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboShipClassChoices, 0 );
	cboShipClass->SetSelection( 0 );
	gbSizer4->Add( cboShipClass, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblHotkey = new wxStaticText( this, wxID_ANY, wxT("Hotkey:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHotkey->Wrap( -1 );
	gbSizer4->Add( lblHotkey, wxGBPosition( 1, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboHotkeyChoices;
	cboHotkey = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboHotkeyChoices, 0 );
	cboHotkey->SetSelection( 0 );
	gbSizer4->Add( cboHotkey, wxGBPosition( 1, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblAIClass = new wxStaticText( this, wxID_ANY, wxT("AI Class:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAIClass->Wrap( -1 );
	gbSizer4->Add( lblAIClass, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboAIClassChoices;
	cboAIClass = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboAIClassChoices, 0 );
	cboAIClass->SetSelection( 0 );
	gbSizer4->Add( cboAIClass, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblPersona = new wxStaticText( this, wxID_ANY, wxT("Persona:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPersona->Wrap( -1 );
	gbSizer4->Add( lblPersona, wxGBPosition( 2, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboPersonaChoices;
	cboPersona = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboPersonaChoices, 0 );
	cboPersona->SetSelection( 0 );
	gbSizer4->Add( cboPersona, wxGBPosition( 2, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblTeam = new wxStaticText( this, wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTeam->Wrap( -1 );
	gbSizer4->Add( lblTeam, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboTeamChoices;
	cboTeam = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboTeamChoices, 0 );
	cboTeam->SetSelection( 0 );
	gbSizer4->Add( cboTeam, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblKillScore = new wxStaticText( this, wxID_ANY, wxT("Kill Score:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblKillScore->Wrap( -1 );
	gbSizer4->Add( lblKillScore, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtKillscore = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtKillscore->SetMaxLength( 0 ); 
	gbSizer4->Add( txtKillscore, wxGBPosition( 3, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblCargoCargo = new wxStaticText( this, wxID_ANY, wxT("Cargo:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCargoCargo->Wrap( -1 );
	gbSizer4->Add( lblCargoCargo, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboCargo = new wxComboBox( this, wxID_ANY, wxT("None"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer4->Add( cboCargo, wxGBPosition( 4, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblAssistPercentage = new wxStaticText( this, wxID_ANY, wxT("Assist %:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAssistPercentage->Wrap( -1 );
	gbSizer4->Add( lblAssistPercentage, wxGBPosition( 4, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAssistPercentage = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAssistPercentage->SetMaxLength( 0 ); 
	gbSizer4->Add( txtAssistPercentage, wxGBPosition( 4, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblAltName = new wxStaticText( this, wxID_ANY, wxT("Alt Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAltName->Wrap( -1 );
	gbSizer4->Add( lblAltName, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboAltName = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer4->Add( cboAltName, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	chkPlayerShip = new wxCheckBox( this, wxID_ANY, wxT("Player Ship"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPlayerShip->Enable( false );
	
	gbSizer4->Add( chkPlayerShip, wxGBPosition( 5, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblCallsign = new wxStaticText( this, wxID_ANY, wxT("Callsign:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCallsign->Wrap( -1 );
	gbSizer4->Add( lblCallsign, wxGBPosition( 6, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboCallsign = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer4->Add( cboCallsign, wxGBPosition( 6, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnMakePlayerShip = new wxButton( this, wxID_ANY, wxT("Set As Player Ship"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnMakePlayerShip, wxGBPosition( 6, 3 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnTextureReplacement = new wxButton( this, wxID_ANY, wxT("Texture Replacement"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnTextureReplacement, wxGBPosition( 7, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnAltShipClass = new wxButton( this, wxID_ANY, wxT("Alt Ship Class"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnAltShipClass, wxGBPosition( 7, 3 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxHORIZONTAL );
	
	btnPrevWing = new wxButton( this, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer92->Add( btnPrevWing, 0, wxALL|wxEXPAND, 3 );
	
	btnNextWing = new wxButton( this, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer92->Add( btnNextWing, 0, wxALL|wxEXPAND, 3 );
	
	
	gbSizer4->Add( bSizer92, wxGBPosition( 0, 4 ), wxGBSpan( 1, 1 ), wxEXPAND, 3 );
	
	btnDelete = new wxButton( this, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnDelete, wxGBPosition( 1, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnReset = new wxButton( this, wxID_ANY, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnReset, wxGBPosition( 2, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnWeapons = new wxButton( this, wxID_ANY, wxT("Weapons"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnWeapons, wxGBPosition( 3, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnPlayerOrders = new wxButton( this, wxID_ANY, wxT("Player Orders"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnPlayerOrders, wxGBPosition( 4, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnSpecialExplosion = new wxButton( this, wxID_ANY, wxT("Special Exp"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnSpecialExplosion, wxGBPosition( 5, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnSpecialHits = new wxButton( this, wxID_ANY, wxT("Special Hits"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer4->Add( btnSpecialHits, wxGBPosition( 6, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	
	bSizer93->Add( gbSizer4, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer95;
	bSizer95 = new wxBoxSizer( wxHORIZONTAL );
	
	btnMiscOptions = new wxButton( this, wxID_ANY, wxT("Misc"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer95->Add( btnMiscOptions, 1, wxALL, 3 );
	
	btnInitialStatus = new wxButton( this, wxID_ANY, wxT("Initial Status"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer95->Add( btnInitialStatus, 1, wxALL, 3 );
	
	btnInitialOrders = new wxButton( this, wxID_ANY, wxT("Initial Orders"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer95->Add( btnInitialOrders, 1, wxALL, 3 );
	
	btnTBLInfo = new wxButton( this, wxID_ANY, wxT("TBL Info"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer95->Add( btnTBLInfo, 1, wxALL, 3 );
	
	btnHideCues = new wxButton( this, wxID_ANY, wxT("Hide Cues"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer95->Add( btnHideCues, 1, wxALL, 3 );
	
	
	bSizer93->Add( bSizer95, 0, wxALIGN_CENTER|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer96;
	bSizer96 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer53;
	sbSizer53 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Arrival") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer39;
	fgSizer39 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer39->SetFlexibleDirection( wxBOTH );
	fgSizer39->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblArrivalLocation = new wxStaticText( this, wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalLocation->Wrap( -1 );
	fgSizer39->Add( lblArrivalLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalLocationChoices;
	cboArrivalLocation = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboArrivalLocationChoices, 0 );
	cboArrivalLocation->SetSelection( 0 );
	fgSizer39->Add( cboArrivalLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalTarget = new wxStaticText( this, wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalTarget->Wrap( -1 );
	fgSizer39->Add( lblArrivalTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalTargetChoices;
	cboArrivalTarget = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboArrivalTargetChoices, 0 );
	cboArrivalTarget->SetSelection( 0 );
	fgSizer39->Add( cboArrivalTarget, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDistance = new wxStaticText( this, wxID_ANY, wxT("Distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDistance->Wrap( -1 );
	fgSizer39->Add( lblArrivalDistance, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtArrivalDistance = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,21 ), 0 );
	txtArrivalDistance->SetMaxLength( 0 ); 
	fgSizer39->Add( txtArrivalDistance, 0, wxALL, 3 );
	
	lblArrivalDelay = new wxStaticText( this, wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelay->Wrap( -1 );
	fgSizer39->Add( lblArrivalDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer98;
	bSizer98 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,21 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer98->Add( spnArrivalDelay, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDelaySeconds = new wxStaticText( this, wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelaySeconds->Wrap( -1 );
	bSizer98->Add( lblArrivalDelaySeconds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer39->Add( bSizer98, 1, wxEXPAND, 5 );
	
	
	sbSizer53->Add( fgSizer39, 0, wxALL|wxEXPAND, 3 );
	
	btnRestrictArrivalPaths = new wxButton( this, wxID_ANY, wxT("Restrict Arrival Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer53->Add( btnRestrictArrivalPaths, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalCue = new wxStaticText( this, wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalCue->Wrap( -1 );
	sbSizer53->Add( lblArrivalCue, 0, wxALL, 3 );
	
	tctArrivalCues = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE );
	sbSizer53->Add( tctArrivalCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoArrivalWarp = new wxCheckBox( this, wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer53->Add( chkNoArrivalWarp, 0, wxALL, 5 );
	
	
	bSizer96->Add( sbSizer53, 1, wxEXPAND|wxRIGHT|wxTOP, 3 );
	
	wxStaticBoxSizer* sbSizer54;
	sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Departure") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer391;
	fgSizer391 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer391->SetFlexibleDirection( wxBOTH );
	fgSizer391->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDepatureLocation = new wxStaticText( this, wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepatureLocation->Wrap( -1 );
	fgSizer391->Add( lblDepatureLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureLocationChoices;
	cboDepartureLocation = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboDepartureLocationChoices, 0 );
	cboDepartureLocation->SetSelection( 0 );
	fgSizer391->Add( cboDepartureLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureTarget = new wxStaticText( this, wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureTarget->Wrap( -1 );
	fgSizer391->Add( lblDepartureTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureTargetChoices;
	cboDepartureTarget = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboDepartureTargetChoices, 0 );
	cboDepartureTarget->SetSelection( 0 );
	fgSizer391->Add( cboDepartureTarget, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer391->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer391->Add( 0, 27, 1, wxEXPAND, 5 );
	
	lblDepartureDelay = new wxStaticText( this, wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureDelay->Wrap( -1 );
	fgSizer391->Add( lblDepartureDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer981;
	bSizer981 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay1 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,21 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer981->Add( spnArrivalDelay1, 0, wxALL|wxEXPAND, 3 );
	
	m_staticText1711 = new wxStaticText( this, wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1711->Wrap( -1 );
	bSizer981->Add( m_staticText1711, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer391->Add( bSizer981, 0, wxALL, 3 );
	
	
	sbSizer54->Add( fgSizer391, 0, wxEXPAND, 5 );
	
	btnRestrictDeparturePaths = new wxButton( this, wxID_ANY, wxT("Restrict Departure Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer54->Add( btnRestrictDeparturePaths, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureCue = new wxStaticText( this, wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureCue->Wrap( -1 );
	sbSizer54->Add( lblDepartureCue, 0, wxALL, 3 );
	
	tctDepartureCues = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE );
	sbSizer54->Add( tctDepartureCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoDepartureWarp = new wxCheckBox( this, wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer54->Add( chkNoDepartureWarp, 0, wxALL, 5 );
	
	
	bSizer96->Add( sbSizer54, 1, wxEXPAND|wxLEFT|wxTOP, 3 );
	
	
	bSizer93->Add( bSizer96, 1, wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer93 );
	this->Layout();
	bSizer93->Fit( this );
	
	this->Centre( wxBOTH );
}

frmShipsEditor::~frmShipsEditor()
{
}

BEGIN_EVENT_TABLE( frmWingEditor, wxFrame )
	EVT_CLOSE( frmWingEditor::_wxFB_OnClose )
END_EVENT_TABLE()

frmWingEditor::frmWingEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxVERTICAL );
	
	pnlProperties = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxGridBagSizer* gbSizer5;
	gbSizer5 = new wxGridBagSizer( 0, 0 );
	gbSizer5->SetFlexibleDirection( wxBOTH );
	gbSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblWingName = new wxStaticText( pnlProperties, wxID_ANY, wxT("Wing Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWingName->Wrap( -1 );
	gbSizer5->Add( lblWingName, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	txtWingName = new wxTextCtrl( pnlProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtWingName->SetMaxLength( 0 ); 
	gbSizer5->Add( txtWingName, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblLeader = new wxStaticText( pnlProperties, wxID_ANY, wxT("Leader:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLeader->Wrap( -1 );
	gbSizer5->Add( lblLeader, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	wxArrayString cboWingLeaderChoices;
	cboWingLeader = new wxChoice( pnlProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboWingLeaderChoices, 0 );
	cboWingLeader->SetSelection( 0 );
	gbSizer5->Add( cboWingLeader, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblWaveNumber = new wxStaticText( pnlProperties, wxID_ANY, wxT("# of Waves"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWaveNumber->Wrap( -1 );
	gbSizer5->Add( lblWaveNumber, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnWaveNumber = new wxSpinCtrl( pnlProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
	gbSizer5->Add( spnWaveNumber, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblWaveThreshold = new wxStaticText( pnlProperties, wxID_ANY, wxT("Wave Threshold:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWaveThreshold->Wrap( -1 );
	gbSizer5->Add( lblWaveThreshold, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnWaveThreshold = new wxSpinCtrl( pnlProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer5->Add( spnWaveThreshold, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblHotkey = new wxStaticText( pnlProperties, wxID_ANY, wxT("Hotkey:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHotkey->Wrap( -1 );
	gbSizer5->Add( lblHotkey, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	wxArrayString cboHotkeyChoices;
	cboHotkey = new wxChoice( pnlProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboHotkeyChoices, 0 );
	cboHotkey->SetSelection( 0 );
	gbSizer5->Add( cboHotkey, wxGBPosition( 4, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	btnSquadLogo = new wxButton( pnlProperties, wxID_ANY, wxT("Squad Logo"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer5->Add( btnSquadLogo, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 3 );
	
	txtSquadLogo = new wxTextCtrl( pnlProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSquadLogo->SetMaxLength( 0 ); 
	gbSizer5->Add( txtSquadLogo, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 3 );
	
	wxBoxSizer* bSizer104;
	bSizer104 = new wxBoxSizer( wxVERTICAL );
	
	chkReinforcement = new wxCheckBox( pnlProperties, wxID_ANY, wxT("Reinforcement Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer104->Add( chkReinforcement, 0, wxALL, 3 );
	
	chkIgnoreForGoals = new wxCheckBox( pnlProperties, wxID_ANY, wxT("Ignore for counting Goals"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer104->Add( chkIgnoreForGoals, 0, wxALL, 3 );
	
	chkNoArrivalMusic = new wxCheckBox( pnlProperties, wxID_ANY, wxT("No Arrival Music"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer104->Add( chkNoArrivalMusic, 0, wxALL, 3 );
	
	chkNoArrivalMessage = new wxCheckBox( pnlProperties, wxID_ANY, wxT("No Arrival Message"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer104->Add( chkNoArrivalMessage, 0, wxALL, 3 );
	
	chkNoDynamicGoals = new wxCheckBox( pnlProperties, wxID_ANY, wxT("No Dynamic Goals"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer104->Add( chkNoDynamicGoals, 0, wxALL, 3 );
	
	
	gbSizer5->Add( bSizer104, wxGBPosition( 0, 2 ), wxGBSpan( 5, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxHORIZONTAL );
	
	btnPrev = new wxButton( pnlProperties, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer106->Add( btnPrev, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnNext = new wxButton( pnlProperties, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer106->Add( btnNext, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	
	gbSizer5->Add( bSizer106, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnDeleteWing = new wxButton( pnlProperties, wxID_ANY, wxT("Delete Wing"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer5->Add( btnDeleteWing, wxGBPosition( 1, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnDisbandWing = new wxButton( pnlProperties, wxID_ANY, wxT("Disband Wing"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer5->Add( btnDisbandWing, wxGBPosition( 2, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnInitialOrders = new wxButton( pnlProperties, ID_btnInitialOrders, wxT("Initial Orders"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer5->Add( btnInitialOrders, wxGBPosition( 3, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnHideCues = new wxToggleButton( pnlProperties, ID_btnHideCues, wxT("Hide Cues"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer5->Add( btnHideCues, wxGBPosition( 4, 3 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	
	gbSizer5->AddGrowableCol( 1 );
	
	pnlProperties->SetSizer( gbSizer5 );
	pnlProperties->Layout();
	gbSizer5->Fit( pnlProperties );
	bSizer101->Add( pnlProperties, 0, wxALL|wxEXPAND, 3 );
	
	pnlCues = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer982;
	bSizer982 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer48;
	sbSizer48 = new wxStaticBoxSizer( new wxStaticBox( pnlCues, wxID_ANY, wxT("Delay Between Waves (Seconds)") ), wxHORIZONTAL );
	
	lblMinWaveDelay = new wxStaticText( pnlCues, wxID_ANY, wxT("Min:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMinWaveDelay->Wrap( -1 );
	sbSizer48->Add( lblMinWaveDelay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnMinWaveDelay = new wxSpinCtrl( pnlCues, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	sbSizer48->Add( spnMinWaveDelay, 1, wxALL, 3 );
	
	lblMaxWaveDelay = new wxStaticText( pnlCues, wxID_ANY, wxT("Max:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMaxWaveDelay->Wrap( -1 );
	sbSizer48->Add( lblMaxWaveDelay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnMaxWaveDelay = new wxSpinCtrl( pnlCues, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	sbSizer48->Add( spnMaxWaveDelay, 1, wxALL, 3 );
	
	
	bSizer982->Add( sbSizer48, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer96;
	bSizer96 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer53;
	sbSizer53 = new wxStaticBoxSizer( new wxStaticBox( pnlCues, wxID_ANY, wxT("Arrival") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer39;
	fgSizer39 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer39->SetFlexibleDirection( wxBOTH );
	fgSizer39->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblArrivalLocation = new wxStaticText( pnlCues, wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalLocation->Wrap( -1 );
	fgSizer39->Add( lblArrivalLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalLocationChoices;
	cboArrivalLocation = new wxChoice( pnlCues, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboArrivalLocationChoices, 0 );
	cboArrivalLocation->SetSelection( 0 );
	fgSizer39->Add( cboArrivalLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalTarget = new wxStaticText( pnlCues, wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalTarget->Wrap( -1 );
	fgSizer39->Add( lblArrivalTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalTargetChoices;
	cboArrivalTarget = new wxChoice( pnlCues, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboArrivalTargetChoices, 0 );
	cboArrivalTarget->SetSelection( 0 );
	fgSizer39->Add( cboArrivalTarget, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDistance = new wxStaticText( pnlCues, wxID_ANY, wxT("Distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDistance->Wrap( -1 );
	fgSizer39->Add( lblArrivalDistance, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtArrivalDistance = new wxTextCtrl( pnlCues, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtArrivalDistance->SetMaxLength( 0 ); 
	fgSizer39->Add( txtArrivalDistance, 0, wxALL, 3 );
	
	lblArrivalDelay = new wxStaticText( pnlCues, wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelay->Wrap( -1 );
	fgSizer39->Add( lblArrivalDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer98;
	bSizer98 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay = new wxSpinCtrl( pnlCues, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer98->Add( spnArrivalDelay, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDelaySeconds = new wxStaticText( pnlCues, wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelaySeconds->Wrap( -1 );
	bSizer98->Add( lblArrivalDelaySeconds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer39->Add( bSizer98, 1, wxEXPAND, 5 );
	
	
	sbSizer53->Add( fgSizer39, 0, wxALL|wxEXPAND, 3 );
	
	btnRestrictArrivalPaths = new wxButton( pnlCues, wxID_ANY, wxT("Restrict Arrival Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer53->Add( btnRestrictArrivalPaths, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalCue = new wxStaticText( pnlCues, wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalCue->Wrap( -1 );
	sbSizer53->Add( lblArrivalCue, 0, wxALL, 3 );
	
	tctArrivalCues = new wxTreeCtrl( pnlCues, wxID_ANY, wxDefaultPosition, wxSize( -1,106 ), wxTR_DEFAULT_STYLE );
	sbSizer53->Add( tctArrivalCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoArrivalWarp = new wxCheckBox( pnlCues, wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer53->Add( chkNoArrivalWarp, 0, wxALL, 5 );
	
	
	bSizer96->Add( sbSizer53, 1, wxEXPAND|wxRIGHT|wxTOP, 3 );
	
	wxStaticBoxSizer* sbSizer54;
	sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( pnlCues, wxID_ANY, wxT("Departure") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer391;
	fgSizer391 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer391->SetFlexibleDirection( wxBOTH );
	fgSizer391->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDepatureLocation = new wxStaticText( pnlCues, wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepatureLocation->Wrap( -1 );
	fgSizer391->Add( lblDepatureLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureLocationChoices;
	cboDepartureLocation = new wxChoice( pnlCues, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboDepartureLocationChoices, 0 );
	cboDepartureLocation->SetSelection( 0 );
	fgSizer391->Add( cboDepartureLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureTarget = new wxStaticText( pnlCues, wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureTarget->Wrap( -1 );
	fgSizer391->Add( lblDepartureTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureTargetChoices;
	cboDepartureTarget = new wxChoice( pnlCues, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboDepartureTargetChoices, 0 );
	cboDepartureTarget->SetSelection( 0 );
	fgSizer391->Add( cboDepartureTarget, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer391->Add( 0, 27, 0, 0, 5 );
	
	
	fgSizer391->Add( 0, 0, 0, 0, 5 );
	
	lblDepartureDelay = new wxStaticText( pnlCues, wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureDelay->Wrap( -1 );
	fgSizer391->Add( lblDepartureDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer981;
	bSizer981 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay1 = new wxSpinCtrl( pnlCues, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer981->Add( spnArrivalDelay1, 0, wxALL|wxEXPAND, 3 );
	
	m_staticText1711 = new wxStaticText( pnlCues, wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1711->Wrap( -1 );
	bSizer981->Add( m_staticText1711, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer391->Add( bSizer981, 0, wxEXPAND, 3 );
	
	
	sbSizer54->Add( fgSizer391, 0, wxALL|wxEXPAND, 3 );
	
	btnRestrictDeparturePaths = new wxButton( pnlCues, wxID_ANY, wxT("Restrict Departure Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer54->Add( btnRestrictDeparturePaths, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureCue = new wxStaticText( pnlCues, wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureCue->Wrap( -1 );
	sbSizer54->Add( lblDepartureCue, 0, wxALL, 3 );
	
	tctDepartureCues = new wxTreeCtrl( pnlCues, wxID_ANY, wxDefaultPosition, wxSize( -1,106 ), wxTR_DEFAULT_STYLE );
	sbSizer54->Add( tctDepartureCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoDepartureWarp = new wxCheckBox( pnlCues, wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer54->Add( chkNoDepartureWarp, 0, wxALL, 5 );
	
	
	bSizer96->Add( sbSizer54, 1, wxEXPAND|wxLEFT|wxTOP, 3 );
	
	
	bSizer982->Add( bSizer96, 1, wxEXPAND, 5 );
	
	
	pnlCues->SetSizer( bSizer982 );
	pnlCues->Layout();
	bSizer982->Fit( pnlCues );
	bSizer101->Add( pnlCues, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer101 );
	this->Layout();
	bSizer101->Fit( this );
	mbrWingEditor = new wxMenuBar( 0 );
	mnuSelectWing = new wxMenu();
	mbrWingEditor->Append( mnuSelectWing, wxT("Select Wing") ); 
	
	this->SetMenuBar( mbrWingEditor );
	
	
	this->Centre( wxBOTH );
}

frmWingEditor::~frmWingEditor()
{
}

BEGIN_EVENT_TABLE( dlgObjectEditor, wxDialog )
	EVT_CLOSE( dlgObjectEditor::_wxFB_OnClose )
	EVT_CHECKBOX( wxID_ANY, dlgObjectEditor::_wxFB_OnFace )
	EVT_RADIOBUTTON( wxID_ANY, dlgObjectEditor::_wxFB_OnOrientationOpt )
	EVT_RADIOBUTTON( wxID_ANY, dlgObjectEditor::_wxFB_OnOrientationOpt )
	EVT_RADIOBUTTON( wxID_ANY, dlgObjectEditor::_wxFB_OnOrientationOpt )
	EVT_BUTTON( wxID_CANCEL, dlgObjectEditor::_wxFB_OnCancel )
	EVT_BUTTON( wxID_OK, dlgObjectEditor::_wxFB_OnOK )
END_EVENT_TABLE()

dlgObjectEditor::dlgObjectEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer115;
	bSizer115 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer64;
	sbSizer64 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Position") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer50;
	fgSizer50 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer50->SetFlexibleDirection( wxBOTH );
	fgSizer50->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText217 = new wxStaticText( this, wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText217->Wrap( -1 );
	fgSizer50->Add( m_staticText217, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPositionX = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer50->Add( spnPositionX, 0, wxALL, 3 );
	
	m_staticText218 = new wxStaticText( this, wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText218->Wrap( -1 );
	fgSizer50->Add( m_staticText218, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPositionY = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer50->Add( spnPositionY, 0, wxALL, 3 );
	
	m_staticText220 = new wxStaticText( this, wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText220->Wrap( -1 );
	fgSizer50->Add( m_staticText220, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPositionZ = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer50->Add( spnPositionZ, 0, wxALL, 3 );
	
	
	sbSizer64->Add( fgSizer50, 0, wxEXPAND, 5 );
	
	
	bSizer115->Add( sbSizer64, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer54;
	sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Orientation") ), wxVERTICAL );
	
	chkPointTo = new wxCheckBox( this, wxID_ANY, wxT("Face/Point towards..."), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer54->Add( chkPointTo, 0, wxALL, 3 );
	
	pnlOrientation = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxGridBagSizer* gbSizer8;
	gbSizer8 = new wxGridBagSizer( 0, 0 );
	gbSizer8->SetFlexibleDirection( wxBOTH );
	gbSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	optObject = new wxRadioButton( pnlOrientation, wxID_ANY, wxT("Object:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optObject->SetValue( true ); 
	gbSizer8->Add( optObject, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	wxString cbObjectChoices[] = { wxT("No object") };
	int cbObjectNChoices = sizeof( cbObjectChoices ) / sizeof( wxString );
	cbObject = new wxChoice( pnlOrientation, wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cbObjectNChoices, cbObjectChoices, 0 );
	cbObject->SetSelection( 0 );
	gbSizer8->Add( cbObject, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_RIGHT|wxALL, 3 );
	
	optLocation = new wxRadioButton( pnlOrientation, wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer8->Add( optLocation, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer54;
	fgSizer54 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer54->SetFlexibleDirection( wxBOTH );
	fgSizer54->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblLocationX = new wxStaticText( pnlOrientation, wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLocationX->Wrap( -1 );
	lblLocationX->Enable( false );
	
	fgSizer54->Add( lblLocationX, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnLocationX = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	spnLocationX->Enable( false );
	
	fgSizer54->Add( spnLocationX, 0, wxALL, 3 );
	
	lblLocationY = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLocationY->Wrap( -1 );
	lblLocationY->Enable( false );
	
	fgSizer54->Add( lblLocationY, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnLocationY = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	spnLocationY->Enable( false );
	
	fgSizer54->Add( spnLocationY, 0, wxALL, 3 );
	
	lblLocationZ = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLocationZ->Wrap( -1 );
	lblLocationZ->Enable( false );
	
	fgSizer54->Add( lblLocationZ, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnLocationZ = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	spnLocationZ->Enable( false );
	
	fgSizer54->Add( spnLocationZ, 0, wxALL, 3 );
	
	
	gbSizer8->Add( fgSizer54, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_RIGHT|wxALL, 3 );
	
	optAngle = new wxRadioButton( pnlOrientation, wxID_ANY, wxT("Angle:"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer8->Add( optAngle, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer55;
	fgSizer55 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer55->SetFlexibleDirection( wxBOTH );
	fgSizer55->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblPitch = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPitch->Wrap( -1 );
	lblPitch->Enable( false );
	
	fgSizer55->Add( lblPitch, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPitch = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 360, 0 );
	spnPitch->Enable( false );
	
	fgSizer55->Add( spnPitch, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblBank = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBank->Wrap( -1 );
	lblBank->Enable( false );
	
	fgSizer55->Add( lblBank, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnBank = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 360, 0 );
	spnBank->Enable( false );
	
	fgSizer55->Add( spnBank, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblHeading = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHeading->Wrap( -1 );
	lblHeading->Enable( false );
	
	fgSizer55->Add( lblHeading, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnHeading = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	spnHeading->Enable( false );
	
	fgSizer55->Add( spnHeading, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	gbSizer8->Add( fgSizer55, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	
	pnlOrientation->SetSizer( gbSizer8 );
	pnlOrientation->Layout();
	gbSizer8->Fit( pnlOrientation );
	sbSizer54->Add( pnlOrientation, 0, wxEXPAND, 3 );
	
	
	bSizer115->Add( sbSizer54, 1, wxALL|wxEXPAND, 3 );
	
	sdbObjectEditor = new wxStdDialogButtonSizer();
	sdbObjectEditorOK = new wxButton( this, wxID_OK );
	sdbObjectEditor->AddButton( sdbObjectEditorOK );
	sdbObjectEditorCancel = new wxButton( this, wxID_CANCEL );
	sdbObjectEditor->AddButton( sdbObjectEditorCancel );
	sdbObjectEditor->Realize();
	
	bSizer115->Add( sdbObjectEditor, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	this->SetSizer( bSizer115 );
	this->Layout();
	bSizer115->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgObjectEditor::~dlgObjectEditor()
{
}

BEGIN_EVENT_TABLE( frmWaypointEditor, wxFrame )
	EVT_CLOSE( frmWaypointEditor::_wxFB_OnClose )
END_EVENT_TABLE()

frmWaypointEditor::frmWaypointEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	menuWaypoint = new wxMenuBar( 0 );
	menuPaths = new wxMenu();
	menuWaypoint->Append( menuPaths, wxT("Select Waypoint Path") ); 
	
	this->SetMenuBar( menuWaypoint );
	
	wxBoxSizer* bSizer78;
	bSizer78 = new wxBoxSizer( wxHORIZONTAL );
	
	lblWaypointName = new wxStaticText( this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWaypointName->Wrap( -1 );
	bSizer78->Add( lblWaypointName, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtWaypointName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtWaypointName->SetMaxLength( 0 ); 
	bSizer78->Add( txtWaypointName, 1, wxALIGN_CENTER|wxALL, 3 );
	
	
	this->SetSizer( bSizer78 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

frmWaypointEditor::~frmWaypointEditor()
{
}

BEGIN_EVENT_TABLE( dlgMissionObjectivesEditor, wxDialog )
	EVT_CLOSE( dlgMissionObjectivesEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgMissionObjectivesEditor::dlgMissionObjectivesEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( 1,-1 ) );
	
	wxBoxSizer* bSizer64;
	bSizer64 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer69;
	bSizer69 = new wxBoxSizer( wxVERTICAL );
	
	tctObjectives = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE );
	bSizer69->Add( tctObjectives, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer64->Add( bSizer69, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer65;
	bSizer65 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer66;
	bSizer66 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText117 = new wxStaticText( this, wxID_ANY, wxT("Display Types"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText117->Wrap( -1 );
	bSizer66->Add( m_staticText117, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString m_choice27Choices[] = { wxT("Primary Goals"), wxT("Secondary Goals"), wxT("Bonus Goals") };
	int m_choice27NChoices = sizeof( m_choice27Choices ) / sizeof( wxString );
	m_choice27 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice27NChoices, m_choice27Choices, 0 );
	m_choice27->SetSelection( 0 );
	bSizer66->Add( m_choice27, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer65->Add( bSizer66, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer33;
	sbSizer33 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Current Objective") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer27;
	fgSizer27 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer27->AddGrowableCol( 1 );
	fgSizer27->SetFlexibleDirection( wxBOTH );
	fgSizer27->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblObjType = new wxStaticText( this, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjType->Wrap( -1 );
	fgSizer27->Add( lblObjType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboObjTypeChoices[] = { wxT("Primary Goal"), wxT("Secondary Goal"), wxT("Bonus Goal") };
	int cboObjTypeNChoices = sizeof( cboObjTypeChoices ) / sizeof( wxString );
	cboObjType = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjTypeNChoices, cboObjTypeChoices, 0 );
	cboObjType->SetSelection( 0 );
	fgSizer27->Add( cboObjType, 0, wxALL|wxEXPAND, 3 );
	
	lblObjName = new wxStaticText( this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjName->Wrap( -1 );
	fgSizer27->Add( lblObjName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtObjName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtObjName->SetMaxLength( 0 ); 
	fgSizer27->Add( txtObjName, 0, wxALL|wxEXPAND, 3 );
	
	lblObjText = new wxStaticText( this, wxID_ANY, wxT("Objective Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjText->Wrap( -1 );
	fgSizer27->Add( lblObjText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtObjText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtObjText->SetMaxLength( 0 ); 
	fgSizer27->Add( txtObjText, 0, wxALL|wxEXPAND, 3 );
	
	lblObjScore = new wxStaticText( this, wxID_ANY, wxT("Score:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjScore->Wrap( -1 );
	fgSizer27->Add( lblObjScore, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtObjScore = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtObjScore->SetMaxLength( 0 ); 
	fgSizer27->Add( txtObjScore, 0, wxALL|wxEXPAND, 3 );
	
	lblObjTeam = new wxStaticText( this, wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjTeam->Wrap( -1 );
	fgSizer27->Add( lblObjTeam, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjTeamChoices;
	cboObjTeam = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjTeamChoices, 0 );
	cboObjTeam->SetSelection( 0 );
	fgSizer27->Add( cboObjTeam, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer33->Add( fgSizer27, 0, wxEXPAND, 5 );
	
	cboCurrObjInvalid = new wxCheckBox( this, wxID_ANY, wxT("Objective Invalid"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer33->Add( cboCurrObjInvalid, 0, wxALL, 3 );
	
	cboCurrObjNoCompletionSound = new wxCheckBox( this, wxID_ANY, wxT("Don't Play Completion Sound"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer33->Add( cboCurrObjNoCompletionSound, 0, wxALL, 3 );
	
	
	bSizer65->Add( sbSizer33, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer67;
	bSizer67 = new wxBoxSizer( wxHORIZONTAL );
	
	btnNewObjective = new wxButton( this, wxID_ANY, wxT("New Obj."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer67->Add( btnNewObjective, 0, wxALL, 3 );
	
	btnConfirm = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer67->Add( btnConfirm, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer67->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer65->Add( bSizer67, 0, 0, 3 );
	
	
	bSizer64->Add( bSizer65, 1, wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer64 );
	this->Layout();
	bSizer64->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgMissionObjectivesEditor::~dlgMissionObjectivesEditor()
{
}

BEGIN_EVENT_TABLE( dlgEventsEditor, wxDialog )
	EVT_CLOSE( dlgEventsEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgEventsEditor::dlgEventsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxHORIZONTAL );
	
	pnlEvents = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer98;
	bSizer98 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer1051;
	bSizer1051 = new wxBoxSizer( wxHORIZONTAL );
	
	trbSexp = new wxTreeCtrl( pnlEvents, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTR_DEFAULT_STYLE );
	trbSexp->SetMinSize( wxSize( 200,-1 ) );
	
	bSizer1051->Add( trbSexp, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer97;
	bSizer97 = new wxBoxSizer( wxVERTICAL );
	
	btnNewEvent = new wxButton( pnlEvents, wxID_ANY, wxT("New Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer97->Add( btnNewEvent, 0, wxALL|wxEXPAND, 3 );
	
	btnInsertEvent = new wxButton( pnlEvents, wxID_ANY, wxT("Insert Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer97->Add( btnInsertEvent, 0, wxALL|wxEXPAND, 3 );
	
	btnDeleteEvent = new wxButton( pnlEvents, wxID_ANY, wxT("Delete Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer97->Add( btnDeleteEvent, 0, wxALL|wxEXPAND, 3 );
	
	lblRepeatCount = new wxStaticText( pnlEvents, wxID_ANY, wxT("Repeat Count:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRepeatCount->Wrap( -1 );
	bSizer97->Add( lblRepeatCount, 0, wxLEFT|wxTOP, 3 );
	
	txtRepeatCount = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtRepeatCount->SetMaxLength( 0 ); 
	bSizer97->Add( txtRepeatCount, 0, wxALL, 3 );
	
	lblTriggerCount = new wxStaticText( pnlEvents, wxID_ANY, wxT("Trigger Count:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTriggerCount->Wrap( -1 );
	bSizer97->Add( lblTriggerCount, 0, wxLEFT|wxTOP, 3 );
	
	txtTriggerCount = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtTriggerCount->SetMaxLength( 0 ); 
	bSizer97->Add( txtTriggerCount, 0, wxALL, 3 );
	
	lblIntervalTime = new wxStaticText( pnlEvents, wxID_ANY, wxT("Interval Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIntervalTime->Wrap( -1 );
	bSizer97->Add( lblIntervalTime, 0, wxLEFT|wxTOP, 3 );
	
	txtIntervalTime = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtIntervalTime->SetMaxLength( 0 ); 
	bSizer97->Add( txtIntervalTime, 0, wxALL, 3 );
	
	lblScore = new wxStaticText( pnlEvents, wxID_ANY, wxT("Score:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblScore->Wrap( -1 );
	bSizer97->Add( lblScore, 0, wxLEFT|wxTOP, 3 );
	
	txtScore = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtScore->SetMaxLength( 0 ); 
	bSizer97->Add( txtScore, 0, wxALL, 3 );
	
	lblTeam = new wxStaticText( pnlEvents, wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTeam->Wrap( -1 );
	bSizer97->Add( lblTeam, 0, wxLEFT|wxTOP, 3 );
	
	wxString cboTeamChoices[] = { wxT("Team 1"), wxT("Team 2") };
	int cboTeamNChoices = sizeof( cboTeamChoices ) / sizeof( wxString );
	cboTeam = new wxChoice( pnlEvents, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboTeamNChoices, cboTeamChoices, 0 );
	cboTeam->SetSelection( 0 );
	bSizer97->Add( cboTeam, 1, wxALL|wxEXPAND, 3 );
	
	chkChained = new wxCheckBox( pnlEvents, wxID_ANY, wxT("Chained Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer97->Add( chkChained, 1, wxALL, 5 );
	
	lblChainDelay = new wxStaticText( pnlEvents, wxID_ANY, wxT("Chain Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblChainDelay->Wrap( -1 );
	bSizer97->Add( lblChainDelay, 0, wxLEFT|wxTOP, 3 );
	
	txtChainDelay = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtChainDelay->SetMaxLength( 0 ); 
	bSizer97->Add( txtChainDelay, 0, wxALL, 3 );
	
	
	bSizer1051->Add( bSizer97, 0, wxEXPAND, 5 );
	
	
	bSizer98->Add( bSizer1051, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer37;
	fgSizer37 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer37->AddGrowableCol( 1 );
	fgSizer37->SetFlexibleDirection( wxBOTH );
	fgSizer37->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDirectiveText = new wxStaticText( pnlEvents, wxID_ANY, wxT("Directive Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDirectiveText->Wrap( -1 );
	fgSizer37->Add( lblDirectiveText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtDirectiveText = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtDirectiveText->SetMaxLength( 0 ); 
	fgSizer37->Add( txtDirectiveText, 1, wxALL|wxEXPAND, 3 );
	
	lblDirectiveKeypress = new wxStaticText( pnlEvents, wxID_ANY, wxT("Directive Keypress Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDirectiveKeypress->Wrap( -1 );
	fgSizer37->Add( lblDirectiveKeypress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtDirectiveKeypress = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtDirectiveKeypress->SetMaxLength( 0 ); 
	fgSizer37->Add( txtDirectiveKeypress, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer98->Add( fgSizer37, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer44;
	sbSizer44 = new wxStaticBoxSizer( new wxStaticBox( pnlEvents, wxID_ANY, wxT("Event Logging") ), wxVERTICAL );
	
	wxBoxSizer* bSizer99;
	bSizer99 = new wxBoxSizer( wxVERTICAL );
	
	lblStateLogging = new wxStaticText( pnlEvents, wxID_ANY, wxT("States to log to Event.log:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStateLogging->Wrap( -1 );
	bSizer99->Add( lblStateLogging, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer371;
	fgSizer371 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer371->SetFlexibleDirection( wxBOTH );
	fgSizer371->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkTrue = new wxCheckBox( pnlEvents, wxID_ANY, wxT("True"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkTrue, 0, wxALL, 3 );
	
	chkTrueAlways = new wxCheckBox( pnlEvents, wxID_ANY, wxT("Always True"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkTrueAlways, 0, wxALL, 3 );
	
	chkRepeatFirst = new wxCheckBox( pnlEvents, wxID_ANY, wxT("First Repeat"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkRepeatFirst, 0, wxALL, 3 );
	
	chkTriggerFirst = new wxCheckBox( pnlEvents, wxID_ANY, wxT("First Trigger"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkTriggerFirst, 0, wxALL, 3 );
	
	chkFalse = new wxCheckBox( pnlEvents, wxID_ANY, wxT("False"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkFalse, 0, wxALL, 3 );
	
	chkFalseAlways = new wxCheckBox( pnlEvents, wxID_ANY, wxT("Always False"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkFalseAlways, 0, wxALL, 3 );
	
	chkRepeatLast = new wxCheckBox( pnlEvents, wxID_ANY, wxT("Last Repeat"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkRepeatLast, 0, wxALL, 3 );
	
	chkTriggerLast = new wxCheckBox( pnlEvents, wxID_ANY, wxT("Last Trigger"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer371->Add( chkTriggerLast, 0, wxALL, 3 );
	
	
	bSizer99->Add( fgSizer371, 1, wxEXPAND, 5 );
	
	
	sbSizer44->Add( bSizer99, 1, wxEXPAND, 5 );
	
	
	bSizer98->Add( sbSizer44, 0, wxALL|wxEXPAND, 3 );
	
	
	pnlEvents->SetSizer( bSizer98 );
	pnlEvents->Layout();
	bSizer98->Fit( pnlEvents );
	bSizer106->Add( pnlEvents, 1, wxEXPAND, 3 );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxVERTICAL );
	
	pnlMessages = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer341;
	sbSizer341 = new wxStaticBoxSizer( new wxStaticBox( pnlMessages, wxID_ANY, wxT("Messages") ), wxVERTICAL );
	
	wxBoxSizer* bSizer102;
	bSizer102 = new wxBoxSizer( wxHORIZONTAL );
	
	lstMessages = new wxListBox( pnlMessages, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	bSizer102->Add( lstMessages, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer781;
	bSizer781 = new wxBoxSizer( wxVERTICAL );
	
	
	bSizer781->Add( 0, 0, 1, wxEXPAND, 5 );
	
	btnNewMessage = new wxButton( pnlMessages, wxID_ANY, wxT("New"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer781->Add( btnNewMessage, 0, wxALL|wxEXPAND, 3 );
	
	btnDeleteMessage = new wxButton( pnlMessages, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer781->Add( btnDeleteMessage, 0, wxALL|wxEXPAND, 3 );
	
	
	bSizer102->Add( bSizer781, 0, wxEXPAND, 5 );
	
	
	sbSizer341->Add( bSizer102, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer74;
	bSizer74 = new wxBoxSizer( wxHORIZONTAL );
	
	lblMessageName = new wxStaticText( pnlMessages, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageName->Wrap( -1 );
	bSizer74->Add( lblMessageName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtMessageName = new wxTextCtrl( pnlMessages, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtMessageName->SetMaxLength( 0 ); 
	bSizer74->Add( txtMessageName, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer341->Add( bSizer74, 0, wxEXPAND, 3 );
	
	lblMessageText = new wxStaticText( pnlMessages, wxID_ANY, wxT("Message Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageText->Wrap( -1 );
	sbSizer341->Add( lblMessageText, 0, wxLEFT|wxTOP, 3 );
	
	txtMessageText = new wxTextCtrl( pnlMessages, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,64 ), wxTE_MULTILINE );
	txtMessageText->SetMaxLength( 0 ); 
	txtMessageText->SetMinSize( wxSize( -1,64 ) );
	txtMessageText->SetMaxSize( wxSize( -1,64 ) );
	
	sbSizer341->Add( txtMessageText, 0, wxALL|wxEXPAND, 3 );
	
	wxGridBagSizer* gbSizer8;
	gbSizer8 = new wxGridBagSizer( 0, 0 );
	gbSizer8->SetFlexibleDirection( wxBOTH );
	gbSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblMessageANI = new wxStaticText( pnlMessages, wxID_ANY, wxT("ANI File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageANI->Wrap( -1 );
	gbSizer8->Add( lblMessageANI, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboMessageANI = new wxComboBox( pnlMessages, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer8->Add( cboMessageANI, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnANIBrowse = new wxButton( pnlMessages, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer8->Add( btnANIBrowse, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblMessageAudio = new wxStaticText( pnlMessages, wxID_ANY, wxT("Audio File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageAudio->Wrap( -1 );
	gbSizer8->Add( lblMessageAudio, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboMessageAudio = new wxComboBox( pnlMessages, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer8->Add( cboMessageAudio, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer75;
	bSizer75 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAudioBrowse = new wxButton( pnlMessages, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer75->Add( btnAudioBrowse, 0, wxALL, 3 );
	
	btnPlayAudio = new wxBitmapButton( pnlMessages, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	bSizer75->Add( btnPlayAudio, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	
	gbSizer8->Add( bSizer75, wxGBPosition( 1, 2 ), wxGBSpan( 1, 1 ), wxEXPAND, 3 );
	
	lblPersona = new wxStaticText( pnlMessages, wxID_ANY, wxT("Persona:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPersona->Wrap( -1 );
	gbSizer8->Add( lblPersona, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_comboBox9 = new wxComboBox( pnlMessages, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer8->Add( m_comboBox9, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	btnUpdateStuff = new wxButton( pnlMessages, wxID_ANY, wxT("Update Stuff"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer8->Add( btnUpdateStuff, wxGBPosition( 2, 2 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	lblMessageTeam = new wxStaticText( pnlMessages, wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageTeam->Wrap( -1 );
	gbSizer8->Add( lblMessageTeam, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboTeamMessageChoices;
	cboTeamMessage = new wxChoice( pnlMessages, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboTeamMessageChoices, 0 );
	cboTeamMessage->SetSelection( 0 );
	gbSizer8->Add( cboTeamMessage, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	m_staticText139 = new wxStaticText( pnlMessages, wxID_ANY, wxT("(TvT only)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText139->Wrap( -1 );
	gbSizer8->Add( m_staticText139, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer341->Add( gbSizer8, 0, wxALIGN_CENTER, 5 );
	
	
	pnlMessages->SetSizer( sbSizer341 );
	pnlMessages->Layout();
	sbSizer341->Fit( pnlMessages );
	bSizer105->Add( pnlMessages, 2, wxALL|wxEXPAND, 3 );
	
	m_sdbSizer6 = new wxStdDialogButtonSizer();
	m_sdbSizer6OK = new wxButton( this, wxID_OK );
	m_sdbSizer6->AddButton( m_sdbSizer6OK );
	m_sdbSizer6Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer6->AddButton( m_sdbSizer6Cancel );
	m_sdbSizer6->Realize();
	
	bSizer105->Add( m_sdbSizer6, 0, wxEXPAND, 5 );
	
	
	bSizer106->Add( bSizer105, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer106 );
	this->Layout();
	bSizer106->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgEventsEditor::~dlgEventsEditor()
{
}

BEGIN_EVENT_TABLE( frmTeamLoadoutEditor, wxFrame )
	EVT_CLOSE( frmTeamLoadoutEditor::_wxFB_OnClose )
	EVT_BUTTON( wxID_CANCEL, frmTeamLoadoutEditor::_wxFB_OnCancel )
	EVT_BUTTON( wxID_OK, frmTeamLoadoutEditor::_wxFB_OnOK )
END_EVENT_TABLE()

frmTeamLoadoutEditor::frmTeamLoadoutEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	mnbDialogMenu = new wxMenuBar( 0 );
	mnuSelectTeam = new wxMenu();
	wxMenuItem* mnuTeam1;
	mnuTeam1 = new wxMenuItem( mnuSelectTeam, ID_mnuTeam1, wxString( wxT("Team 1") ) , wxEmptyString, wxITEM_RADIO );
	mnuSelectTeam->Append( mnuTeam1 );
	mnuTeam1->Check( true );
	
	wxMenuItem* mnuTeam2;
	mnuTeam2 = new wxMenuItem( mnuSelectTeam, ID_mnuTeam2, wxString( wxT("Team 2") ) , wxEmptyString, wxITEM_RADIO );
	mnuSelectTeam->Append( mnuTeam2 );
	mnuTeam2->Enable( false );
	
	mnbDialogMenu->Append( mnuSelectTeam, wxT("Select Team") ); 
	
	m_menu16 = new wxMenu();
	wxMenuItem* meuBalanceTeams;
	meuBalanceTeams = new wxMenuItem( m_menu16, wxID_ANY, wxString( wxT("Balance Teams") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu16->Append( meuBalanceTeams );
	meuBalanceTeams->Enable( false );
	
	mnbDialogMenu->Append( m_menu16, wxT("Options") ); 
	
	this->SetMenuBar( mnbDialogMenu );
	
	wxBoxSizer* bSizer79;
	bSizer79 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer80;
	bSizer80 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxVERTICAL );
	
	lblAvailableStartShips = new wxStaticText( this, wxID_ANY, wxT("Available Starting Ships:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAvailableStartShips->Wrap( -1 );
	bSizer81->Add( lblAvailableStartShips, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblShipsFromVariable = new wxStaticText( this, wxID_ANY, wxT("From Variable:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipsFromVariable->Wrap( -1 );
	bSizer81->Add( lblShipsFromVariable, 0, wxALL, 3 );
	
	lbxStartShipsVariable = new wxListBox( this, ID_lbxStartShipsVariable, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_ALWAYS_SB|wxLB_SINGLE|wxLB_SORT ); 
	bSizer81->Add( lbxStartShipsVariable, 1, wxALL|wxEXPAND, 3 );
	
	lblShipsFromTbl = new wxStaticText( this, wxID_ANY, wxT("From Table Entry:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipsFromTbl->Wrap( -1 );
	bSizer81->Add( lblShipsFromTbl, 0, wxALL, 3 );
	
	wxArrayString cklShipsFromTblChoices;
	cklShipsFromTbl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cklShipsFromTblChoices, wxLB_ALWAYS_SB|wxLB_SINGLE );
	bSizer81->Add( cklShipsFromTbl, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer82;
	bSizer82 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText141 = new wxStaticText( this, wxID_ANY, wxT("Extra Available:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText141->Wrap( -1 );
	bSizer82->Add( m_staticText141, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnExtraShipsAvailable = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 99, 0 );
	bSizer82->Add( spnExtraShipsAvailable, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer81->Add( bSizer82, 0, wxEXPAND, 3 );
	
	lblSetShipAmountFromVariable = new wxStaticText( this, wxID_ANY, wxT("Set amount from variable:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSetShipAmountFromVariable->Wrap( -1 );
	bSizer81->Add( lblSetShipAmountFromVariable, 0, wxALL, 3 );
	
	wxArrayString cboSetShipAmountFromVariableChoices;
	cboSetShipAmountFromVariable = new wxChoice( this, ID_cboSetShipAmountFromVariable, wxDefaultPosition, wxDefaultSize, cboSetShipAmountFromVariableChoices, 0 );
	cboSetShipAmountFromVariable->SetSelection( 0 );
	bSizer81->Add( cboSetShipAmountFromVariable, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer83;
	bSizer83 = new wxBoxSizer( wxHORIZONTAL );
	
	lblAmountOfShipsInWings = new wxStaticText( this, wxID_ANY, wxT("Amount used in Wings:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmountOfShipsInWings->Wrap( -1 );
	bSizer83->Add( lblAmountOfShipsInWings, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAmountOfShipsInWings = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer83->Add( txtAmountOfShipsInWings, 0, wxALL, 3 );
	
	
	bSizer81->Add( bSizer83, 0, 0, 3 );
	
	
	bSizer80->Add( bSizer81, 1, wxALL|wxEXPAND, 3 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer80->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer811;
	bSizer811 = new wxBoxSizer( wxVERTICAL );
	
	lblAvailableStartWeapons = new wxStaticText( this, wxID_ANY, wxT("Available Starting Weapons:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAvailableStartWeapons->Wrap( -1 );
	bSizer811->Add( lblAvailableStartWeapons, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblWeaponsFromVariable = new wxStaticText( this, wxID_ANY, wxT("From Variable:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWeaponsFromVariable->Wrap( -1 );
	bSizer811->Add( lblWeaponsFromVariable, 0, wxALL, 3 );
	
	lbxStartWeaponsVariable = new wxListBox( this, ID_StartWeaponsVariable, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_ALWAYS_SB|wxLB_SINGLE ); 
	bSizer811->Add( lbxStartWeaponsVariable, 1, wxALL|wxEXPAND, 3 );
	
	lblWeaponsFromTbl = new wxStaticText( this, wxID_ANY, wxT("From Table Entry:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWeaponsFromTbl->Wrap( -1 );
	bSizer811->Add( lblWeaponsFromTbl, 0, wxALL, 3 );
	
	wxArrayString cklWeaponsFromTblChoices;
	cklWeaponsFromTbl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cklWeaponsFromTblChoices, wxLB_ALWAYS_SB );
	bSizer811->Add( cklWeaponsFromTbl, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer821;
	bSizer821 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText1411 = new wxStaticText( this, wxID_ANY, wxT("Extra available"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1411->Wrap( -1 );
	bSizer821->Add( m_staticText1411, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnExtraWeaponsAvailable = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 99999, 0 );
	bSizer821->Add( spnExtraWeaponsAvailable, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer811->Add( bSizer821, 0, wxEXPAND, 3 );
	
	lblSetWeaponAmountFromVariable = new wxStaticText( this, wxID_ANY, wxT("Set amount from variable"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSetWeaponAmountFromVariable->Wrap( -1 );
	bSizer811->Add( lblSetWeaponAmountFromVariable, 0, wxALL, 3 );
	
	wxArrayString cboSetWeaponAmountFromVariableChoices;
	cboSetWeaponAmountFromVariable = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboSetWeaponAmountFromVariableChoices, 0 );
	cboSetWeaponAmountFromVariable->SetSelection( 0 );
	bSizer811->Add( cboSetWeaponAmountFromVariable, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer831;
	bSizer831 = new wxBoxSizer( wxHORIZONTAL );
	
	lblAmountOfWeaponsInWings = new wxStaticText( this, wxID_ANY, wxT("Amount used in Wings:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmountOfWeaponsInWings->Wrap( -1 );
	bSizer831->Add( lblAmountOfWeaponsInWings, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAmountOfWeaponsInWings = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer831->Add( txtAmountOfWeaponsInWings, 0, wxALL, 3 );
	
	
	bSizer811->Add( bSizer831, 0, 0, 3 );
	
	
	bSizer80->Add( bSizer811, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer79->Add( bSizer80, 1, wxEXPAND, 5 );
	
	sdbDialogButtons = new wxStdDialogButtonSizer();
	sdbDialogButtonsOK = new wxButton( this, wxID_OK );
	sdbDialogButtons->AddButton( sdbDialogButtonsOK );
	sdbDialogButtonsCancel = new wxButton( this, wxID_CANCEL );
	sdbDialogButtons->AddButton( sdbDialogButtonsCancel );
	sdbDialogButtons->Realize();
	
	bSizer79->Add( sdbDialogButtons, 0, wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer79 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

frmTeamLoadoutEditor::~frmTeamLoadoutEditor()
{
}

BEGIN_EVENT_TABLE( dlgBackgroundEditor, wxDialog )
	EVT_CLOSE( dlgBackgroundEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgBackgroundEditor::dlgBackgroundEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	
	wxBoxSizer* bSizer89;
	bSizer89 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer90;
	bSizer90 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString cboBackgroundPresetChoices[] = { wxT("Background 1"), wxT("Background 2") };
	int cboBackgroundPresetNChoices = sizeof( cboBackgroundPresetChoices ) / sizeof( wxString );
	cboBackgroundPreset = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboBackgroundPresetNChoices, cboBackgroundPresetChoices, 0 );
	cboBackgroundPreset->SetSelection( 0 );
	bSizer90->Add( cboBackgroundPreset, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnImportBackground = new wxButton( this, wxID_ANY, wxT("Import"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer90->Add( btnImportBackground, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer91->Add( bSizer90, 0, 0, 5 );
	
	wxStaticBoxSizer* sbSizer35;
	sbSizer35 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bitmaps") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxVERTICAL );
	
	lclBGBitmaps = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON|wxLC_SINGLE_SEL );
	bSizer92->Add( lclBGBitmaps, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer93;
	bSizer93 = new wxBoxSizer( wxHORIZONTAL );
	
	btnBitmapAdd = new wxButton( this, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer93->Add( btnBitmapAdd, 1, wxALL|wxEXPAND, 3 );
	
	btnBitmapDelete = new wxButton( this, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer93->Add( btnBitmapDelete, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer92->Add( bSizer93, 0, wxEXPAND, 3 );
	
	
	sbSizer35->Add( bSizer92, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer95;
	bSizer95 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer107;
	bSizer107 = new wxBoxSizer( wxHORIZONTAL );
	
	lblBitmap = new wxStaticText( this, wxID_ANY, wxT("Bitmap:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmap->Wrap( -1 );
	bSizer107->Add( lblBitmap, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString cboBitmapSelectChoices;
	cboBitmapSelect = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboBitmapSelectChoices, 0 );
	cboBitmapSelect->SetSelection( 0 );
	bSizer107->Add( cboBitmapSelect, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer95->Add( bSizer107, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer31->SetFlexibleDirection( wxBOTH );
	fgSizer31->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch = new wxStaticText( this, wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch->Wrap( -1 );
	fgSizer31->Add( lblBitmapPitch, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank = new wxStaticText( this, wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank->Wrap( -1 );
	fgSizer31->Add( lblBitmapBank, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading = new wxStaticText( this, wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading->Wrap( -1 );
	fgSizer31->Add( lblBitmapHeading, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer31->Add( spnBitmapPitch, 0, wxALL, 3 );
	
	spnBitmapBank = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer31->Add( spnBitmapBank, 0, wxALL, 3 );
	
	spnBitmapHeading = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer31->Add( spnBitmapHeading, 0, wxALL, 3 );
	
	
	bSizer95->Add( fgSizer31, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxGridBagSizer* gbSizer2;
	gbSizer2 = new wxGridBagSizer( 0, 0 );
	gbSizer2->SetFlexibleDirection( wxBOTH );
	gbSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapScale = new wxStaticText( this, wxID_ANY, wxT("Scale (x/y):"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapScale->Wrap( -1 );
	gbSizer2->Add( lblBitmapScale, wxGBPosition( 0, 0 ), wxGBSpan( 1, 2 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapScaleX = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer2->Add( spnBitmapScaleX, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapScaleY = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer2->Add( spnBitmapScaleY, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer95->Add( gbSizer2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxGridBagSizer* gbSizer3;
	gbSizer3 = new wxGridBagSizer( 0, 0 );
	gbSizer3->SetFlexibleDirection( wxBOTH );
	gbSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapDivisions = new wxStaticText( this, wxID_ANY, wxT("# of Divisions (x/y):"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapDivisions->Wrap( -1 );
	gbSizer3->Add( lblBitmapDivisions, wxGBPosition( 0, 0 ), wxGBSpan( 1, 2 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapDivisionsX = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer3->Add( spnBitmapDivisionsX, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapDivisionsY = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer3->Add( spnBitmapDivisionsY, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer95->Add( gbSizer3, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	sbSizer35->Add( bSizer95, 0, wxEXPAND, 5 );
	
	
	bSizer91->Add( sbSizer35, 1, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer36;
	sbSizer36 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Nebula") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer100;
	bSizer100 = new wxBoxSizer( wxVERTICAL );
	
	chkFullNebula = new wxCheckBox( this, wxID_ANY, wxT("Full Nebula"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer100->Add( chkFullNebula, 0, wxALL, 3 );
	
	chkToggleShipTrails = new wxCheckBox( this, wxID_ANY, wxT("Toggle ship trails"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer100->Add( chkToggleShipTrails, 0, wxALL, 3 );
	
	wxFlexGridSizer* fgSizer32;
	fgSizer32 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer32->SetFlexibleDirection( wxBOTH );
	fgSizer32->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblNebulaRange = new wxStaticText( this, wxID_ANY, wxT("Range:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaRange->Wrap( -1 );
	fgSizer32->Add( lblNebulaRange, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtNebulaRange = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtNebulaRange->SetMaxLength( 0 ); 
	fgSizer32->Add( txtNebulaRange, 0, wxALL|wxEXPAND, 3 );
	
	lblNebulaPattern = new wxStaticText( this, wxID_ANY, wxT("Pattern:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaPattern->Wrap( -1 );
	fgSizer32->Add( lblNebulaPattern, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboNebulaPatternChoices[] = { wxT("nbackblue1"), wxT("nbackblue2"), wxT("nbackcyan"), wxT("nbackgreen"), wxT("nbackpurp1"), wxT("nbackpurp2"), wxT("nbackred"), wxT("nbackblack") };
	int cboNebulaPatternNChoices = sizeof( cboNebulaPatternChoices ) / sizeof( wxString );
	cboNebulaPattern = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboNebulaPatternNChoices, cboNebulaPatternChoices, 0 );
	cboNebulaPattern->SetSelection( 0 );
	fgSizer32->Add( cboNebulaPattern, 0, wxALL|wxEXPAND, 3 );
	
	lblLightningStorm = new wxStaticText( this, wxID_ANY, wxT("Lightning Storm:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLightningStorm->Wrap( -1 );
	fgSizer32->Add( lblLightningStorm, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboLightningStormChoices[] = { wxT("none"), wxT("s_standard"), wxT("s_active"), wxT("s_emp"), wxT("s_medium") };
	int cboLightningStormNChoices = sizeof( cboLightningStormChoices ) / sizeof( wxString );
	cboLightningStorm = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboLightningStormNChoices, cboLightningStormChoices, 0 );
	cboLightningStorm->SetSelection( 0 );
	fgSizer32->Add( cboLightningStorm, 0, wxALL|wxEXPAND, 3 );
	
	lblNebulaFogNear = new wxStaticText( this, wxID_ANY, wxT("Fog Near:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaFogNear->Wrap( -1 );
	fgSizer32->Add( lblNebulaFogNear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinCtrl49 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer32->Add( m_spinCtrl49, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblNebulaFogMultiplier = new wxStaticText( this, wxID_ANY, wxT("Fog Multiplier:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaFogMultiplier->Wrap( -1 );
	fgSizer32->Add( lblNebulaFogMultiplier, 0, wxALL, 5 );
	
	m_spinCtrl50 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer32->Add( m_spinCtrl50, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer100->Add( fgSizer32, 0, wxEXPAND, 5 );
	
	
	sbSizer36->Add( bSizer100, 0, 0, 5 );
	
	wxBoxSizer* bSizer110;
	bSizer110 = new wxBoxSizer( wxVERTICAL );
	
	lblNebulaPoofs = new wxStaticText( this, wxID_ANY, wxT("Nebula Poofs:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaPoofs->Wrap( -1 );
	bSizer110->Add( lblNebulaPoofs, 0, wxALL, 3 );
	
	wxString clbNebulaPoofsChoices[] = { wxT("PoofGreen01"), wxT("PoofGreen02"), wxT("PoofRed01"), wxT("PoofRed02"), wxT("PoofPurp01"), wxT("PoofPurp02") };
	int clbNebulaPoofsNChoices = sizeof( clbNebulaPoofsChoices ) / sizeof( wxString );
	clbNebulaPoofs = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, clbNebulaPoofsNChoices, clbNebulaPoofsChoices, wxLB_ALWAYS_SB );
	bSizer110->Add( clbNebulaPoofs, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer36->Add( bSizer110, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer91->Add( sbSizer36, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer421;
	sbSizer421 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("FS1 Nebula") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer43;
	fgSizer43 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer43->SetFlexibleDirection( wxBOTH );
	fgSizer43->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText160 = new wxStaticText( this, wxID_ANY, wxT("Pattern:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText160->Wrap( -1 );
	fgSizer43->Add( m_staticText160, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboNebulaPattern1Choices[] = { wxT("None"), wxT("Pattern01"), wxT("Pattern02"), wxT("Pattern03") };
	int cboNebulaPattern1NChoices = sizeof( cboNebulaPattern1Choices ) / sizeof( wxString );
	cboNebulaPattern1 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboNebulaPattern1NChoices, cboNebulaPattern1Choices, 0 );
	cboNebulaPattern1->SetSelection( 0 );
	fgSizer43->Add( cboNebulaPattern1, 0, wxALL|wxEXPAND, 3 );
	
	m_staticText161 = new wxStaticText( this, wxID_ANY, wxT("Color"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText161->Wrap( -1 );
	fgSizer43->Add( m_staticText161, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboNebulaColourChoices[] = { wxT("Red"), wxT("Blue"), wxT("Gold"), wxT("Purple"), wxT("Maroon"), wxT("Green"), wxT("Grey Blue"), wxT("Violet"), wxT("Grey Green") };
	int cboNebulaColourNChoices = sizeof( cboNebulaColourChoices ) / sizeof( wxString );
	cboNebulaColour = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboNebulaColourNChoices, cboNebulaColourChoices, 0 );
	cboNebulaColour->SetSelection( 0 );
	fgSizer43->Add( cboNebulaColour, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer421->Add( fgSizer43, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	wxFlexGridSizer* fgSizer312;
	fgSizer312 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer312->SetFlexibleDirection( wxBOTH );
	fgSizer312->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch2 = new wxStaticText( this, wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch2->Wrap( -1 );
	fgSizer312->Add( lblBitmapPitch2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank2 = new wxStaticText( this, wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank2->Wrap( -1 );
	fgSizer312->Add( lblBitmapBank2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading2 = new wxStaticText( this, wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading2->Wrap( -1 );
	fgSizer312->Add( lblBitmapHeading2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch2 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer312->Add( spnBitmapPitch2, 0, wxALL, 3 );
	
	spnBitmapBank2 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer312->Add( spnBitmapBank2, 0, wxALL, 3 );
	
	spnBitmapHeading2 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer312->Add( spnBitmapHeading2, 0, wxALL, 3 );
	
	
	sbSizer421->Add( fgSizer312, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer91->Add( sbSizer421, 0, wxEXPAND, 3 );
	
	
	bSizer89->Add( bSizer91, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer102;
	bSizer102 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer1001;
	bSizer1001 = new wxBoxSizer( wxHORIZONTAL );
	
	btnBGSwap = new wxButton( this, wxID_ANY, wxT("Swap with"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1001->Add( btnBGSwap, 0, wxALL, 3 );
	
	wxString cboBackgroundSwapChoices[] = { wxT("Background 1"), wxT("Background 2") };
	int cboBackgroundSwapNChoices = sizeof( cboBackgroundSwapChoices ) / sizeof( wxString );
	cboBackgroundSwap = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboBackgroundSwapNChoices, cboBackgroundSwapChoices, 0 );
	cboBackgroundSwap->SetSelection( 0 );
	bSizer1001->Add( cboBackgroundSwap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer102->Add( bSizer1001, 0, wxALIGN_RIGHT, 5 );
	
	wxStaticBoxSizer* sbSizer37;
	sbSizer37 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Suns") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer1021;
	bSizer1021 = new wxBoxSizer( wxVERTICAL );
	
	lclBGSunbitmaps = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON );
	bSizer1021->Add( lclBGSunbitmaps, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAddSunBitmap = new wxButton( this, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer103->Add( btnAddSunBitmap, 1, wxALL|wxEXPAND, 3 );
	
	btnDeleteSunBitmap = new wxButton( this, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer103->Add( btnDeleteSunBitmap, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer1021->Add( bSizer103, 0, wxEXPAND, 3 );
	
	
	sbSizer37->Add( bSizer1021, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer104;
	bSizer104 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer108;
	bSizer108 = new wxBoxSizer( wxHORIZONTAL );
	
	lblSun = new wxStaticText( this, wxID_ANY, wxT("Sun:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSun->Wrap( -1 );
	bSizer108->Add( lblSun, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxArrayString cboSunSelectChoices;
	cboSunSelect = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboSunSelectChoices, 0 );
	cboSunSelect->SetSelection( 0 );
	bSizer108->Add( cboSunSelect, 0, wxALL, 3 );
	
	
	bSizer104->Add( bSizer108, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer311;
	fgSizer311 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer311->SetFlexibleDirection( wxBOTH );
	fgSizer311->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch1 = new wxStaticText( this, wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch1->Wrap( -1 );
	fgSizer311->Add( lblBitmapPitch1, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank1 = new wxStaticText( this, wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank1->Wrap( -1 );
	fgSizer311->Add( lblBitmapBank1, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading1 = new wxStaticText( this, wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading1->Wrap( -1 );
	fgSizer311->Add( lblBitmapHeading1, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch1 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer311->Add( spnBitmapPitch1, 0, wxALL, 3 );
	
	spnBitmapBank1 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer311->Add( spnBitmapBank1, 0, wxALL, 3 );
	
	spnBitmapHeading1 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer311->Add( spnBitmapHeading1, 0, wxALL, 3 );
	
	
	bSizer104->Add( fgSizer311, 1, wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText179 = new wxStaticText( this, wxID_ANY, wxT("Scale:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText179->Wrap( -1 );
	bSizer105->Add( m_staticText179, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnSunScale = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 10, 10 );
	bSizer105->Add( spnSunScale, 0, wxALL, 3 );
	
	
	bSizer104->Add( bSizer105, 0, wxALIGN_CENTER|wxALL|wxSHAPED, 3 );
	
	
	sbSizer37->Add( bSizer104, 1, 0, 5 );
	
	
	bSizer102->Add( sbSizer37, 1, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer38;
	sbSizer38 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Ambient Light") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer40;
	fgSizer40 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer40->AddGrowableCol( 1 );
	fgSizer40->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer40->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblAmbientRed = new wxStaticText( this, wxID_ANY, wxT("Red:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmbientRed->Wrap( -1 );
	fgSizer40->Add( lblAmbientRed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldAmbientRed = new wxSlider( this, wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer40->Add( sldAmbientRed, 1, wxALL|wxEXPAND, 3 );
	
	spnAmbientRed = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer40->Add( spnAmbientRed, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblAmbientGreen = new wxStaticText( this, wxID_ANY, wxT("Green:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmbientGreen->Wrap( -1 );
	fgSizer40->Add( lblAmbientGreen, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldAmbientGreen = new wxSlider( this, wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer40->Add( sldAmbientGreen, 0, wxALL|wxEXPAND, 3 );
	
	spnAmbientGreen = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer40->Add( spnAmbientGreen, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblAmbientBlue = new wxStaticText( this, wxID_ANY, wxT("Blue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmbientBlue->Wrap( -1 );
	fgSizer40->Add( lblAmbientBlue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldAmbientBlue = new wxSlider( this, wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer40->Add( sldAmbientBlue, 0, wxALL|wxEXPAND, 3 );
	
	spnAmbientBlue = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 48,-1 ), wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer40->Add( spnAmbientBlue, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	sbSizer38->Add( fgSizer40, 0, wxEXPAND, 5 );
	
	
	bSizer102->Add( sbSizer38, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer40;
	sbSizer40 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Skybox") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer411;
	fgSizer411 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer411->AddGrowableCol( 1 );
	fgSizer411->SetFlexibleDirection( wxBOTH );
	fgSizer411->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnSkyboxSelect = new wxButton( this, wxID_ANY, wxT("Skybox Model"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer411->Add( btnSkyboxSelect, 0, wxALL|wxEXPAND, 3 );
	
	txtSkybox = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSkybox->SetMaxLength( 0 ); 
	fgSizer411->Add( txtSkybox, 0, wxALL|wxEXPAND, 3 );
	
	btnSkyboxMap = new wxButton( this, wxID_ANY, wxT("Skybox Map"), wxDefaultPosition, wxDefaultSize, 0 );
	btnSkyboxMap->Enable( false );
	
	fgSizer411->Add( btnSkyboxMap, 0, wxALL|wxEXPAND, 3 );
	
	m_textCtrl73 = new wxTextCtrl( this, wxID_ANY, wxT("Coming Soon!"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrl73->Enable( false );
	
	fgSizer411->Add( m_textCtrl73, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer40->Add( fgSizer411, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer1071;
	bSizer1071 = new wxBoxSizer( wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer3121;
	fgSizer3121 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer3121->SetFlexibleDirection( wxBOTH );
	fgSizer3121->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch21 = new wxStaticText( this, wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch21->Wrap( -1 );
	fgSizer3121->Add( lblBitmapPitch21, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank21 = new wxStaticText( this, wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank21->Wrap( -1 );
	fgSizer3121->Add( lblBitmapBank21, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading21 = new wxStaticText( this, wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading21->Wrap( -1 );
	fgSizer3121->Add( lblBitmapHeading21, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch21 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer3121->Add( spnBitmapPitch21, 0, wxALL, 3 );
	
	spnBitmapBank21 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer3121->Add( spnBitmapBank21, 0, wxALL, 3 );
	
	spnBitmapHeading21 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer3121->Add( spnBitmapHeading21, 0, wxALL, 3 );
	
	
	bSizer1071->Add( fgSizer3121, 0, 0, 5 );
	
	wxFlexGridSizer* fgSizer41;
	fgSizer41 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer41->SetFlexibleDirection( wxBOTH );
	fgSizer41->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkSBNoCull = new wxCheckBox( this, wxID_ANY, wxT("No Cull"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer41->Add( chkSBNoCull, 0, wxALL, 3 );
	
	chkSBNoGlowmaps = new wxCheckBox( this, wxID_ANY, wxT("No Glowmaps"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer41->Add( chkSBNoGlowmaps, 0, wxALL, 3 );
	
	chkSBNoLighting = new wxCheckBox( this, wxID_ANY, wxT("No Lighting"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer41->Add( chkSBNoLighting, 0, wxALL, 3 );
	
	chkSBNoZBuffer = new wxCheckBox( this, wxID_ANY, wxT("No Z-Buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer41->Add( chkSBNoZBuffer, 0, wxALL, 3 );
	
	chkSBForceClamp = new wxCheckBox( this, wxID_ANY, wxT("Force Clamp"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer41->Add( chkSBForceClamp, 0, wxALL, 3 );
	
	chkSBTransparent = new wxCheckBox( this, wxID_ANY, wxT("Transparent"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer41->Add( chkSBTransparent, 0, wxALL, 3 );
	
	
	bSizer1071->Add( fgSizer41, 0, wxEXPAND, 5 );
	
	
	sbSizer40->Add( bSizer1071, 1, wxEXPAND, 5 );
	
	
	bSizer102->Add( sbSizer40, 1, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer39;
	sbSizer39 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Misc.") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer42;
	fgSizer42 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer42->AddGrowableCol( 1 );
	fgSizer42->SetFlexibleDirection( wxBOTH );
	fgSizer42->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText186 = new wxStaticText( this, wxID_ANY, wxT("Number of Stars:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText186->Wrap( -1 );
	fgSizer42->Add( m_staticText186, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldNumStars = new wxSlider( this, wxID_ANY, 500, 0, 2000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer42->Add( sldNumStars, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	m_spinCtrl43 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 62,-1 ), wxSP_ARROW_KEYS, 0, 2000, 500 );
	fgSizer42->Add( m_spinCtrl43, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	sbSizer39->Add( fgSizer42, 0, wxEXPAND, 5 );
	
	m_checkBox48 = new wxCheckBox( this, wxID_ANY, wxT("Takes place inside Subspace"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer39->Add( m_checkBox48, 0, wxALL, 3 );
	
	wxFlexGridSizer* fgSizer37;
	fgSizer37 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer37->AddGrowableCol( 1 );
	fgSizer37->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer37->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnEnvMap = new wxButton( this, wxID_ANY, wxT("Environment Map"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer37->Add( btnEnvMap, 0, wxALL|wxEXPAND, 3 );
	
	txtEnvMap = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtEnvMap->SetMaxLength( 0 ); 
	fgSizer37->Add( txtEnvMap, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer39->Add( fgSizer37, 0, wxEXPAND, 5 );
	
	
	bSizer102->Add( sbSizer39, 0, wxEXPAND, 3 );
	
	
	bSizer89->Add( bSizer102, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer89 );
	this->Layout();
	bSizer89->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgBackgroundEditor::~dlgBackgroundEditor()
{
}

BEGIN_EVENT_TABLE( dlgReinforcementsEditor, wxDialog )
	EVT_CLOSE( dlgReinforcementsEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgReinforcementsEditor::dlgReinforcementsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxGridBagSizer* gbSizer8;
	gbSizer8 = new wxGridBagSizer( 0, 0 );
	gbSizer8->SetFlexibleDirection( wxBOTH );
	gbSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblReinforcements = new wxStaticText( this, wxID_ANY, wxT("Reinforced Wings:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblReinforcements->Wrap( -1 );
	gbSizer8->Add( lblReinforcements, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	lstReinforcements = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, wxLB_ALWAYS_SB ); 
	gbSizer8->Add( lstReinforcements, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxVERTICAL );
	
	btnAdd = new wxButton( this, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer101->Add( btnAdd, 0, wxALL, 3 );
	
	btnDelete = new wxButton( this, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer101->Add( btnDelete, 0, wxALL, 3 );
	
	m_staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer101->Add( m_staticline3, 0, wxEXPAND | wxALL, 3 );
	
	btnOk = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer101->Add( btnOk, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer101->Add( btnCancel, 0, wxALL, 3 );
	
	
	gbSizer8->Add( bSizer101, wxGBPosition( 0, 1 ), wxGBSpan( 2, 1 ), 0, 5 );
	
	wxFlexGridSizer* fgSizer39;
	fgSizer39 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer39->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer39->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblUses = new wxStaticText( this, wxID_ANY, wxT("Uses:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblUses->Wrap( -1 );
	fgSizer39->Add( lblUses, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	spnUses = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 64,-1 ), wxSP_ARROW_KEYS, 0, 1000, 0 );
	fgSizer39->Add( spnUses, 0, wxALL, 3 );
	
	
	fgSizer39->Add( 50, 0, 1, wxEXPAND, 5 );
	
	lblDelayAfterArrival = new wxStaticText( this, wxID_ANY, wxT("Delay After Arrival:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDelayAfterArrival->Wrap( -1 );
	fgSizer39->Add( lblDelayAfterArrival, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDelayAfterArrival = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 64,-1 ), wxSP_ARROW_KEYS, 0, 1000, 0 );
	fgSizer39->Add( spnDelayAfterArrival, 0, wxALL, 3 );
	
	
	gbSizer8->Add( fgSizer39, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	
	gbSizer8->AddGrowableCol( 0 );
	gbSizer8->AddGrowableRow( 0 );
	
	this->SetSizer( gbSizer8 );
	this->Layout();
	gbSizer8->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgReinforcementsEditor::~dlgReinforcementsEditor()
{
}

dlgReinforcementsPicker::dlgReinforcementsPicker( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer114;
	bSizer114 = new wxBoxSizer( wxHORIZONTAL );
	
	lstReincforcements = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( 200,200 ), wxLC_LIST );
	bSizer114->Add( lstReincforcements, 0, wxALL|wxEXPAND, 8 );
	
	wxBoxSizer* bSizer115;
	bSizer115 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer115->Add( btnOK, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer115->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer114->Add( bSizer115, 1, wxALIGN_BOTTOM|wxBOTTOM|wxRIGHT, 8 );
	
	
	this->SetSizer( bSizer114 );
	this->Layout();
	bSizer114->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgReinforcementsPicker::~dlgReinforcementsPicker()
{
}

BEGIN_EVENT_TABLE( dlgAsteroidFieldEditor, wxDialog )
	EVT_CLOSE( dlgAsteroidFieldEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgAsteroidFieldEditor::dlgAsteroidFieldEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer59;
	bSizer59 = new wxBoxSizer( wxVERTICAL );
	
	chkAsteroidsEnabled = new wxCheckBox( this, wxID_ANY, wxT("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer59->Add( chkAsteroidsEnabled, 1, wxALL, 5 );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxHORIZONTAL );
	
	pFieldProperties = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbField;
	sbField = new wxStaticBoxSizer( new wxStaticBox( pFieldProperties, wxID_ANY, wxT("Field Properties") ), wxVERTICAL );
	
	wxStaticBoxSizer* grpFieldMode;
	grpFieldMode = new wxStaticBoxSizer( new wxStaticBox( pFieldProperties, wxID_ANY, wxT("Mode") ), wxHORIZONTAL );
	
	optFieldActive = new wxRadioButton( pFieldProperties, wxID_ANY, wxT("Active Field"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optFieldActive->SetValue( true ); 
	grpFieldMode->Add( optFieldActive, 1, wxALL|wxEXPAND, 3 );
	
	optFieldPassive = new wxRadioButton( pFieldProperties, wxID_ANY, wxT("Passive Field"), wxDefaultPosition, wxDefaultSize, 0 );
	grpFieldMode->Add( optFieldPassive, 1, wxALL|wxEXPAND, 3 );
	
	
	sbField->Add( grpFieldMode, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* grpFieldContent;
	grpFieldContent = new wxStaticBoxSizer( new wxStaticBox( pFieldProperties, wxID_ANY, wxT("Content") ), wxVERTICAL );
	
	wxGridBagSizer* gbFieldContent;
	gbFieldContent = new wxGridBagSizer( 0, 0 );
	gbFieldContent->SetFlexibleDirection( wxBOTH );
	gbFieldContent->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	optFieldtypeAsteroid = new wxRadioButton( pFieldProperties, wxID_ANY, wxT("Asteroid"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	gbFieldContent->Add( optFieldtypeAsteroid, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	optFieldTypeShip = new wxRadioButton( pFieldProperties, wxID_ANY, wxT("Ship"), wxDefaultPosition, wxDefaultSize, 0 );
	gbFieldContent->Add( optFieldTypeShip, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	chkBrownRocks = new wxCheckBox( pFieldProperties, wxID_ANY, wxT("Brown"), wxDefaultPosition, wxDefaultSize, 0 );
	gbFieldContent->Add( chkBrownRocks, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjType1Choices;
	cboObjType1 = new wxChoice( pFieldProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjType1Choices, 0 );
	cboObjType1->SetSelection( 0 );
	gbFieldContent->Add( cboObjType1, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	chkBlueRocks = new wxCheckBox( pFieldProperties, wxID_ANY, wxT("Blue"), wxDefaultPosition, wxDefaultSize, 0 );
	gbFieldContent->Add( chkBlueRocks, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjType2Choices;
	cboObjType2 = new wxChoice( pFieldProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjType2Choices, 0 );
	cboObjType2->SetSelection( 0 );
	gbFieldContent->Add( cboObjType2, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	chkOrangeRocks = new wxCheckBox( pFieldProperties, wxID_ANY, wxT("Orange"), wxDefaultPosition, wxDefaultSize, 0 );
	gbFieldContent->Add( chkOrangeRocks, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjType3Choices;
	cboObjType3 = new wxChoice( pFieldProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjType3Choices, 0 );
	cboObjType3->SetSelection( 0 );
	gbFieldContent->Add( cboObjType3, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	m_staticline1 = new wxStaticLine( pFieldProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	gbFieldContent->Add( m_staticline1, wxGBPosition( 4, 0 ), wxGBSpan( 1, 2 ), wxEXPAND | wxALL, 5 );
	
	lblNumberObjects = new wxStaticText( pFieldProperties, wxID_ANY, wxT("Number:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNumberObjects->Wrap( -1 );
	gbFieldContent->Add( lblNumberObjects, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnObjects = new wxSpinCtrl( pFieldProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbFieldContent->Add( spnObjects, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	m_staticText68 = new wxStaticText( pFieldProperties, wxID_ANY, wxT("Avg. Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText68->Wrap( -1 );
	gbFieldContent->Add( m_staticText68, wxGBPosition( 6, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	numAvgSpeed = new wxTextCtrl( pFieldProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numAvgSpeed->SetMaxLength( 0 ); 
	gbFieldContent->Add( numAvgSpeed, wxGBPosition( 6, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	
	grpFieldContent->Add( gbFieldContent, 1, wxEXPAND, 5 );
	
	
	sbField->Add( grpFieldContent, 0, wxALL|wxEXPAND, 3 );
	
	
	pFieldProperties->SetSizer( sbField );
	pFieldProperties->Layout();
	sbField->Fit( pFieldProperties );
	bSizer106->Add( pFieldProperties, 1, wxEXPAND | wxALL, 3 );
	
	pBoundingBoxes = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbOuterBox;
	sbOuterBox = new wxStaticBoxSizer( new wxStaticBox( pBoundingBoxes, wxID_ANY, wxT("Outer Box") ), wxVERTICAL );
	
	wxFlexGridSizer* fgOutBox;
	fgOutBox = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgOutBox->SetFlexibleDirection( wxBOTH );
	fgOutBox->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgOutBox->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblOuterMinimum = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Minimum:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterMinimum->Wrap( -1 );
	fgOutBox->Add( lblOuterMinimum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblOuterMaximum = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Maximum:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterMaximum->Wrap( -1 );
	fgOutBox->Add( lblOuterMaximum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblOuterX = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterX->Wrap( -1 );
	fgOutBox->Add( lblOuterX, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtOuterMinimumX = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgOutBox->Add( txtOuterMinimumX, 0, wxALL, 3 );
	
	txtOuterMaximumX = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgOutBox->Add( txtOuterMaximumX, 0, wxALL, 3 );
	
	lblOuterY = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterY->Wrap( -1 );
	fgOutBox->Add( lblOuterY, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtOuterMinimumY = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgOutBox->Add( txtOuterMinimumY, 0, wxALL, 3 );
	
	txtOuterMaximumY = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgOutBox->Add( txtOuterMaximumY, 0, wxALL, 3 );
	
	lblOuterZ = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterZ->Wrap( -1 );
	fgOutBox->Add( lblOuterZ, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtOuterMinimumZ = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgOutBox->Add( txtOuterMinimumZ, 0, wxALL, 3 );
	
	txtOuterMaximumZ = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgOutBox->Add( txtOuterMaximumZ, 0, wxALL, 3 );
	
	
	sbOuterBox->Add( fgOutBox, 0, wxEXPAND, 3 );
	
	
	bSizer105->Add( sbOuterBox, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbInnerBox;
	sbInnerBox = new wxStaticBoxSizer( new wxStaticBox( pBoundingBoxes, wxID_ANY, wxT("Inner Box") ), wxVERTICAL );
	
	chkInnerBoxEnable = new wxCheckBox( pBoundingBoxes, wxID_ANY, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	sbInnerBox->Add( chkInnerBoxEnable, 0, wxALL, 3 );
	
	wxFlexGridSizer* fgInnerBox;
	fgInnerBox = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgInnerBox->SetFlexibleDirection( wxBOTH );
	fgInnerBox->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgInnerBox->Add( 0, 0, 1, wxEXPAND, 3 );
	
	lblInnerMinimum = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Minimum:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	lblInnerMinimum->Wrap( -1 );
	fgInnerBox->Add( lblInnerMinimum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblInnerMaximum = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Maximum:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	lblInnerMaximum->Wrap( -1 );
	fgInnerBox->Add( lblInnerMaximum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblInnerX = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblInnerX->Wrap( -1 );
	fgInnerBox->Add( lblInnerX, 0, wxALIGN_CENTER|wxALL, 3 );
	
	numInnerBoxMinX = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMinX->SetMaxLength( 0 ); 
	fgInnerBox->Add( numInnerBoxMinX, 0, wxALL, 3 );
	
	numInnerBoxMaxX = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMaxX->SetMaxLength( 0 ); 
	fgInnerBox->Add( numInnerBoxMaxX, 0, wxALL, 3 );
	
	lblInnerY = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblInnerY->Wrap( -1 );
	fgInnerBox->Add( lblInnerY, 0, wxALIGN_CENTER|wxALL, 3 );
	
	numInnerBoxMinY = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMinY->SetMaxLength( 0 ); 
	fgInnerBox->Add( numInnerBoxMinY, 0, wxALL, 3 );
	
	numInnerBoxMaxY = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMaxY->SetMaxLength( 0 ); 
	fgInnerBox->Add( numInnerBoxMaxY, 0, wxALL, 3 );
	
	lblInnerZ = new wxStaticText( pBoundingBoxes, wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblInnerZ->Wrap( -1 );
	fgInnerBox->Add( lblInnerZ, 0, wxALIGN_CENTER|wxALL, 3 );
	
	numInnerBoxMinZ = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMinZ->SetMaxLength( 0 ); 
	fgInnerBox->Add( numInnerBoxMinZ, 0, wxALL, 3 );
	
	numInnerBoxMaxZ = new wxTextCtrl( pBoundingBoxes, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMaxZ->SetMaxLength( 0 ); 
	fgInnerBox->Add( numInnerBoxMaxZ, 0, wxALL, 3 );
	
	
	sbInnerBox->Add( fgInnerBox, 0, wxEXPAND, 3 );
	
	
	bSizer105->Add( sbInnerBox, 0, wxEXPAND, 3 );
	
	
	pBoundingBoxes->SetSizer( bSizer105 );
	pBoundingBoxes->Layout();
	bSizer105->Fit( pBoundingBoxes );
	bSizer106->Add( pBoundingBoxes, 1, wxEXPAND | wxALL, 3 );
	
	
	bSizer59->Add( bSizer106, 0, wxEXPAND, 5 );
	
	grpButtons = new wxStdDialogButtonSizer();
	grpButtonsOK = new wxButton( this, wxID_OK );
	grpButtons->AddButton( grpButtonsOK );
	grpButtonsCancel = new wxButton( this, wxID_CANCEL );
	grpButtons->AddButton( grpButtonsCancel );
	grpButtons->Realize();
	
	bSizer59->Add( grpButtons, 1, wxALIGN_RIGHT|wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer59 );
	this->Layout();
	bSizer59->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgAsteroidFieldEditor::~dlgAsteroidFieldEditor()
{
}

BEGIN_EVENT_TABLE( dlgMissionSpecsEditor, wxDialog )
	EVT_CLOSE( dlgMissionSpecsEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgMissionSpecsEditor::dlgMissionSpecsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 800,590 ), wxSize( 800,590 ) );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer10->AddGrowableCol( 0 );
	fgSizer10->AddGrowableCol( 1 );
	fgSizer10->AddGrowableCol( 2 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer222;
	sbSizer222 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Meta Info") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer2->AddGrowableCol( 1 );
	fgSizer2->AddGrowableRow( 0 );
	fgSizer2->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblTitle = new wxStaticText( this, wxID_ANY, wxT("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTitle->Wrap( 0 );
	fgSizer2->Add( lblTitle, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3 );
	
	txtTitle = new wxTextCtrl( this, wxID_ANY, wxT("Untitled"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTitle->SetMaxLength( 0 ); 
	fgSizer2->Add( txtTitle, 1, wxALL|wxEXPAND, 3 );
	
	lblDesigner = new wxStaticText( this, wxID_ANY, wxT("Designer:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDesigner->Wrap( 0 );
	fgSizer2->Add( lblDesigner, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3 );
	
	txtDesigner = new wxTextCtrl( this, wxID_ANY, wxT("wxFRED"), wxDefaultPosition, wxDefaultSize, 0 );
	txtDesigner->SetMaxLength( 0 ); 
	fgSizer2->Add( txtDesigner, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	lblCreated = new wxStaticText( this, wxID_ANY, wxT("Created:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCreated->Wrap( -1 );
	fgSizer2->Add( lblCreated, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtCreated = new wxStaticText( this, wxID_ANY, wxT("xx/xx/xx at 00:00 AM"), wxDefaultPosition, wxDefaultSize, 0 );
	txtCreated->Wrap( -1 );
	fgSizer2->Add( txtCreated, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblModified = new wxStaticText( this, wxID_ANY, wxT("Last Modified:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblModified->Wrap( -1 );
	fgSizer2->Add( lblModified, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtModified = new wxStaticText( this, wxID_ANY, wxT("xx/xx/xx at 00:00 AM"), wxDefaultPosition, wxDefaultSize, 0 );
	txtModified->Wrap( -1 );
	fgSizer2->Add( txtModified, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer222->Add( fgSizer2, 0, wxALL|wxEXPAND, 0 );
	
	
	bSizer57->Add( sbSizer222, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer33;
	sbSizer33 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mission Type") ), wxVERTICAL );
	
	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxVERTICAL );
	
	optSinglePlayer = new wxRadioButton( this, wxID_ANY, wxT("Single Player"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optSinglePlayer->SetValue( true ); 
	bSizer55->Add( optSinglePlayer, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND|wxRIGHT, 2 );
	
	optMultiPlayer = new wxRadioButton( this, wxID_ANY, wxT("Multi Player"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer55->Add( optMultiPlayer, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	optTraining = new wxRadioButton( this, wxID_ANY, wxT("Training"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer55->Add( optTraining, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	
	bSizer54->Add( bSizer55, 1, wxEXPAND, 5 );
	
	pnlMultiplayer = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlMultiplayer->Enable( false );
	
	wxBoxSizer* bSizer561;
	bSizer561 = new wxBoxSizer( wxVERTICAL );
	
	optCooperative = new wxRadioButton( pnlMultiplayer, wxID_ANY, wxT("Cooperative"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optCooperative->SetValue( true ); 
	optCooperative->Enable( false );
	
	bSizer561->Add( optCooperative, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	optTeamVsTeam = new wxRadioButton( pnlMultiplayer, wxID_ANY, wxT("Team Vs. Team"), wxDefaultPosition, wxDefaultSize, 0 );
	optTeamVsTeam->Enable( false );
	
	bSizer561->Add( optTeamVsTeam, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	optDogfight = new wxRadioButton( pnlMultiplayer, wxID_ANY, wxT("Dogfight"), wxDefaultPosition, wxDefaultSize, 0 );
	optDogfight->Enable( false );
	
	bSizer561->Add( optDogfight, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	
	pnlMultiplayer->SetSizer( bSizer561 );
	pnlMultiplayer->Layout();
	bSizer561->Fit( pnlMultiplayer );
	bSizer54->Add( pnlMultiplayer, 1, wxEXPAND | wxALL, 5 );
	
	
	sbSizer33->Add( bSizer54, 1, wxEXPAND, 0 );
	
	
	bSizer57->Add( sbSizer33, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer212;
	sbSizer212 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Multiplayer") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer5->AddGrowableCol( 1 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText75 = new wxStaticText( this, wxID_ANY, wxT("Max Respawns:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText75->Wrap( -1 );
	fgSizer5->Add( m_staticText75, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxLEFT, 3 );
	
	spnMaxRespawns = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
	spnMaxRespawns->Enable( false );
	
	fgSizer5->Add( spnMaxRespawns, 0, wxALL, 3 );
	
	m_staticText76 = new wxStaticText( this, wxID_ANY, wxT("Max Respawn Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText76->Wrap( -1 );
	fgSizer5->Add( m_staticText76, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxLEFT, 3 );
	
	spnMaxRespawnDelay = new wxSpinCtrl( this, wxID_ANY, wxT("-1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1, 999, 0 );
	spnMaxRespawnDelay->Enable( false );
	
	fgSizer5->Add( spnMaxRespawnDelay, 0, wxALL, 3 );
	
	
	sbSizer212->Add( fgSizer5, 1, wxSHAPED, 0 );
	
	
	bSizer57->Add( sbSizer212, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer17;
	sbSizer17 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Squadron Reassignment") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer3->AddGrowableCol( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText74 = new wxStaticText( this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText74->Wrap( -1 );
	fgSizer3->Add( m_staticText74, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 3 );
	
	txtSquadronName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSquadronName->SetMaxLength( 0 ); 
	fgSizer3->Add( txtSquadronName, 0, wxALL|wxEXPAND, 3 );
	
	btnSquadronLogo = new wxButton( this, wxID_ANY, wxT("Logo"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	fgSizer3->Add( btnSquadronLogo, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 3 );
	
	txtSquadronLogo = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSquadronLogo->SetMaxLength( 0 ); 
	fgSizer3->Add( txtSquadronLogo, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer17->Add( fgSizer3, 1, wxEXPAND, 0 );
	
	
	bSizer57->Add( sbSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	
	fgSizer10->Add( bSizer57, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer78;
	bSizer78 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer21;
	sbSizer21 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Support Ships") ), wxVERTICAL );
	
	wxBoxSizer* bSizer571;
	bSizer571 = new wxBoxSizer( wxVERTICAL );
	
	chkDisallowSupportShips = new wxCheckBox( this, wxID_ANY, wxT("Disallow Support Ships"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer571->Add( chkDisallowSupportShips, 0, wxALL, 3 );
	
	chkSupportShipsRepairHull = new wxCheckBox( this, wxID_ANY, wxT("Support Ships repair hull"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer571->Add( chkSupportShipsRepairHull, 0, wxALL, 3 );
	
	
	sbSizer21->Add( bSizer571, 0, 0, 5 );
	
	pnlRepairHull = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlRepairHull->Enable( false );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer6->AddGrowableCol( 2 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer6->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblRepairCeiling = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("Repair Ceiling:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRepairCeiling->Wrap( -1 );
	lblRepairCeiling->Enable( false );
	
	fgSizer6->Add( lblRepairCeiling, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	fgSizer6->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblHullRepairCeiling = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("Hull:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHullRepairCeiling->Wrap( -1 );
	lblHullRepairCeiling->Enable( false );
	
	fgSizer6->Add( lblHullRepairCeiling, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALIGN_RIGHT|wxLEFT, 4 );
	
	spnHullRepairCeiling = new wxSpinCtrl( pnlRepairHull, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 96,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	spnHullRepairCeiling->Enable( false );
	
	fgSizer6->Add( spnHullRepairCeiling, 0, wxALL, 3 );
	
	lblHullPercent = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("%"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHullPercent->Wrap( -1 );
	lblHullPercent->Enable( false );
	
	fgSizer6->Add( lblHullPercent, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
	
	lblSubsystemRepairCeiling = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("Subsystem:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSubsystemRepairCeiling->Wrap( -1 );
	lblSubsystemRepairCeiling->Enable( false );
	
	fgSizer6->Add( lblSubsystemRepairCeiling, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxLEFT, 4 );
	
	spnSubsystemRepairCeiling = new wxSpinCtrl( pnlRepairHull, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 96,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	spnSubsystemRepairCeiling->Enable( false );
	
	fgSizer6->Add( spnSubsystemRepairCeiling, 0, wxALL, 3 );
	
	lblSubstemPercent = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("%"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSubstemPercent->Wrap( -1 );
	lblSubstemPercent->Enable( false );
	
	fgSizer6->Add( lblSubstemPercent, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
	
	
	pnlRepairHull->SetSizer( fgSizer6 );
	pnlRepairHull->Layout();
	fgSizer6->Fit( pnlRepairHull );
	sbSizer21->Add( pnlRepairHull, 1, wxEXPAND | wxALL, 5 );
	
	
	bSizer78->Add( sbSizer21, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer22;
	sbSizer22 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Ship Trails") ), wxVERTICAL );
	
	wxBoxSizer* bSizer651;
	bSizer651 = new wxBoxSizer( wxVERTICAL );
	
	chkToggleNebula = new wxCheckBox( this, wxID_ANY, wxT("Toggle (off in nebula; on elsewhere)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer651->Add( chkToggleNebula, 0, wxALL, 3 );
	
	wxBoxSizer* bSizer66;
	bSizer66 = new wxBoxSizer( wxHORIZONTAL );
	
	chkMinimumTrailSpeed = new wxCheckBox( this, wxID_ANY, wxT("Minimum Speed to display"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer66->Add( chkMinimumTrailSpeed, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnMinimumTrailSpeed = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 45 );
	spnMinimumTrailSpeed->Enable( false );
	
	bSizer66->Add( spnMinimumTrailSpeed, 0, wxALL, 3 );
	
	
	bSizer651->Add( bSizer66, 1, wxEXPAND, 5 );
	
	
	sbSizer22->Add( bSizer651, 0, wxALIGN_CENTER_HORIZONTAL, 0 );
	
	
	bSizer78->Add( sbSizer22, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer211;
	sbSizer211 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Built-in Command Messages") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer7->AddGrowableCol( 1 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText71 = new wxStaticText( this, wxID_ANY, wxT("Sender:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText71->Wrap( -1 );
	fgSizer7->Add( m_staticText71, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	cboMessageSender = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer7->Add( cboMessageSender, 1, wxALL|wxEXPAND, 3 );
	
	m_staticText72 = new wxStaticText( this, wxID_ANY, wxT("Persona:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText72->Wrap( -1 );
	fgSizer7->Add( m_staticText72, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 0 );
	
	wxArrayString cboPersonaChoices;
	cboPersona = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboPersonaChoices, 0 );
	cboPersona->SetSelection( 0 );
	fgSizer7->Add( cboPersona, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer211->Add( fgSizer7, 0, wxEXPAND, 5 );
	
	
	bSizer78->Add( sbSizer211, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer221;
	sbSizer221 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Music and Sound") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer8->AddGrowableCol( 1 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText73 = new wxStaticText( this, wxID_ANY, wxT("Default:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText73->Wrap( -1 );
	fgSizer8->Add( m_staticText73, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxArrayString cboMusicChoices;
	cboMusic = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboMusicChoices, 0 );
	cboMusic->SetSelection( 0 );
	fgSizer8->Add( cboMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText741 = new wxStaticText( this, wxID_ANY, wxT("If Music pack is present:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText741->Wrap( 70 );
	fgSizer8->Add( m_staticText741, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	cboMusicPackPresent = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer8->Add( cboMusicPackPresent, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	
	sbSizer221->Add( fgSizer8, 0, wxEXPAND, 5 );
	
	btnSoundEnvironment = new wxButton( this, wxID_ANY, wxT("Sound Environment"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer221->Add( btnSoundEnvironment, 0, wxALL|wxEXPAND, 3 );
	
	
	bSizer78->Add( sbSizer221, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer10->Add( bSizer78, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer68;
	bSizer68 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer23;
	sbSizer23 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Flags") ), wxVERTICAL );
	
	wxString m_checkList4Choices[] = { wxT("All Teams at War"), wxT("Red Alert Mission"), wxT("Scramble Mission"), wxT("Disallow Promotion/Badges"), wxT("Disable Built-In Messages"), wxT("Disable Built-In Command Messages"), wxT("No Traitor"), wxT("All Ships Beam-Freed by Default"), wxT("Allow Daisy-Chained Docking"), wxT("No Briefing"), wxT("Toggle Debriefing (On/Off)"), wxT("Use Autopilot Cinematics"), wxT("Deactivate Hardcoded Autopilot"), wxT("Player Starts Under AI Control (NO MULTI)"), wxT("2D Mission") };
	int m_checkList4NChoices = sizeof( m_checkList4Choices ) / sizeof( wxString );
	m_checkList4 = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,140 ), m_checkList4NChoices, m_checkList4Choices, wxLB_ALWAYS_SB|wxLB_HSCROLL );
	sbSizer23->Add( m_checkList4, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer68->Add( sbSizer23, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer231;
	sbSizer231 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("AI Options") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer9->AddGrowableCol( 1 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText771 = new wxStaticText( this, wxID_ANY, wxT("AI Profile:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText771->Wrap( -1 );
	fgSizer9->Add( m_staticText771, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3 );
	
	wxArrayString cboAIProfileChoices;
	cboAIProfile = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboAIProfileChoices, 0 );
	cboAIProfile->SetSelection( 0 );
	fgSizer9->Add( cboAIProfile, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	
	sbSizer231->Add( fgSizer9, 1, wxEXPAND, 5 );
	
	
	bSizer68->Add( sbSizer231, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Loading Screen") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer4->AddGrowableCol( 1 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnLoadingScreen640x480 = new wxButton( this, wxID_ANY, wxT("640x480"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	fgSizer4->Add( btnLoadingScreen640x480, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	txtLoadingScreen640x480 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtLoadingScreen640x480->SetMaxLength( 0 ); 
	fgSizer4->Add( txtLoadingScreen640x480, 0, wxALL|wxEXPAND, 3 );
	
	btnLoadingScreen1024x768 = new wxButton( this, wxID_ANY, wxT("1024x768"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	fgSizer4->Add( btnLoadingScreen1024x768, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	txtLoadingScreen1024x768 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtLoadingScreen1024x768->SetMaxLength( 0 ); 
	fgSizer4->Add( txtLoadingScreen1024x768, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer7->Add( fgSizer4, 1, wxEXPAND, 0 );
	
	
	bSizer68->Add( sbSizer7, 0, wxALL|wxEXPAND, 3 );
	
	btnCustomWingNames = new wxButton( this, wxID_ANY, wxT("Custom Wing Names"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer68->Add( btnCustomWingNames, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText159 = new wxStaticText( this, wxID_ANY, wxT("Player Entry Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText159->Wrap( -1 );
	bSizer91->Add( m_staticText159, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinCtrl18 = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 30, 0 );
	bSizer91->Add( m_spinCtrl18, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer68->Add( bSizer91, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer10->Add( bSizer68, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer71->Add( fgSizer10, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer73;
	bSizer73 = new wxBoxSizer( wxVERTICAL );
	
	lblMissionDescription = new wxStaticText( this, wxID_ANY, wxT("Mission Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMissionDescription->Wrap( -1 );
	bSizer73->Add( lblMissionDescription, 0, wxALIGN_BOTTOM|wxLEFT, 9 );
	
	txtMissionDescription = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtMissionDescription->SetMaxLength( 0 ); 
	bSizer73->Add( txtMissionDescription, 1, wxALL|wxEXPAND, 3 );
	
	lblDesignerNotes = new wxStaticText( this, wxID_ANY, wxT("Designer Notes:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDesignerNotes->Wrap( -1 );
	bSizer73->Add( lblDesignerNotes, 0, wxLEFT, 9 );
	
	txtDesignerNotes = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtDesignerNotes->SetMaxLength( 0 ); 
	bSizer73->Add( txtDesignerNotes, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer71->Add( bSizer73, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer71 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgMissionSpecsEditor::~dlgMissionSpecsEditor()
{
}

BEGIN_EVENT_TABLE( dlgSoundEnvironment, wxDialog )
	EVT_BUTTON( wxID_ANY, dlgSoundEnvironment::_wxFB_OnOK )
	EVT_BUTTON( wxID_ANY, dlgSoundEnvironment::_wxFB_OnCancel )
END_EVENT_TABLE()

dlgSoundEnvironment::dlgSoundEnvironment( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer116;
	bSizer116 = new wxBoxSizer( wxHORIZONTAL );
	
	wxGridBagSizer* gbSizer11;
	gbSizer11 = new wxGridBagSizer( 0, 0 );
	gbSizer11->SetFlexibleDirection( wxBOTH );
	gbSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblEnvironment = new wxStaticText( this, wxID_ANY, wxT("Environment:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblEnvironment->Wrap( -1 );
	gbSizer11->Add( lblEnvironment, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboEnvironment = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer11->Add( cboEnvironment, wxGBPosition( 0, 1 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 3 );
	
	lblVolume = new wxStaticText( this, wxID_ANY, wxT("Volume:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVolume->Wrap( -1 );
	gbSizer11->Add( lblVolume, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnVolume = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer11->Add( spnVolume, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblDamping = new wxStaticText( this, wxID_ANY, wxT("Damping:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDamping->Wrap( -1 );
	gbSizer11->Add( lblDamping, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDamping = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer11->Add( spnDamping, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblDecayTime = new wxStaticText( this, wxID_ANY, wxT("Decay Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDecayTime->Wrap( -1 );
	gbSizer11->Add( lblDecayTime, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDecayTime = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer11->Add( spnDecayTime, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblDecaySeconds = new wxStaticText( this, wxID_ANY, wxT("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDecaySeconds->Wrap( -1 );
	gbSizer11->Add( lblDecaySeconds, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer116->Add( gbSizer11, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer117;
	bSizer117 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer118;
	bSizer118 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer118->Add( btnOK, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer118->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer117->Add( bSizer118, 1, wxALIGN_RIGHT, 5 );
	
	wxStaticBoxSizer* sbSizer45;
	sbSizer45 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Preview") ), wxHORIZONTAL );
	
	m_bpButton7 = new wxBitmapButton( this, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 24,24 ), wxBU_AUTODRAW );
	sbSizer45->Add( m_bpButton7, 0, wxALIGN_CENTER|wxALL, 3 );
	
	m_filePicker2 = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxSize( -1,24 ), wxFLP_FILE_MUST_EXIST|wxFLP_OPEN );
	sbSizer45->Add( m_filePicker2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer117->Add( sbSizer45, 1, wxALIGN_RIGHT|wxLEFT|wxTOP, 5 );
	
	
	bSizer116->Add( bSizer117, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer116 );
	this->Layout();
	bSizer116->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgSoundEnvironment::~dlgSoundEnvironment()
{
}

BEGIN_EVENT_TABLE( frmBriefingEditor, wxFrame )
	EVT_CLOSE( frmBriefingEditor::_wxFB_OnClose )
END_EVENT_TABLE()

frmBriefingEditor::frmBriefingEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menubar8 = new wxMenuBar( 0 );
	mnuSelectTeam = new wxMenu();
	wxMenuItem* mnuTeam1;
	mnuTeam1 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 1") ) , wxEmptyString, wxITEM_RADIO );
	mnuSelectTeam->Append( mnuTeam1 );
	
	wxMenuItem* mnuTeam2;
	mnuTeam2 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 2") ) , wxEmptyString, wxITEM_RADIO );
	mnuSelectTeam->Append( mnuTeam2 );
	
	m_menubar8->Append( mnuSelectTeam, wxT("Select Team") ); 
	
	mnuOptions = new wxMenu();
	wxMenuItem* mnuBalanceTeams;
	mnuBalanceTeams = new wxMenuItem( mnuOptions, wxID_ANY, wxString( wxT("Balance Teams") ) , wxEmptyString, wxITEM_NORMAL );
	mnuOptions->Append( mnuBalanceTeams );
	
	m_menubar8->Append( mnuOptions, wxT("Options") ); 
	
	this->SetMenuBar( m_menubar8 );
	
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxVERTICAL );
	
	m_panel13 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer1031;
	bSizer1031 = new wxBoxSizer( wxVERTICAL );
	
	wxGridBagSizer* gbSizer9;
	gbSizer9 = new wxGridBagSizer( 0, 0 );
	gbSizer9->SetFlexibleDirection( wxBOTH );
	gbSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxVERTICAL );
	
	lblStage = new wxStaticText( m_panel13, wxID_ANY, wxT("No stages"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStage->Wrap( -1 );
	bSizer106->Add( lblStage, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer107;
	bSizer107 = new wxBoxSizer( wxHORIZONTAL );
	
	lblCameraTransisitonTime = new wxStaticText( m_panel13, wxID_ANY, wxT("Camera Transition Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCameraTransisitonTime->Wrap( -1 );
	bSizer107->Add( lblCameraTransisitonTime, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinCtrl53 = new wxSpinCtrl( m_panel13, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxSP_ARROW_KEYS, 0, 999999, 505 );
	bSizer107->Add( m_spinCtrl53, 0, wxALL, 3 );
	
	m_staticText192 = new wxStaticText( m_panel13, wxID_ANY, wxT("ms"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText192->Wrap( -1 );
	bSizer107->Add( m_staticText192, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer106->Add( bSizer107, 0, wxEXPAND, 5 );
	
	chkCutToNextStage = new wxCheckBox( m_panel13, wxID_ANY, wxT("Cut to Next Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer106->Add( chkCutToNextStage, 0, wxALL, 3 );
	
	chkCutToPreviousStage = new wxCheckBox( m_panel13, wxID_ANY, wxT("Cut to Previous Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer106->Add( chkCutToPreviousStage, 0, wxALL, 3 );
	
	
	gbSizer9->Add( bSizer106, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	lblText = new wxStaticText( m_panel13, wxID_ANY, wxT("Briefing Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblText->Wrap( -1 );
	gbSizer9->Add( lblText, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_BOTTOM|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_textCtrl75 = new wxTextCtrl( m_panel13, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,60 ), wxTE_MULTILINE );
	gbSizer9->Add( m_textCtrl75, wxGBPosition( 2, 0 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer109;
	bSizer109 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer110;
	bSizer110 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxHORIZONTAL );
	
	btnPreviousStage = new wxButton( m_panel13, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizer105->Add( btnPreviousStage, 0, wxALL|wxEXPAND, 2 );
	
	btnNextStage = new wxButton( m_panel13, wxID_ANY, wxT("Next"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizer105->Add( btnNextStage, 0, wxALL|wxEXPAND, 2 );
	
	
	bSizer110->Add( bSizer105, 1, wxEXPAND, 3 );
	
	btnAddStage = new wxButton( m_panel13, wxID_ANY, wxT("Add Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer110->Add( btnAddStage, 1, wxALL|wxEXPAND, 2 );
	
	btnInsertStage = new wxButton( m_panel13, wxID_ANY, wxT("Insert Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer110->Add( btnInsertStage, 1, wxALL|wxEXPAND, 2 );
	
	btnDeleteStage = new wxButton( m_panel13, wxID_ANY, wxT("Delete Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer110->Add( btnDeleteStage, 1, wxALL|wxEXPAND, 2 );
	
	
	bSizer109->Add( bSizer110, 1, 0, 5 );
	
	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );
	
	btnSaveView = new wxButton( m_panel13, wxID_ANY, wxT("Save View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer111->Add( btnSaveView, 1, wxALL|wxEXPAND, 2 );
	
	btnGoToView = new wxButton( m_panel13, wxID_ANY, wxT("Go To View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer111->Add( btnGoToView, 1, wxALL|wxEXPAND, 2 );
	
	btnCopyView = new wxButton( m_panel13, wxID_ANY, wxT("Copy View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer111->Add( btnCopyView, 1, wxALL|wxEXPAND, 2 );
	
	btnPasteView = new wxButton( m_panel13, wxID_ANY, wxT("Paste View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer111->Add( btnPasteView, 1, wxALL|wxEXPAND, 2 );
	
	
	bSizer109->Add( bSizer111, 1, wxEXPAND, 5 );
	
	
	gbSizer9->Add( bSizer109, wxGBPosition( 0, 1 ), wxGBSpan( 2, 1 ), wxALIGN_RIGHT, 3 );
	
	
	gbSizer9->AddGrowableCol( 0 );
	gbSizer9->AddGrowableCol( 1 );
	
	bSizer1031->Add( gbSizer9, 0, wxEXPAND|wxRIGHT, 3 );
	
	wxBoxSizer* bSizer112;
	bSizer112 = new wxBoxSizer( wxHORIZONTAL );
	
	lblVoiceFile = new wxStaticText( m_panel13, wxID_ANY, wxT("Voice File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVoiceFile->Wrap( -1 );
	bSizer112->Add( lblVoiceFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_filePicker1 = new wxFilePickerCtrl( m_panel13, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	bSizer112->Add( m_filePicker1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayVoice = new wxBitmapButton( m_panel13, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	bSizer112->Add( btnPlayVoice, 0, wxALL, 3 );
	
	
	bSizer1031->Add( bSizer112, 0, 0, 5 );
	
	wxStaticBoxSizer* sbSizer43;
	sbSizer43 = new wxStaticBoxSizer( new wxStaticBox( m_panel13, wxID_ANY, wxT("Briefing Music") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer40;
	fgSizer40 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer40->SetFlexibleDirection( wxBOTH );
	fgSizer40->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDefaultMusic = new wxStaticText( m_panel13, wxID_ANY, wxT("Default:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDefaultMusic->Wrap( -1 );
	fgSizer40->Add( lblDefaultMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString m_choice42Choices[] = { wxT("None") };
	int m_choice42NChoices = sizeof( m_choice42Choices ) / sizeof( wxString );
	m_choice42 = new wxChoice( m_panel13, wxID_ANY, wxDefaultPosition, wxSize( 120,-1 ), m_choice42NChoices, m_choice42Choices, 0 );
	m_choice42->SetSelection( 0 );
	fgSizer40->Add( m_choice42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayMusic = new wxBitmapButton( m_panel13, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	fgSizer40->Add( btnPlayMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText196 = new wxStaticText( m_panel13, wxID_ANY, wxT("If music pack is present:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText196->Wrap( 70 );
	fgSizer40->Add( m_staticText196, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString m_choice43Choices[] = { wxT("None") };
	int m_choice43NChoices = sizeof( m_choice43Choices ) / sizeof( wxString );
	m_choice43 = new wxChoice( m_panel13, wxID_ANY, wxDefaultPosition, wxSize( 120,-1 ), m_choice43NChoices, m_choice43Choices, 0 );
	m_choice43->SetSelection( 0 );
	fgSizer40->Add( m_choice43, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayMusicFromPack = new wxBitmapButton( m_panel13, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	fgSizer40->Add( btnPlayMusicFromPack, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer43->Add( fgSizer40, 1, wxEXPAND, 5 );
	
	
	bSizer1031->Add( sbSizer43, 0, 0, 5 );
	
	wxBoxSizer* bSizer114;
	bSizer114 = new wxBoxSizer( wxVERTICAL );
	
	lblUsageFormula = new wxStaticText( m_panel13, wxID_ANY, wxT("Usage Formula:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblUsageFormula->Wrap( -1 );
	bSizer114->Add( lblUsageFormula, 0, wxALL, 5 );
	
	m_treeCtrl9 = new wxTreeCtrl( m_panel13, wxID_ANY, wxDefaultPosition, wxSize( -1,60 ), wxTR_DEFAULT_STYLE );
	bSizer114->Add( m_treeCtrl9, 0, wxEXPAND, 3 );
	
	chkDrawLines = new wxCheckBox( m_panel13, wxID_ANY, wxT("Draw Lines Between Marked Icons"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer114->Add( chkDrawLines, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer1031->Add( bSizer114, 0, wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	wxStaticBoxSizer* sbSizer44;
	sbSizer44 = new wxStaticBoxSizer( new wxStaticBox( m_panel13, wxID_ANY, wxT("Selected Icon Info") ), wxHORIZONTAL );
	
	wxGridBagSizer* gbSizer10;
	gbSizer10 = new wxGridBagSizer( 0, 0 );
	gbSizer10->SetFlexibleDirection( wxBOTH );
	gbSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblIconLabel = new wxStaticText( m_panel13, wxID_ANY, wxT("Label:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconLabel->Wrap( -1 );
	gbSizer10->Add( lblIconLabel, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtIconLabel = new wxTextCtrl( m_panel13, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	gbSizer10->Add( txtIconLabel, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblIconImage = new wxStaticText( m_panel13, wxID_ANY, wxT("Icon Image:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconImage->Wrap( -1 );
	gbSizer10->Add( lblIconImage, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboIconImage = new wxComboBox( m_panel13, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 ); 
	gbSizer10->Add( cboIconImage, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblShipType = new wxStaticText( m_panel13, wxID_ANY, wxT("Ship Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipType->Wrap( -1 );
	gbSizer10->Add( lblShipType, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboShipType = new wxComboBox( m_panel13, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 ); 
	gbSizer10->Add( cboShipType, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblIconTeam = new wxStaticText( m_panel13, wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconTeam->Wrap( -1 );
	gbSizer10->Add( lblIconTeam, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_comboBox13 = new wxComboBox( m_panel13, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 ); 
	gbSizer10->Add( m_comboBox13, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblIconId = new wxStaticText( m_panel13, wxID_ANY, wxT("ID:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconId->Wrap( -1 );
	gbSizer10->Add( lblIconId, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtIconID = new wxTextCtrl( m_panel13, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer10->Add( txtIconID, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer1141;
	bSizer1141 = new wxBoxSizer( wxVERTICAL );
	
	chkHighlightIcon = new wxCheckBox( m_panel13, wxID_ANY, wxT("Highlight"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1141->Add( chkHighlightIcon, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	chkChangeLocally = new wxCheckBox( m_panel13, wxID_ANY, wxT("Change Locally"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1141->Add( chkChangeLocally, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	chkFlipIconLR = new wxCheckBox( m_panel13, wxID_ANY, wxT("Flip Icon (L/R)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1141->Add( chkFlipIconLR, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	gbSizer10->Add( bSizer1141, wxGBPosition( 1, 3 ), wxGBSpan( 3, 1 ), wxEXPAND, 5 );
	
	btnMakeIcon = new wxButton( m_panel13, wxID_ANY, wxT("Make Icon"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer10->Add( btnMakeIcon, wxGBPosition( 0, 4 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	btnDeleteIcon = new wxButton( m_panel13, wxID_ANY, wxT("Delete Icon"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer10->Add( btnDeleteIcon, wxGBPosition( 1, 4 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	btnPropagate = new wxButton( m_panel13, wxID_ANY, wxT("Propagate"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer10->Add( btnPropagate, wxGBPosition( 2, 4 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	
	sbSizer44->Add( gbSizer10, 0, wxEXPAND, 3 );
	
	
	bSizer1031->Add( sbSizer44, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer115;
	bSizer115 = new wxBoxSizer( wxVERTICAL );
	
	lblIconText = new wxStaticText( m_panel13, wxID_ANY, wxT("Icon Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconText->Wrap( -1 );
	bSizer115->Add( lblIconText, 0, wxALL, 3 );
	
	txtIconText = new wxTextCtrl( m_panel13, wxID_ANY, wxT("1\n2\n3"), wxDefaultPosition, wxSize( -1,60 ), wxTE_MULTILINE|wxTE_WORDWRAP );
	bSizer115->Add( txtIconText, 0, wxBOTTOM|wxEXPAND, 4 );
	
	
	bSizer1031->Add( bSizer115, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 3 );
	
	
	m_panel13->SetSizer( bSizer1031 );
	m_panel13->Layout();
	bSizer1031->Fit( m_panel13 );
	bSizer103->Add( m_panel13, 1, wxEXPAND, 0 );
	
	
	this->SetSizer( bSizer103 );
	this->Layout();
	bSizer103->Fit( this );
	
	this->Centre( wxBOTH );
}

frmBriefingEditor::~frmBriefingEditor()
{
}

BEGIN_EVENT_TABLE( frmDebriefingEditor, wxFrame )
	EVT_CLOSE( frmDebriefingEditor::_wxFB_OnClose )
END_EVENT_TABLE()

frmDebriefingEditor::frmDebriefingEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxVERTICAL );
	
	pnlMain = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxHORIZONTAL );
	
	txtStages = new wxStaticText( pnlMain, wxID_ANY, wxT("No stages"), wxDefaultPosition, wxDefaultSize, 0 );
	txtStages->Wrap( -1 );
	bSizer23->Add( txtStages, 2, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPrev = new wxButton( pnlMain, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer23->Add( btnPrev, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnNext = new wxButton( pnlMain, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer23->Add( btnNext, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer22->Add( bSizer23, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAdd = new wxButton( pnlMain, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer24->Add( btnAdd, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnInsert = new wxButton( pnlMain, wxID_ANY, wxT("Insert"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer24->Add( btnInsert, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnDelete = new wxButton( pnlMain, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer24->Add( btnDelete, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer22->Add( bSizer24, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	lblUsageFormula = new wxStaticText( pnlMain, wxID_ANY, wxT("Usage Formula:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblUsageFormula->Wrap( -1 );
	bSizer22->Add( lblUsageFormula, 0, wxALIGN_LEFT|wxALL|wxEXPAND|wxTOP, 3 );
	
	treeUsageFormula = new wxTreeCtrl( pnlMain, wxID_ANY, wxDefaultPosition, wxSize( 250,200 ), wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_MULTIPLE|wxSUNKEN_BORDER );
	bSizer22->Add( treeUsageFormula, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer21->Add( bSizer22, 0, wxALIGN_TOP|wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxHORIZONTAL );
	
	lblVoiceWaveFile = new wxStaticText( pnlMain, wxID_ANY, wxT("Voice Wave File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVoiceWaveFile->Wrap( -1 );
	bSizer26->Add( lblVoiceWaveFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtVoiceWaveFile = new wxTextCtrl( pnlMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtVoiceWaveFile->SetMaxLength( 0 ); 
	bSizer26->Add( txtVoiceWaveFile, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnBrowse = new wxButton( pnlMain, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer26->Add( btnBrowse, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayVoice = new wxBitmapButton( pnlMain, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	bSizer26->Add( btnPlayVoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer25->Add( bSizer26, 0, wxEXPAND|wxALL, 0 );
	
	lblStageText = new wxStaticText( pnlMain, wxID_ANY, wxT("Stage Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStageText->Wrap( -1 );
	bSizer25->Add( lblStageText, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 3 );
	
	txtStageText = new wxTextCtrl( pnlMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 320,100 ), wxTE_MULTILINE );
	txtStageText->SetMaxLength( 0 ); 
	bSizer25->Add( txtStageText, 0, wxALL|wxEXPAND, 3 );
	
	lblRecommendationText = new wxStaticText( pnlMain, wxID_ANY, wxT("Recommendation Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRecommendationText->Wrap( -1 );
	bSizer25->Add( lblRecommendationText, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 3 );
	
	txtRecommendationText = new wxTextCtrl( pnlMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 320,100 ), wxTE_MULTILINE );
	txtRecommendationText->SetMaxLength( 0 ); 
	bSizer25->Add( txtRecommendationText, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer21->Add( bSizer25, 1, wxALIGN_TOP|wxALL|wxEXPAND, 3 );
	
	
	pnlMain->SetSizer( bSizer21 );
	pnlMain->Layout();
	bSizer21->Fit( pnlMain );
	bSizer121->Add( pnlMain, 0, wxEXPAND, 0 );
	
	pnlMusic = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer123;
	bSizer123 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer46;
	sbSizer46 = new wxStaticBoxSizer( new wxStaticBox( pnlMusic, wxID_ANY, wxT("Debriefing Music") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer40;
	fgSizer40 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer40->SetFlexibleDirection( wxBOTH );
	fgSizer40->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblMusicSuccess = new wxStaticText( pnlMusic, wxID_ANY, wxT("Success Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMusicSuccess->Wrap( -1 );
	fgSizer40->Add( lblMusicSuccess, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cbMusicSuccessChoices;
	cbMusicSuccess = new wxChoice( pnlMusic, wxID_ANY, wxDefaultPosition, wxDefaultSize, cbMusicSuccessChoices, 0 );
	cbMusicSuccess->SetSelection( 0 );
	fgSizer40->Add( cbMusicSuccess, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	btnPlaySuccess = new wxBitmapButton( pnlMusic, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer40->Add( btnPlaySuccess, 0, wxALL, 3 );
	
	m_staticText210 = new wxStaticText( pnlMusic, wxID_ANY, wxT("Nuetral Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText210->Wrap( -1 );
	fgSizer40->Add( m_staticText210, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString m_choice45Choices;
	m_choice45 = new wxChoice( pnlMusic, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice45Choices, 0 );
	m_choice45->SetSelection( 0 );
	fgSizer40->Add( m_choice45, 0, wxALL, 5 );
	
	btnPlayNuetral = new wxBitmapButton( pnlMusic, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer40->Add( btnPlayNuetral, 0, wxALL, 3 );
	
	m_staticText211 = new wxStaticText( pnlMusic, wxID_ANY, wxT("Failure Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText211->Wrap( -1 );
	fgSizer40->Add( m_staticText211, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString m_choice46Choices;
	m_choice46 = new wxChoice( pnlMusic, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice46Choices, 0 );
	m_choice46->SetSelection( 0 );
	fgSizer40->Add( m_choice46, 0, wxALL, 5 );
	
	btnPlayFailure = new wxBitmapButton( pnlMusic, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer40->Add( btnPlayFailure, 0, wxALL, 3 );
	
	
	sbSizer46->Add( fgSizer40, 1, wxEXPAND, 5 );
	
	
	bSizer123->Add( sbSizer46, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	pnlMusic->SetSizer( bSizer123 );
	pnlMusic->Layout();
	bSizer123->Fit( pnlMusic );
	bSizer121->Add( pnlMusic, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer121 );
	this->Layout();
	bSizer121->Fit( this );
	mnbDebriefingEditor = new wxMenuBar( 0 );
	mnuSelectTeam = new wxMenu();
	wxMenuItem* mnuSelectTeamTeam1;
	mnuSelectTeamTeam1 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 1") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuSelectTeamTeam1 );
	
	wxMenuItem* mnuSelectTeamTeam2;
	mnuSelectTeamTeam2 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 2") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuSelectTeamTeam2 );
	
	mnbDebriefingEditor->Append( mnuSelectTeam, wxT("Select Team") ); 
	
	mnuOptions = new wxMenu();
	wxMenuItem* mnuOptionsBalanceTeams;
	mnuOptionsBalanceTeams = new wxMenuItem( mnuOptions, wxID_ANY, wxString( wxT("Balance Teams") ) , wxEmptyString, wxITEM_NORMAL );
	mnuOptions->Append( mnuOptionsBalanceTeams );
	
	mnbDebriefingEditor->Append( mnuOptions, wxT("Options") ); 
	
	this->SetMenuBar( mnbDebriefingEditor );
	
	
	this->Centre( wxBOTH );
}

frmDebriefingEditor::~frmDebriefingEditor()
{
}

BEGIN_EVENT_TABLE( frmCommandBriefingEditor, wxFrame )
	EVT_CLOSE( frmCommandBriefingEditor::_wxFB_OnClose )
END_EVENT_TABLE()

frmCommandBriefingEditor::frmCommandBriefingEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menubar6 = new wxMenuBar( 0 );
	mnuSelectTeam = new wxMenu();
	wxMenuItem* mnuTeam1;
	mnuTeam1 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 1") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuTeam1 );
	
	wxMenuItem* mnuTeam2;
	mnuTeam2 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 2") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuTeam2 );
	
	m_menubar6->Append( mnuSelectTeam, wxT("Select Team") ); 
	
	mnuOptions = new wxMenu();
	wxMenuItem* mnuBalanceTeams;
	mnuBalanceTeams = new wxMenuItem( mnuOptions, wxID_ANY, wxString( wxT("Balance Teams") ) , wxEmptyString, wxITEM_NORMAL );
	mnuOptions->Append( mnuBalanceTeams );
	
	m_menubar6->Append( mnuOptions, wxT("Options") ); 
	
	this->SetMenuBar( m_menubar6 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	m_panel5 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxHORIZONTAL );
	
	txtNumCBStages = new wxStaticText( m_panel5, wxID_ANY, wxT("No stages"), wxDefaultPosition, wxDefaultSize, 0 );
	txtNumCBStages->Wrap( -1 );
	bSizer34->Add( txtNumCBStages, 2, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPrev = new wxButton( m_panel5, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer34->Add( btnPrev, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnNext = new wxButton( m_panel5, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer34->Add( btnNext, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer33->Add( bSizer34, 0, wxEXPAND, 3 );
	
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAdd = new wxButton( m_panel5, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer35->Add( btnAdd, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnInsert = new wxButton( m_panel5, wxID_ANY, wxT("Insert"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer35->Add( btnInsert, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxLEFT|wxRIGHT, 3 );
	
	btnDelete = new wxButton( m_panel5, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer35->Add( btnDelete, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer33->Add( bSizer35, 0, wxALIGN_CENTER_HORIZONTAL, 3 );
	
	
	bSizer32->Add( bSizer33, 0, wxALIGN_TOP, 3 );
	
	
	bSizer32->Add( 5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( m_panel5, ID_btnOK, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer36->Add( btnOK, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnCancel = new wxButton( m_panel5, ID_btnCancel, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer36->Add( btnCancel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer32->Add( bSizer36, 1, wxEXPAND, 1 );
	
	
	bSizer31->Add( bSizer32, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 3 );
	
	wxID_STATIC1 = new wxStaticText( m_panel5, wxID_ANY, wxT("Stage Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	wxID_STATIC1->Wrap( -1 );
	bSizer31->Add( wxID_STATIC1, 0, wxALIGN_LEFT|wxALL, 3 );
	
	txtStageText = new wxTextCtrl( m_panel5, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtStageText->SetMaxLength( 0 ); 
	bSizer31->Add( txtStageText, 2, wxALL|wxEXPAND, 3 );
	
	wxFlexGridSizer* fgSizer28;
	fgSizer28 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer28->AddGrowableCol( 1 );
	fgSizer28->SetFlexibleDirection( wxBOTH );
	fgSizer28->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblAniFile = new wxStaticText( m_panel5, wxID_ANY, wxT("Ani File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAniFile->Wrap( -1 );
	fgSizer28->Add( lblAniFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpAniFile = new wxFilePickerCtrl( m_panel5, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer28->Add( fpAniFile, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer28->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblVoiceWaveFile = new wxStaticText( m_panel5, wxID_ANY, wxT("Voice Wave File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVoiceWaveFile->Wrap( -1 );
	fgSizer28->Add( lblVoiceWaveFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpVoiceWave = new wxFilePickerCtrl( m_panel5, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer28->Add( fpVoiceWave, 0, wxALL|wxEXPAND, 3 );
	
	btnVoicePlay = new wxBitmapButton( m_panel5, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer28->Add( btnVoicePlay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer31->Add( fgSizer28, 1, wxEXPAND, 3 );
	
	
	m_panel5->SetSizer( bSizer31 );
	m_panel5->Layout();
	bSizer31->Fit( m_panel5 );
	bSizer30->Add( m_panel5, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer30 );
	this->Layout();
	bSizer30->Fit( this );
	
	this->Centre( wxBOTH );
}

frmCommandBriefingEditor::~frmCommandBriefingEditor()
{
}

BEGIN_EVENT_TABLE( dlgFictionViewer, wxDialog )
	EVT_CLOSE( dlgFictionViewer::_wxFB_OnClose )
END_EVENT_TABLE()

dlgFictionViewer::dlgFictionViewer( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer124;
	bSizer124 = new wxBoxSizer( wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer41;
	fgSizer41 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer41->SetFlexibleDirection( wxBOTH );
	fgSizer41->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblStoryFile = new wxStaticText( this, wxID_ANY, wxT("Story File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStoryFile->Wrap( -1 );
	fgSizer41->Add( lblStoryFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpStoryFile = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Select a story file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer41->Add( fpStoryFile, 0, wxALL, 3 );
	
	lblFontFile = new wxStaticText( this, wxID_ANY, wxT("Font File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFontFile->Wrap( -1 );
	fgSizer41->Add( lblFontFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpFontFile = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Select a font file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer41->Add( fpFontFile, 0, wxALL, 3 );
	
	lblMusic = new wxStaticText( this, wxID_ANY, wxT("Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMusic->Wrap( -1 );
	fgSizer41->Add( lblMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer125;
	bSizer125 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString cbMusicChoices;
	cbMusic = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,23 ), cbMusicChoices, 0 );
	cbMusic->SetSelection( 0 );
	bSizer125->Add( cbMusic, 1, wxALL|wxEXPAND, 3 );
	
	btnPlayMusic = new wxBitmapButton( this, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	bSizer125->Add( btnPlayMusic, 0, wxALL, 2 );
	
	
	fgSizer41->Add( bSizer125, 1, wxEXPAND, 5 );
	
	
	bSizer124->Add( fgSizer41, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer126;
	bSizer126 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer126->Add( btnOK, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, ID_btnCancel, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer126->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer124->Add( bSizer126, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer124 );
	this->Layout();
	bSizer124->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgFictionViewer::~dlgFictionViewer()
{
}

BEGIN_EVENT_TABLE( dlgShieldSystemEditor, wxDialog )
	EVT_CLOSE( dlgShieldSystemEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgShieldSystemEditor::dlgShieldSystemEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("All ships of type") ), wxVERTICAL );
	
	wxArrayString cboShipTypeChoices;
	cboShipType = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboShipTypeChoices, 0 );
	cboShipType->SetSelection( 0 );
	sbSizer10->Add( cboShipType, 0, wxEXPAND|wxALL, 3 );
	
	optShipTypeHasShieldSystem = new wxRadioButton( this, wxID_ANY, wxT("Have shield systems"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optShipTypeHasShieldSystem->SetValue( true ); 
	sbSizer10->Add( optShipTypeHasShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	optShipTypeNoShieldSystem = new wxRadioButton( this, wxID_ANY, wxT("No shield systems"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer10->Add( optShipTypeNoShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	
	bSizer29->Add( sbSizer10, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("All ships on team") ), wxVERTICAL );
	
	wxArrayString cboShipTeamChoices;
	cboShipTeam = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboShipTeamChoices, 0 );
	cboShipTeam->SetSelection( 0 );
	sbSizer11->Add( cboShipTeam, 0, wxEXPAND|wxALL, 3 );
	
	optShipTeamHasShieldSystem = new wxRadioButton( this, wxID_ANY, wxT("Have shield systems"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optShipTeamHasShieldSystem->SetValue( true ); 
	sbSizer11->Add( optShipTeamHasShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	optShipTeamNoShieldSystem = new wxRadioButton( this, wxID_ANY, wxT("No shield systems"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer11->Add( optShipTeamNoShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	
	bSizer29->Add( sbSizer11, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer28->Add( bSizer29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	dlgShieldConfirm = new wxStdDialogButtonSizer();
	dlgShieldConfirmOK = new wxButton( this, wxID_OK );
	dlgShieldConfirm->AddButton( dlgShieldConfirmOK );
	dlgShieldConfirmCancel = new wxButton( this, wxID_CANCEL );
	dlgShieldConfirm->AddButton( dlgShieldConfirmCancel );
	dlgShieldConfirm->Realize();
	
	bSizer28->Add( dlgShieldConfirm, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer28 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgShieldSystemEditor::~dlgShieldSystemEditor()
{
}

BEGIN_EVENT_TABLE( dlgSetGlobalShipFlagsEditor, wxDialog )
	EVT_CLOSE( dlgSetGlobalShipFlagsEditor::_wxFB_OnClose )
END_EVENT_TABLE()

dlgSetGlobalShipFlagsEditor::dlgSetGlobalShipFlagsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxVERTICAL );
	
	btnGlobalNoShields = new wxButton( this, wxID_ANY, wxT("Global No-Shields"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer40->Add( btnGlobalNoShields, 0, wxEXPAND|wxALL, 5 );
	
	btnGlobalNoSubspaceDrive = new wxButton( this, wxID_ANY, wxT("Global No-Subspace-Drive"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer40->Add( btnGlobalNoSubspaceDrive, 0, wxEXPAND|wxALL, 5 );
	
	btnGlobalPrimitiveSensors = new wxButton( this, wxID_ANY, wxT("Global Primitive-Sensors"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer40->Add( btnGlobalPrimitiveSensors, 0, wxEXPAND|wxALL, 5 );
	
	btnGlobalAffectedByGravity = new wxButton( this, wxID_ANY, wxT("Global Affected-By-Gravity"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer40->Add( btnGlobalAffectedByGravity, 0, wxEXPAND|wxALL, 5 );
	
	
	this->SetSizer( bSizer40 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgSetGlobalShipFlagsEditor::~dlgSetGlobalShipFlagsEditor()
{
}

BEGIN_EVENT_TABLE( dlgVoiceActingManager, wxDialog )
	EVT_CLOSE( dlgVoiceActingManager::_wxFB_OnClose )
END_EVENT_TABLE()

dlgVoiceActingManager::dlgVoiceActingManager( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer107;
	bSizer107 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer34;
	fgSizer34 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer34->AddGrowableCol( 0 );
	fgSizer34->AddGrowableCol( 1 );
	fgSizer34->AddGrowableRow( 0 );
	fgSizer34->AddGrowableRow( 1 );
	fgSizer34->SetFlexibleDirection( wxBOTH );
	fgSizer34->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer42;
	sbSizer42 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("File Name Options") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer45;
	sbSizer45 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Abbreviations") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer29;
	fgSizer29 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer29->SetFlexibleDirection( wxBOTH );
	fgSizer29->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblCampaign = new wxStaticText( this, wxID_ANY, wxT("Campaign:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaign->Wrap( -1 );
	fgSizer29->Add( lblCampaign, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevCampaign = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevCampaign->SetMaxLength( 0 ); 
	fgSizer29->Add( txtAbbrevCampaign, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblMission = new wxStaticText( this, wxID_ANY, wxT("Mission:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMission->Wrap( -1 );
	fgSizer29->Add( lblMission, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevMission = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevMission->SetMaxLength( 0 ); 
	fgSizer29->Add( txtAbbrevMission, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblCmdBriefingStage = new wxStaticText( this, wxID_ANY, wxT("Cmd. Briefing Stage:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCmdBriefingStage->Wrap( -1 );
	fgSizer29->Add( lblCmdBriefingStage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevCB = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevCB->SetMaxLength( 0 ); 
	fgSizer29->Add( txtAbbrevCB, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblBriefingStage = new wxStaticText( this, wxID_ANY, wxT("Briefing Stage:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBriefingStage->Wrap( -1 );
	fgSizer29->Add( lblBriefingStage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevBriefing = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevBriefing->SetMaxLength( 0 ); 
	fgSizer29->Add( txtAbbrevBriefing, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblDebriefingStage = new wxStaticText( this, wxID_ANY, wxT("Debriefing Stage:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDebriefingStage->Wrap( -1 );
	fgSizer29->Add( lblDebriefingStage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevDebrief = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevDebrief->SetMaxLength( 0 ); 
	fgSizer29->Add( txtAbbrevDebrief, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblMessage = new wxStaticText( this, wxID_ANY, wxT("Message:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessage->Wrap( -1 );
	fgSizer29->Add( lblMessage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevMessage = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevMessage->SetMaxLength( 0 ); 
	fgSizer29->Add( txtAbbrevMessage, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	
	sbSizer45->Add( fgSizer29, 1, wxEXPAND, 5 );
	
	
	sbSizer42->Add( sbSizer45, 0, wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer46;
	sbSizer46 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Other") ), wxHORIZONTAL );
	
	lblAudioFileExtension = new wxStaticText( this, wxID_ANY, wxT("File Extension:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAudioFileExtension->Wrap( -1 );
	sbSizer46->Add( lblAudioFileExtension, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboVAFileExtChoices;
	cboVAFileExt = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboVAFileExtChoices, 0 );
	cboVAFileExt->SetSelection( 0 );
	sbSizer46->Add( cboVAFileExt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer42->Add( sbSizer46, 0, wxALL, 3 );
	
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText125 = new wxStaticText( this, wxID_ANY, wxT("Example"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText125->Wrap( -1 );
	bSizer72->Add( m_staticText125, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtExampleFileName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtExampleFileName->SetMaxLength( 0 ); 
	txtExampleFileName->Enable( false );
	
	bSizer72->Add( txtExampleFileName, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer42->Add( bSizer72, 0, wxALL|wxEXPAND, 3 );
	
	chkVANoReplaceExistingFiles = new wxCheckBox( this, wxID_ANY, wxT("Don't replace existing files"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer42->Add( chkVANoReplaceExistingFiles, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer42->Add( 0, 21, 1, wxEXPAND, 5 );
	
	btnGenerateFileNames = new wxButton( this, wxID_ANY, wxT("Generate File Names"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer42->Add( btnGenerateFileNames, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer34->Add( sbSizer42, 1, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer44;
	sbSizer44 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Script options") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer47;
	sbSizer47 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Script Entry Format") ), wxHORIZONTAL );
	
	txtScriptEntryFormat = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtScriptEntryFormat->SetMaxLength( 0 ); 
	sbSizer47->Add( txtScriptEntryFormat, 1, wxALL|wxEXPAND, 3 );
	
	lblScriptHelp = new wxStaticText( this, wxID_ANY, wxT("$filename - Name of the message file\n$message - Text of the message\n$persona - Persona of the sender\n$sender - Name of the sender\n\nNote: $persona and $sender will only appear for the Message section"), wxDefaultPosition, wxDefaultSize, 0 );
	lblScriptHelp->Wrap( 190 );
	sbSizer47->Add( lblScriptHelp, 1, wxALL, 3 );
	
	
	sbSizer44->Add( sbSizer47, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer48;
	sbSizer48 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Export...") ), wxVERTICAL );
	
	optEverything = new wxRadioButton( this, wxID_ANY, wxT("Everything"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer48->Add( optEverything, 0, wxALL, 3 );
	
	optJustCommandBriefings = new wxRadioButton( this, wxID_ANY, wxT("Just Command Briefings"), wxDefaultPosition, wxDefaultSize, 0 );
	optJustCommandBriefings->SetValue( true ); 
	sbSizer48->Add( optJustCommandBriefings, 0, wxALL, 3 );
	
	optJustBriefings = new wxRadioButton( this, wxID_ANY, wxT("Just Briefings"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer48->Add( optJustBriefings, 0, wxALL, 3 );
	
	optJustDebriefings = new wxRadioButton( this, wxID_ANY, wxT("Just Debriefings"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer48->Add( optJustDebriefings, 0, wxALL, 3 );
	
	optJustMessages = new wxRadioButton( this, wxID_ANY, wxT("Just Messages"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer48->Add( optJustMessages, 0, wxALL, 3 );
	
	wxBoxSizer* bSizer128;
	bSizer128 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer128->Add( 21, 0, 1, wxEXPAND, 5 );
	
	chkGroupMessageList = new wxCheckBox( this, wxID_ANY, wxT("Group send-message-list messages before others"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGroupMessageList->Enable( false );
	
	bSizer128->Add( chkGroupMessageList, 0, wxALL, 3 );
	
	
	sbSizer48->Add( bSizer128, 1, 0, 5 );
	
	
	sbSizer44->Add( sbSizer48, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer44->Add( 0, 0, 1, wxEXPAND, 5 );
	
	btnGenerateScript = new wxButton( this, wxID_ANY, wxT("Generate Script"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer44->Add( btnGenerateScript, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer34->Add( sbSizer44, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer107->Add( fgSizer34, 1, 0, 5 );
	
	
	this->SetSizer( bSizer107 );
	this->Layout();
	bSizer107->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgVoiceActingManager::~dlgVoiceActingManager()
{
}

BEGIN_EVENT_TABLE( frmCampaignEditor, wxFrame )
	EVT_CLOSE( frmCampaignEditor::_wxFB_OnClose )
END_EVENT_TABLE()

frmCampaignEditor::frmCampaignEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 862,705 ), wxSize( 862,705 ) );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxHORIZONTAL );
	
	m_splitter1 = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_splitter1->SetSashSize( 5 );
	m_splitter1->Connect( wxEVT_IDLE, wxIdleEventHandler( frmCampaignEditor::m_splitter1OnIdle ), NULL, this );
	
	pnlCampaign = new wxScrolledWindow( m_splitter1, wxID_ANY, wxDefaultPosition, wxSize( 400,-1 ), wxHSCROLL|wxSUNKEN_BORDER|wxVSCROLL );
	pnlCampaign->SetScrollRate( 5, 5 );
	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxVERTICAL );
	
	lblAvailableMissions = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Available missions:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAvailableMissions->Wrap( -1 );
	bSizer45->Add( lblAvailableMissions, 0, wxALIGN_LEFT|wxALL|wxTOP, 3 );
	
	lstAvailableMissions = new wxListCtrl( pnlCampaign, ID_lstAvailableMissions, wxDefaultPosition, wxDefaultSize, wxLC_LIST|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING );
	bSizer45->Add( lstAvailableMissions, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer46;
	bSizer46 = new wxBoxSizer( wxHORIZONTAL );
	
	lblCampaignName = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Campaign Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaignName->Wrap( -1 );
	bSizer46->Add( lblCampaignName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtCampaignName = new wxTextCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtCampaignName->SetMaxLength( 0 ); 
	bSizer46->Add( txtCampaignName, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblCampaignType = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaignType->Wrap( 0 );
	bSizer46->Add( lblCampaignType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cbCampaignTypeChoices[] = { wxT("single"), wxT("multi coop"), wxT("multi teams") };
	int cbCampaignTypeNChoices = sizeof( cbCampaignTypeChoices ) / sizeof( wxString );
	cbCampaignType = new wxChoice( pnlCampaign, wxID_ANY, wxDefaultPosition, wxDefaultSize, cbCampaignTypeNChoices, cbCampaignTypeChoices, 0 );
	cbCampaignType->SetSelection( 0 );
	bSizer46->Add( cbCampaignType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer45->Add( bSizer46, 0, wxALL|wxEXPAND, 3 );
	
	lblCampaignDescription = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Campaign Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaignDescription->Wrap( -1 );
	bSizer45->Add( lblCampaignDescription, 0, wxALIGN_LEFT|wxALL, 3 );
	
	txtCampaignDescription = new wxTextCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,55 ), wxTE_MULTILINE );
	txtCampaignDescription->SetMaxLength( 0 ); 
	bSizer45->Add( txtCampaignDescription, 0, wxALL|wxEXPAND, 3 );
	
	chkUsesCustomTechDatabase = new wxCheckBox( pnlCampaign, wxID_ANY, wxT("Uses custom tech database"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
	bSizer45->Add( chkUsesCustomTechDatabase, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer15;
	sbSizer15 = new wxStaticBoxSizer( new wxStaticBox( pnlCampaign, wxID_ANY, wxT("Mission options") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer42;
	fgSizer42 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer42->SetFlexibleDirection( wxBOTH );
	fgSizer42->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBriefingCutscene = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Briefing Cutscene:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBriefingCutscene->Wrap( -1 );
	fgSizer42->Add( lblBriefingCutscene, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpBriefingCutscene = new wxFilePickerCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxSize( -1,-1 ), wxFLP_DEFAULT_STYLE );
	fgSizer42->Add( fpBriefingCutscene, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer42->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer43;
	fgSizer43 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer43->SetFlexibleDirection( wxBOTH );
	fgSizer43->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblMainhallIndex = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Mainhall Index:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMainhallIndex->Wrap( -1 );
	fgSizer43->Add( lblMainhallIndex, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnMainHallIndex = new wxSpinCtrl( pnlCampaign, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 9, 0 );
	fgSizer43->Add( spnMainHallIndex, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblDebriefingPersonaIndex = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Debriefing Persona Index:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDebriefingPersonaIndex->Wrap( -1 );
	fgSizer43->Add( lblDebriefingPersonaIndex, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDebriefingPersonaIndex = new wxSpinCtrl( pnlCampaign, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 9, 0 );
	fgSizer43->Add( spnDebriefingPersonaIndex, 0, wxALL, 3 );
	
	
	fgSizer42->Add( fgSizer43, 1, wxEXPAND, 5 );
	
	
	sbSizer15->Add( fgSizer42, 0, 0, 5 );
	
	
	bSizer45->Add( sbSizer15, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer47;
	bSizer47 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxVERTICAL );
	
	lblBranches = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Branches:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBranches->Wrap( 0 );
	bSizer48->Add( lblBranches, 0, wxALIGN_LEFT|wxALL|wxLEFT|wxRIGHT|wxTOP, 3 );
	
	treeBranches = new wxTreeCtrl( pnlCampaign, wxID_ANY, wxDefaultPosition, wxSize( 200,200 ), wxTR_EDIT_LABELS|wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_SINGLE|wxSUNKEN_BORDER );
	bSizer48->Add( treeBranches, 0, wxALL|wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 3 );
	
	
	bSizer47->Add( bSizer48, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 3 );
	
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( pnlCampaign, wxID_ANY, wxT("Branch Options") ), wxVERTICAL );
	
	btnMoveUp = new wxButton( pnlCampaign, wxID_ANY, wxT("Move Up"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	sbSizer16->Add( btnMoveUp, 0, wxALL, 3 );
	
	btnMoveDown = new wxButton( pnlCampaign, wxID_ANY, wxT("Move Down"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	sbSizer16->Add( btnMoveDown, 0, wxALL, 3 );
	
	btnToggleLoop = new wxButton( pnlCampaign, wxID_ANY, wxT("Toggle Loop"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	sbSizer16->Add( btnToggleLoop, 0, wxALL, 3 );
	
	
	bSizer49->Add( sbSizer16, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	btnRealignTree = new wxButton( pnlCampaign, wxID_ANY, wxT("Realign Tree"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer49->Add( btnRealignTree, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	btnLoadMission = new wxButton( pnlCampaign, wxID_ANY, wxT("Load Mission"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer49->Add( btnLoadMission, 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT, 3 );
	
	btnClose = new wxButton( pnlCampaign, wxID_ANY, wxT("Close"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer49->Add( btnClose, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer47->Add( bSizer49, 0, wxALIGN_TOP|wxALL|wxEXPAND, 3 );
	
	
	bSizer45->Add( bSizer47, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	lblDesignerNotes = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Designer Notes:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDesignerNotes->Wrap( -1 );
	bSizer45->Add( lblDesignerNotes, 0, wxALL, 3 );
	
	txtDesignerNotes = new wxTextCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,50 ), wxTE_MULTILINE|wxTE_READONLY );
	txtDesignerNotes->SetMaxLength( 0 ); 
	bSizer45->Add( txtDesignerNotes, 0, wxEXPAND|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer49;
	sbSizer49 = new wxStaticBoxSizer( new wxStaticBox( pnlCampaign, wxID_ANY, wxT("Mission Loop Options") ), wxVERTICAL );
	
	lblMissionLoopDiscription = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Discription:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMissionLoopDiscription->Wrap( -1 );
	sbSizer49->Add( lblMissionLoopDiscription, 0, wxALIGN_LEFT|wxALL, 3 );
	
	txtMissionLoopDescription = new wxTextCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,50 ), wxTE_MULTILINE );
	txtMissionLoopDescription->SetMaxLength( 0 ); 
	sbSizer49->Add( txtMissionLoopDescription, 0, wxALL|wxEXPAND, 3 );
	
	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer31->AddGrowableCol( 1 );
	fgSizer31->SetFlexibleDirection( wxBOTH );
	fgSizer31->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblLoopBriefAni = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Briefing Animation:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLoopBriefAni->Wrap( -1 );
	fgSizer31->Add( lblLoopBriefAni, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpLoopBriefAni = new wxFilePickerCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxT("Select an animation file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer31->Add( fpLoopBriefAni, 1, wxALL|wxEXPAND, 3 );
	
	lblBriefVoice = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Briefing Voice:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBriefVoice->Wrap( -1 );
	fgSizer31->Add( lblBriefVoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpLoopBriefVoice = new wxFilePickerCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxT("Selec a voice file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer31->Add( fpLoopBriefVoice, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer49->Add( fgSizer31, 1, wxEXPAND, 5 );
	
	
	bSizer45->Add( sbSizer49, 1, wxEXPAND, 5 );
	
	
	pnlCampaign->SetSizer( bSizer45 );
	pnlCampaign->Layout();
	pnlCampaignGraph = new wxPanel( m_splitter1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	m_splitter1->SplitVertically( pnlCampaign, pnlCampaignGraph, 0 );
	bSizer44->Add( m_splitter1, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer44 );
	this->Layout();
	m_menubar2 = new wxMenuBar( 0 );
	mnuFile = new wxMenu();
	wxMenuItem* mnuFileNew;
	mnuFileNew = new wxMenuItem( mnuFile, ID_NEW, wxString( wxT("New") ) + wxT('\t') + wxT("Ctrl-N"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileNew );
	
	wxMenuItem* mnuFileOpen;
	mnuFileOpen = new wxMenuItem( mnuFile, ID_OPEN, wxString( wxT("Open") ) + wxT('\t') + wxT("Ctrl-O"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileOpen );
	
	wxMenuItem* mnuFileSave;
	mnuFileSave = new wxMenuItem( mnuFile, ID_SAVE, wxString( wxT("Save") ) + wxT('\t') + wxT("Ctrl-S"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSave );
	
	wxMenuItem* mnuFileSaveAs;
	mnuFileSaveAs = new wxMenuItem( mnuFile, ID_SAVE_AS, wxString( wxT("Save As...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSaveAs );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileExit;
	mnuFileExit = new wxMenuItem( mnuFile, ID_EXIT, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileExit );
	
	m_menubar2->Append( mnuFile, wxT("File") ); 
	
	other = new wxMenu();
	wxMenuItem* errorChecker;
	errorChecker = new wxMenuItem( other, ID_ERROR_CHECKER, wxString( wxT("Error Checker") ) + wxT('\t') + wxT("Alt-H"), wxEmptyString, wxITEM_NORMAL );
	other->Append( errorChecker );
	
	m_menubar2->Append( other, wxT("Other") ); 
	
	initialStatus = new wxMenu();
	wxMenuItem* ships;
	ships = new wxMenuItem( initialStatus, ID_SHIPS, wxString( wxT("Ships") ) , wxEmptyString, wxITEM_NORMAL );
	initialStatus->Append( ships );
	
	wxMenuItem* weapons;
	weapons = new wxMenuItem( initialStatus, ID_WEAPONS, wxString( wxT("Weapons") ) , wxEmptyString, wxITEM_NORMAL );
	initialStatus->Append( weapons );
	
	m_menubar2->Append( initialStatus, wxT("Initial Status") ); 
	
	this->SetMenuBar( m_menubar2 );
	
	
	this->Centre( wxBOTH );
}

frmCampaignEditor::~frmCampaignEditor()
{
}

BEGIN_EVENT_TABLE( dlgMissionStats, wxDialog )
	EVT_CLOSE( dlgMissionStats::_wxFB_OnClose )
END_EVENT_TABLE()

dlgMissionStats::dlgMissionStats( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer130;
	bSizer130 = new wxBoxSizer( wxVERTICAL );
	
	txtMissionStats = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	txtMissionStats->Enable( false );
	
	bSizer130->Add( txtMissionStats, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer131;
	bSizer131 = new wxBoxSizer( wxHORIZONTAL );
	
	btnDumpToFile = new wxButton( this, wxID_ANY, wxT("Dump to File"), wxDefaultPosition, wxSize( -1,30 ), 0 );
	bSizer131->Add( btnDumpToFile, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxSize( -1,30 ), 0 );
	bSizer131->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer130->Add( bSizer131, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer130 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgMissionStats::~dlgMissionStats()
{
}

BEGIN_EVENT_TABLE( dlgAboutBox, wxDialog )
	EVT_CLOSE( dlgAboutBox::_wxFB_OnClose )
END_EVENT_TABLE()

dlgAboutBox::dlgAboutBox( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer104;
	bSizer104 = new wxBoxSizer( wxHORIZONTAL );
	
	bmpLogo = new wxStaticBitmap( this, wxID_ANY, wxBitmap( fred_splash_xpm ), wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
	bmpLogo->SetBackgroundColour( wxColour( 255, 255, 255 ) );
	
	bSizer104->Add( bmpLogo, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxVERTICAL );
	
	lblAppTitle = new wxStaticText( this, wxID_ANY, wxT("FRED2_OPEN - FreeSpace Editor, Version 3.6.19"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAppTitle->Wrap( -1 );
	bSizer105->Add( lblAppTitle, 0, wxALL, 3 );
	
	lblCopywrite = new wxStaticText( this, wxID_ANY, wxT("Copyright 1999 Volition, Inc.  All Rights Reserved"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCopywrite->Wrap( -1 );
	bSizer105->Add( lblCopywrite, 0, wxALL, 3 );
	
	
	bSizer105->Add( 0, 0, 1, wxALL|wxEXPAND, 5 );
	
	lblDevelopers = new wxStaticText( this, wxID_ANY, wxT("Bravely maintained by:\n\tGoober5000, taylor, Karajorma and the SCP Team.\n\nPorted to OpenGL by:\n\tRandomTiger, Phreak, and Fry_Day.\n\nPorted to wxWidgets by:\n\tGoober5000, taylor, The E, and z64555."), wxDefaultPosition, wxSize( 300,-1 ), 0 );
	lblDevelopers->Wrap( -1 );
	bSizer105->Add( lblDevelopers, 0, wxALL, 3 );
	
	
	bSizer105->Add( 0, 0, 1, wxALL|wxEXPAND, 5 );
	
	lblQuote = new wxStaticText( this, wxID_ANY, wxT("\"Fred2 is the omega of all giant unwieldy pieces of code. It's big, it's horrifying, and it just doesn't care. View it at your own risk\" - Dave Baranec"), wxDefaultPosition, wxDefaultSize, 0 );
	lblQuote->Wrap( 370 );
	bSizer105->Add( lblQuote, 0, wxALL, 3 );
	
	
	bSizer105->Add( 0, 0, 1, wxALL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer36;
	fgSizer36 = new wxFlexGridSizer( 0, 5, 0, 5 );
	fgSizer36->AddGrowableCol( 1 );
	fgSizer36->AddGrowableCol( 2 );
	fgSizer36->AddGrowableCol( 3 );
	fgSizer36->SetFlexibleDirection( wxBOTH );
	fgSizer36->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer36->Add( 0, 0, 1, wxEXPAND, 5 );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	btnOK->SetDefault(); 
	fgSizer36->Add( btnOK, 0, wxEXPAND, 3 );
	
	btnReportBug = new wxButton( this, wxID_ANY, wxT("Report a Bug"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	fgSizer36->Add( btnReportBug, 0, wxEXPAND, 3 );
	
	btnVisitForums = new wxButton( this, wxID_ANY, wxT("Visit Forums"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	fgSizer36->Add( btnVisitForums, 0, wxALIGN_CENTER|wxEXPAND, 3 );
	
	
	fgSizer36->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	bSizer105->Add( fgSizer36, 0, wxEXPAND, 3 );
	
	
	bSizer104->Add( bSizer105, 1, wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer104 );
	this->Layout();
	bSizer104->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgAboutBox::~dlgAboutBox()
{
}

BEGIN_EVENT_TABLE( dlgSexpHelp, wxDialog )
	EVT_CLOSE( dlgSexpHelp::_wxFB_OnClose )
END_EVENT_TABLE()

dlgSexpHelp::dlgSexpHelp( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer100;
	bSizer100 = new wxBoxSizer( wxVERTICAL );
	
	pnlSexpHelp = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxVERTICAL );
	
	lblArgInfo = new wxStaticText( pnlSexpHelp, wxID_ANY, wxT("Argument Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArgInfo->Wrap( -1 );
	bSizer103->Add( lblArgInfo, 0, wxALL, 3 );
	
	txtArgInfo = new wxTextCtrl( pnlSexpHelp, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer103->Add( txtArgInfo, 0, wxALL|wxEXPAND, 3 );
	
	lblSexpInfo = new wxStaticText( pnlSexpHelp, wxID_ANY, wxT("Sexp Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSexpInfo->Wrap( -1 );
	bSizer103->Add( lblSexpInfo, 0, wxALL, 3 );
	
	txtSexpInfo = new wxTextCtrl( pnlSexpHelp, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer103->Add( txtSexpInfo, 1, wxALL|wxEXPAND, 3 );
	
	
	pnlSexpHelp->SetSizer( bSizer103 );
	pnlSexpHelp->Layout();
	bSizer103->Fit( pnlSexpHelp );
	bSizer100->Add( pnlSexpHelp, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer100 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgSexpHelp::~dlgSexpHelp()
{
}

pnlSexpHelp::pnlSexpHelp( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxVERTICAL );
	
	lblArgInfo = new wxStaticText( this, wxID_ANY, wxT("Argument Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArgInfo->Wrap( -1 );
	bSizer103->Add( lblArgInfo, 0, wxALL, 3 );
	
	txtArgInfo = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer103->Add( txtArgInfo, 0, wxALL|wxEXPAND, 3 );
	
	lblSexpInfo = new wxStaticText( this, wxID_ANY, wxT("Sexp Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSexpInfo->Wrap( -1 );
	bSizer103->Add( lblSexpInfo, 0, wxALL, 3 );
	
	txtSexpInfo = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer103->Add( txtSexpInfo, 1, wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer103 );
	this->Layout();
	bSizer103->Fit( this );
}

pnlSexpHelp::~pnlSexpHelp()
{
}
