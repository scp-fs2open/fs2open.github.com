///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxFRED_base.h"

#include "res/constx.png.h"
#include "res/constxy.png.h"
#include "res/constxz.png.h"
#include "res/consty.png.h"
#include "res/constyz.png.h"
#include "res/constz.png.h"
#include "res/fred_splash.xpm"
#include "res/orbitsel.png.h"
#include "res/play.xpm"
#include "res/rotlocal.png.h"
#include "res/select.png.h"
#include "res/selectlist.png.h"
#include "res/selectlock.png.h"
#include "res/selectmove.png.h"
#include "res/selectrot.png.h"
#include "res/showdist.png.h"
#include "res/wingdisband.png.h"
#include "res/wingform.png.h"
#include "res/zoomext.png.h"
#include "res/zoomsel.png.h"

///////////////////////////////////////////////////////////////////////////
using namespace fredBase;

frmFRED::frmFRED( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	mbrFRED = new wxMenuBar( 0 );
	mnuFile = new wxMenu();
	wxMenuItem* mnuFileNew;
	mnuFileNew = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("New") ) + wxT('\t') + wxT("Ctrl+N"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileNew );
	
	wxMenuItem* mnuFileOpen;
	mnuFileOpen = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Open...") ) + wxT('\t') + wxT("Ctrl-O"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileOpen );
	
	wxMenuItem* mnuFileSave;
	mnuFileSave = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Save") ) + wxT('\t') + wxT("Ctrl+S"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSave );
	
	wxMenuItem* mnuFileSaveAs;
	mnuFileSaveAs = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Save As...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSaveAs );
	
	wxMenuItem* mnuFileRevert;
	mnuFileRevert = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Revert") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileRevert );
	
	mnuFile->AppendSeparator();
	
	mnuFileSaveFormat = new wxMenu();
	wxMenuItem* mnuFileSaveFormatItem = new wxMenuItem( mnuFile, wxID_ANY, wxT("Save Format"), wxEmptyString, wxITEM_NORMAL, mnuFileSaveFormat );
	wxMenuItem* mnuFileSaveFormatFs2Open;
	mnuFileSaveFormatFs2Open = new wxMenuItem( mnuFileSaveFormat, wxID_ANY, wxString( wxT("FS2 open") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFileSaveFormat->Append( mnuFileSaveFormatFs2Open );
	
	wxMenuItem* mnuFileSaveFormatFs2Retail;
	mnuFileSaveFormatFs2Retail = new wxMenuItem( mnuFileSaveFormat, wxID_ANY, wxString( wxT("FS2 retail") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFileSaveFormat->Append( mnuFileSaveFormatFs2Retail );
	
	mnuFile->Append( mnuFileSaveFormatItem );
	
	mnuFileImport = new wxMenu();
	wxMenuItem* mnuFileImportItem = new wxMenuItem( mnuFile, wxID_ANY, wxT("Import"), wxEmptyString, wxITEM_NORMAL, mnuFileImport );
	wxMenuItem* mnuFileImportFs1Mission;
	mnuFileImportFs1Mission = new wxMenuItem( mnuFileImport, wxID_ANY, wxString( wxT("FS1 mission...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFileImport->Append( mnuFileImportFs1Mission );
	
	wxMenuItem* mnuFileImportWeaponLoadouts;
	mnuFileImportWeaponLoadouts = new wxMenuItem( mnuFileImport, wxID_ANY, wxString( wxT("FS1 weapon loadouts...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFileImport->Append( mnuFileImportWeaponLoadouts );
	
	mnuFile->Append( mnuFileImportItem );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileRunFreespace;
	mnuFileRunFreespace = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Run Freespace") ) + wxT('\t') + wxT("Alt+R"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileRunFreespace );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileRecentFiles;
	mnuFileRecentFiles = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Recent File List") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileRecentFiles );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileExit;
	mnuFileExit = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileExit );
	
	mbrFRED->Append( mnuFile, wxT("File") ); 
	
	mnuEdit = new wxMenu();
	wxMenuItem* mnuEditUndo;
	mnuEditUndo = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("Undo") ) + wxT('\t') + wxT("Ctrl+Z"), wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditUndo );
	
	wxMenuItem* mnuEditDelete;
	mnuEditDelete = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("Delete") ) + wxT('\t') + wxT("Del"), wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditDelete );
	
	wxMenuItem* mnuEditDeleteWing;
	mnuEditDeleteWing = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("Delete Wing") ) + wxT('\t') + wxT("Ctrl+Del"), wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditDeleteWing );
	
	mnuEdit->AppendSeparator();
	
	wxMenuItem* mnuEditDisableUndo;
	mnuEditDisableUndo = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("Disable Undo") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditDisableUndo );
	
	mbrFRED->Append( mnuEdit, wxT("Edit") ); 
	
	mnuView = new wxMenu();
	wxMenuItem* mnuViewToolbar;
	mnuViewToolbar = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Toolbar") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewToolbar );
	
	wxMenuItem* mnuViewStatusBar;
	mnuViewStatusBar = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Status Bar") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewStatusBar );
	
	mnuView->AppendSeparator();
	
	mnuViewDisplayFiter = new wxMenu();
	wxMenuItem* mnuViewDisplayFiterItem = new wxMenuItem( mnuView, wxID_ANY, wxT("Display Filter"), wxEmptyString, wxITEM_NORMAL, mnuViewDisplayFiter );
	wxMenuItem* mnuViewDisplayFilterShowShips;
	mnuViewDisplayFilterShowShips = new wxMenuItem( mnuViewDisplayFiter, wxID_ANY, wxString( wxT("Show Ships") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowShips );
	
	wxMenuItem* mnuViewDisplayFilterShowPlayerStarts;
	mnuViewDisplayFilterShowPlayerStarts = new wxMenuItem( mnuViewDisplayFiter, wxID_ANY, wxString( wxT("Show Player Starts") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowPlayerStarts );
	
	wxMenuItem* mnuViewDisplayFilterShowWaypoints;
	mnuViewDisplayFilterShowWaypoints = new wxMenuItem( mnuViewDisplayFiter, wxID_ANY, wxString( wxT("Show Waypoints") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowWaypoints );
	
	mnuViewDisplayFiter->AppendSeparator();
	
	wxMenuItem* mnuViewDisplayFilterShowFriendly;
	mnuViewDisplayFilterShowFriendly = new wxMenuItem( mnuViewDisplayFiter, wxID_ANY, wxString( wxT("Show Friendly") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowFriendly );
	
	wxMenuItem* mnuViewDisplayFilterShowHostile;
	mnuViewDisplayFilterShowHostile = new wxMenuItem( mnuViewDisplayFiter, wxID_ANY, wxString( wxT("Show Hostile") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewDisplayFiter->Append( mnuViewDisplayFilterShowHostile );
	
	mnuView->Append( mnuViewDisplayFiterItem );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewHideMarkedObjects;
	mnuViewHideMarkedObjects = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Hide Marked Objects") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewHideMarkedObjects );
	
	wxMenuItem* mnuViewShowHiddenObjects;
	mnuViewShowHiddenObjects = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Hidden Objects") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowHiddenObjects );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewShowShipModels;
	mnuViewShowShipModels = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Ship Models") ) + wxT('\t') + wxT("Shift+Alt+M"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowShipModels );
	
	wxMenuItem* mnuViewShowOutlines;
	mnuViewShowOutlines = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Outlines") ) + wxT('\t') + wxT("Shift+Alt+O"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowOutlines );
	
	wxMenuItem* mnuViewShowShipInfo;
	mnuViewShowShipInfo = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Ship Info") ) + wxT('\t') + wxT("Shift+Alt+I"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowShipInfo );
	
	wxMenuItem* mnuViewShowCoordinates;
	mnuViewShowCoordinates = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Coordinates") ) + wxT('\t') + wxT("Shift+Alt+C"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowCoordinates );
	
	wxMenuItem* mnuViewShowGridPositions;
	mnuViewShowGridPositions = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Grid Positions") ) + wxT('\t') + wxT("Shift+Alt+P"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowGridPositions );
	
	wxMenuItem* mnuViewShowDistances;
	mnuViewShowDistances = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Distances") ) + wxT('\t') + wxT("D"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowDistances );
	
	wxMenuItem* mnuViewShowModelPaths;
	mnuViewShowModelPaths = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Model Paths") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowModelPaths );
	
	wxMenuItem* mnuViewShowModelDockPoints;
	mnuViewShowModelDockPoints = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Model Dock Points") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowModelDockPoints );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewShowGrid;
	mnuViewShowGrid = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Grid") ) + wxT('\t') + wxT("Shift+Alt+G"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowGrid );
	
	wxMenuItem* mnuViewShowHorizon;
	mnuViewShowHorizon = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Horizon") ) + wxT('\t') + wxT("Shift+Alt+H"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowHorizon );
	
	wxMenuItem* mnuViewDoubleFineGridlines;
	mnuViewDoubleFineGridlines = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Double Fine Gridlines") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewDoubleFineGridlines );
	
	wxMenuItem* mnuViewAntiAliasedGridlines;
	mnuViewAntiAliasedGridlines = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Anti-Aliased Gridlines") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewAntiAliasedGridlines );
	
	wxMenuItem* mnuViewShow3DCompass;
	mnuViewShow3DCompass = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show 3-D Compass") ) + wxT('\t') + wxT("Shift+Alt+3"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShow3DCompass );
	
	wxMenuItem* mnuViewShowBackground;
	mnuViewShowBackground = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Background") ) + wxT('\t') + wxT("Shift+Alt+B"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewShowBackground );
	
	mnuView->AppendSeparator();
	
	mnuViewViewpoint = new wxMenu();
	wxMenuItem* mnuViewViewpointItem = new wxMenuItem( mnuView, wxID_ANY, wxT("Viewpoint\tShift+V"), wxEmptyString, wxITEM_NORMAL, mnuViewViewpoint );
	wxMenuItem* mnuViewViewpointCamera;
	mnuViewViewpointCamera = new wxMenuItem( mnuViewViewpoint, wxID_ANY, wxString( wxT("Camera") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewViewpoint->Append( mnuViewViewpointCamera );
	
	wxMenuItem* mnuViewViewpointCurrentShip;
	mnuViewViewpointCurrentShip = new wxMenuItem( mnuViewViewpoint, wxID_ANY, wxString( wxT("Current Ship") ) , wxEmptyString, wxITEM_NORMAL );
	mnuViewViewpoint->Append( mnuViewViewpointCurrentShip );
	
	mnuView->Append( mnuViewViewpointItem );
	
	wxMenuItem* mnuViewSaveCameraPos;
	mnuViewSaveCameraPos = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Save Camera Pos") ) + wxT('\t') + wxT("Ctrl+P"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewSaveCameraPos );
	
	wxMenuItem* mnuViewRestoreCameraPos;
	mnuViewRestoreCameraPos = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Restore Camera Pos") ) + wxT('\t') + wxT("Ctrl+R"), wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewRestoreCameraPos );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuViewLightingFromSuns;
	mnuViewLightingFromSuns = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Lighting From Suns") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewLightingFromSuns );
	
	mbrFRED->Append( mnuView, wxT("View") ); 
	
	mnuSpeed = new wxMenu();
	mnuSpeedMovement = new wxMenu();
	wxMenuItem* mnuSpeedMovementItem = new wxMenuItem( mnuSpeed, wxID_ANY, wxT("Movement"), wxEmptyString, wxITEM_NORMAL, mnuSpeedMovement );
	wxMenuItem* mnuSpeedMovementX1;
	mnuSpeedMovementX1 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x1") ) + wxT('\t') + wxT("1"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX1 );
	
	wxMenuItem* mnuSpeedMovementX2;
	mnuSpeedMovementX2 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x2") ) + wxT('\t') + wxT("2"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX2 );
	
	wxMenuItem* mnuSpeedMovementX3;
	mnuSpeedMovementX3 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x3") ) + wxT('\t') + wxT("3"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX3 );
	
	wxMenuItem* mnuSpeedMovementX5;
	mnuSpeedMovementX5 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x5") ) + wxT('\t') + wxT("4"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX5 );
	
	wxMenuItem* mnuSpeedMovementX8;
	mnuSpeedMovementX8 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x8") ) + wxT('\t') + wxT("5"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX8 );
	
	wxMenuItem* mnuSpeedMovementX10;
	mnuSpeedMovementX10 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x10") ) + wxT('\t') + wxT("6"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX10 );
	
	wxMenuItem* mnuSpeedMovementX50;
	mnuSpeedMovementX50 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x50") ) + wxT('\t') + wxT("7"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX50 );
	
	wxMenuItem* mnuSpeedMovementX100;
	mnuSpeedMovementX100 = new wxMenuItem( mnuSpeedMovement, wxID_ANY, wxString( wxT("x100") ) + wxT('\t') + wxT("8"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedMovement->Append( mnuSpeedMovementX100 );
	
	mnuSpeed->Append( mnuSpeedMovementItem );
	
	mnuSpeedRotation = new wxMenu();
	wxMenuItem* mnuSpeedRotationItem = new wxMenuItem( mnuSpeed, wxID_ANY, wxT("Rotation"), wxEmptyString, wxITEM_NORMAL, mnuSpeedRotation );
	wxMenuItem* mnuSpeedRotationX1;
	mnuSpeedRotationX1 = new wxMenuItem( mnuSpeedRotation, wxID_ANY, wxString( wxT("x1") ) + wxT('\t') + wxT("Shift+1"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX1 );
	
	wxMenuItem* mnuSpeedRotationX5;
	mnuSpeedRotationX5 = new wxMenuItem( mnuSpeedRotation, wxID_ANY, wxString( wxT("x5") ) + wxT('\t') + wxT("Shift+2"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX5 );
	
	wxMenuItem* mnuSpeedRotationX12;
	mnuSpeedRotationX12 = new wxMenuItem( mnuSpeedRotation, wxID_ANY, wxString( wxT("x12") ) + wxT('\t') + wxT("Shift+3"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX12 );
	
	wxMenuItem* mnuSpeedRotationX25;
	mnuSpeedRotationX25 = new wxMenuItem( mnuSpeedRotation, wxID_ANY, wxString( wxT("x25") ) + wxT('\t') + wxT("Shift+4"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX25 );
	
	wxMenuItem* mnuSpeedRotationX50;
	mnuSpeedRotationX50 = new wxMenuItem( mnuSpeedRotation, wxID_ANY, wxString( wxT("x50") ) + wxT('\t') + wxT("Shift+5"), wxEmptyString, wxITEM_NORMAL );
	mnuSpeedRotation->Append( mnuSpeedRotationX50 );
	
	mnuSpeed->Append( mnuSpeedRotationItem );
	
	mbrFRED->Append( mnuSpeed, wxT("Speed") ); 
	
	mnuEditors = new wxMenu();
	wxMenuItem* mnuEditorsShips;
	mnuEditorsShips = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Ships") ) + wxT('\t') + wxT("Shift+S"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsShips );
	
	wxMenuItem* mnuEditorsWings;
	mnuEditorsWings = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Wings") ) + wxT('\t') + wxT("Shift+W"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsWings );
	
	wxMenuItem* mnuEditorsObjects;
	mnuEditorsObjects = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Objects") ) + wxT('\t') + wxT("Shift+O"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsObjects );
	
	wxMenuItem* mnuEditorsWaypointPaths;
	mnuEditorsWaypointPaths = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Waypoint Paths") ) + wxT('\t') + wxT("Shift+Y"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsWaypointPaths );
	
	wxMenuItem* mnuEditorsMissionObjectives;
	mnuEditorsMissionObjectives = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Mission Objectives") ) + wxT('\t') + wxT("Shift+G"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsMissionObjectives );
	
	wxMenuItem* mnuEditorsEvents;
	mnuEditorsEvents = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Events") ) + wxT('\t') + wxT("Shift+E"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsEvents );
	
	wxMenuItem* mnuEditorsTeamLoadout;
	mnuEditorsTeamLoadout = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Team Loadout") ) + wxT('\t') + wxT("Shift+P"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsTeamLoadout );
	
	wxMenuItem* mnuEditorsBackground;
	mnuEditorsBackground = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Background") ) + wxT('\t') + wxT("Shift+I"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsBackground );
	
	wxMenuItem* mnuEditorsReinforcements;
	mnuEditorsReinforcements = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Reinforcements") ) + wxT('\t') + wxT("Shift+R"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsReinforcements );
	
	wxMenuItem* mnuEditorsAsteroidField;
	mnuEditorsAsteroidField = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Asteroid Field") ) + wxT('\t') + wxT("Shift+A"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsAsteroidField );
	
	wxMenuItem* mnuEditorsMissionSpecs;
	mnuEditorsMissionSpecs = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Mission Specs") ) + wxT('\t') + wxT("Shift+N"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsMissionSpecs );
	
	wxMenuItem* mnuEditorsBriefing;
	mnuEditorsBriefing = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Briefing") ) + wxT('\t') + wxT("Shift+B"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsBriefing );
	
	wxMenuItem* mnuEditorsDebriefing;
	mnuEditorsDebriefing = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Debriefing") ) + wxT('\t') + wxT("Shift+D"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsDebriefing );
	
	wxMenuItem* mnuEditorsCommandBriefing;
	mnuEditorsCommandBriefing = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Command Briefing") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsCommandBriefing );
	
	wxMenuItem* mnuEditorsFictionViewer;
	mnuEditorsFictionViewer = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Fiction Viewer") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsFictionViewer );
	
	wxMenuItem* mnuEditorsShieldSystem;
	mnuEditorsShieldSystem = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Shield System") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsShieldSystem );
	
	wxMenuItem* mnuEditorsSetGlobalShipFlags;
	mnuEditorsSetGlobalShipFlags = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Set Global Ship Flags") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsSetGlobalShipFlags );
	
	wxMenuItem* mnuEditorsVoiceActingManager;
	mnuEditorsVoiceActingManager = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Voice Manager") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsVoiceActingManager );
	
	mnuEditors->AppendSeparator();
	
	wxMenuItem* mnuEditorsCampaign;
	mnuEditorsCampaign = new wxMenuItem( mnuEditors, wxID_ANY, wxString( wxT("Campaign") ) + wxT('\t') + wxT("Shift+C"), wxEmptyString, wxITEM_NORMAL );
	mnuEditors->Append( mnuEditorsCampaign );
	
	mbrFRED->Append( mnuEditors, wxT("Editors") ); 
	
	mnuGroups = new wxMenu();
	wxMenuItem* mnuGroupsGroup1;
	mnuGroupsGroup1 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 1") ) + wxT('\t') + wxT("Ctrl+1"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup1 );
	
	wxMenuItem* mnuGroupsGroup2;
	mnuGroupsGroup2 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 2") ) + wxT('\t') + wxT("Ctrl+2"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup2 );
	
	wxMenuItem* mnuGroupsGroup3;
	mnuGroupsGroup3 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 3") ) + wxT('\t') + wxT("Ctrl+3"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup3 );
	
	wxMenuItem* mnuGroupsGroup4;
	mnuGroupsGroup4 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 4") ) + wxT('\t') + wxT("Ctrl+4"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup4 );
	
	wxMenuItem* mnuGroupsGroup5;
	mnuGroupsGroup5 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 5") ) + wxT('\t') + wxT("Ctrl+5"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup5 );
	
	wxMenuItem* mnuGroupsGroup6;
	mnuGroupsGroup6 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 6") ) + wxT('\t') + wxT("Ctrl+6"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup6 );
	
	wxMenuItem* mnuGroupsGroup7;
	mnuGroupsGroup7 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 7") ) + wxT('\t') + wxT("Ctrl+7"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup7 );
	
	wxMenuItem* mnuGroupsGroup8;
	mnuGroupsGroup8 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 8") ) + wxT('\t') + wxT("Ctrl+8"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup8 );
	
	wxMenuItem* mnuGroupsGroup9;
	mnuGroupsGroup9 = new wxMenuItem( mnuGroups, wxID_ANY, wxString( wxT("Group 9") ) + wxT('\t') + wxT("Ctrl+9"), wxEmptyString, wxITEM_NORMAL );
	mnuGroups->Append( mnuGroupsGroup9 );
	
	mnuGroupsSetGroup = new wxMenu();
	wxMenuItem* mnuGroupsSetGroupItem = new wxMenuItem( mnuGroups, wxID_ANY, wxT("Set Group"), wxEmptyString, wxITEM_NORMAL, mnuGroupsSetGroup );
	wxMenuItem* mnuGroupsSetGroupGroup1;
	mnuGroupsSetGroupGroup1 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 1") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup1 );
	
	wxMenuItem* mnuGroupsSetGroupGroup2;
	mnuGroupsSetGroupGroup2 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 2") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup2 );
	
	wxMenuItem* mnuGroupsSetGroupGroup3;
	mnuGroupsSetGroupGroup3 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 3") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup3 );
	
	wxMenuItem* mnuGroupsSetGroupGroup4;
	mnuGroupsSetGroupGroup4 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 4") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup4 );
	
	wxMenuItem* mnuGroupsSetGroupGroup5;
	mnuGroupsSetGroupGroup5 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 5") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup5 );
	
	wxMenuItem* mnuGroupsSetGroupGroup6;
	mnuGroupsSetGroupGroup6 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 6") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup6 );
	
	wxMenuItem* mnuGroupsSetGroupGroup7;
	mnuGroupsSetGroupGroup7 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 7") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup7 );
	
	wxMenuItem* mnuGroupsSetGroupGroup8;
	mnuGroupsSetGroupGroup8 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 8") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup8 );
	
	wxMenuItem* mnuGroupsSetGroupGroup9;
	mnuGroupsSetGroupGroup9 = new wxMenuItem( mnuGroupsSetGroup, wxID_ANY, wxString( wxT("Group 9") ) , wxEmptyString, wxITEM_NORMAL );
	mnuGroupsSetGroup->Append( mnuGroupsSetGroupGroup9 );
	
	mnuGroups->Append( mnuGroupsSetGroupItem );
	
	mbrFRED->Append( mnuGroups, wxT("Groups") ); 
	
	mnuMisc = new wxMenu();
	wxMenuItem* mnuMiscLevelObject;
	mnuMiscLevelObject = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Level Object") ) + wxT('\t') + wxT("L"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscLevelObject );
	
	wxMenuItem* mnuMiscAlignObject;
	mnuMiscAlignObject = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Align Object") ) + wxT('\t') + wxT("Ctrl+L"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscAlignObject );
	
	wxMenuItem* mnuMiscMarkWing;
	mnuMiscMarkWing = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Mark Wing") ) + wxT('\t') + wxT("W"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscMarkWing );
	
	wxMenuItem* mnuMiscControlObject;
	mnuMiscControlObject = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Control Object") ) + wxT('\t') + wxT("T"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscControlObject );
	
	wxMenuItem* mnuMiscNextObject;
	mnuMiscNextObject = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Next Object") ) + wxT('\t') + wxT("Tab"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscNextObject );
	
	wxMenuItem* mnuMiscPreviousObject;
	mnuMiscPreviousObject = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Prev Object") ) + wxT('\t') + wxT("Ctrl+Tab"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscPreviousObject );
	
	wxMenuItem* mnuMiscAdjustGrid;
	mnuMiscAdjustGrid = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Adjust Grid") ) , wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscAdjustGrid );
	
	wxMenuItem* mnuMiscNextSubsystem;
	mnuMiscNextSubsystem = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Next Subsystem") ) + wxT('\t') + wxT("K"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscNextSubsystem );
	
	wxMenuItem* mnuMiscPrevSubsystem;
	mnuMiscPrevSubsystem = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Prev Subsystem") ) + wxT('\t') + wxT("Shift+K"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscPrevSubsystem );
	
	wxMenuItem* mnuMiscCancelSubsystem;
	mnuMiscCancelSubsystem = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Cancel Subsystem") ) + wxT('\t') + wxT("Alt+K"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscCancelSubsystem );
	
	wxMenuItem* mnuMiscMissionStatistics;
	mnuMiscMissionStatistics = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Mission Statistics") ) + wxT('\t') + wxT("Ctrl+Shift+D"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscMissionStatistics );
	
	mnuMisc->AppendSeparator();
	
	wxMenuItem* mnuMiscErrorChecker;
	mnuMiscErrorChecker = new wxMenuItem( mnuMisc, wxID_ANY, wxString( wxT("Error Checker") ) + wxT('\t') + wxT("Shift+H"), wxEmptyString, wxITEM_NORMAL );
	mnuMisc->Append( mnuMiscErrorChecker );
	
	mbrFRED->Append( mnuMisc, wxT("Misc") ); 
	
	mnuHelp = new wxMenu();
	wxMenuItem* mnuHelpHelpTopics;
	mnuHelpHelpTopics = new wxMenuItem( mnuHelp, wxID_ANY, wxString( wxT("Help Topics") ) + wxT('\t') + wxT("F1"), wxEmptyString, wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpHelpTopics );
	
	wxMenuItem* mnuHelpShowSexpHelp;
	mnuHelpShowSexpHelp = new wxMenuItem( mnuHelp, wxID_ANY, wxString( wxT("Show SEXP Help") ) , wxEmptyString, wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpShowSexpHelp );
	
	mnuHelp->AppendSeparator();
	
	wxMenuItem* mnuHelpAbout;
	mnuHelpAbout = new wxMenuItem( mnuHelp, wxID_ANY, wxString( wxT("About wxFRED2...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpAbout );
	
	mbrFRED->Append( mnuHelp, wxT("Help") ); 
	
	this->SetMenuBar( mbrFRED );
	
	tbrFRED = this->CreateToolBar( wxTB_HORIZONTAL, wxID_ANY );
	tbrFRED->SetToolSeparation( 0 );
	tbrFRED->SetToolPacking( 0 );
	optSelect = tbrFRED->AddTool( wxID_ANY, wxEmptyString, select_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Select (S)"), wxT("Select objects only."), NULL ); 
	
	optSelectMove = tbrFRED->AddTool( wxID_ANY, wxEmptyString, selectmove_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Select and Move (M)"), wxT("Select and move selected objects."), NULL ); 
	
	optSelectRotate = tbrFRED->AddTool( wxID_ANY, wxT("Select and Rotate"), selectrot_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Select and Rotate (R)"), wxT("Select and rotate selected objects."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	chkRotateLocally = tbrFRED->AddTool( wxID_ANY, wxEmptyString, rotlocal_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Rotate Locally (X)"), wxT("Enable/disable local rotation for the selected group."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	optConstraintX = tbrFRED->AddTool( wxID_ANY, wxEmptyString, constx_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("X Constraint (`)"), wxT("Constrain actions to the global X axis."), NULL ); 
	
	optConstraintY = tbrFRED->AddTool( wxID_ANY, wxEmptyString, consty_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Y Constraint (`)"), wxT("Constrain actions to the global Y axis."), NULL ); 
	
	optConstraintZ = tbrFRED->AddTool( wxID_ANY, wxEmptyString, constz_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("Z Constraint (`)"), wxT("Constrain actions to the global Z axis."), NULL ); 
	
	optConstraintXZ = tbrFRED->AddTool( wxID_ANY, wxEmptyString, constxz_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("XZ Constraint (`)"), wxT("Constrain actions to the global XZ plane."), NULL ); 
	
	optConstraintYZ = tbrFRED->AddTool( wxID_ANY, wxEmptyString, constyz_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("YZ Constraint (`)"), wxT("Constrain actions to the global YZ plane."), NULL ); 
	
	optConstraintXY = tbrFRED->AddTool( wxID_ANY, wxEmptyString, constxy_png_to_wx_bitmap(), wxNullBitmap, wxITEM_RADIO, wxT("XY Constraint (`)"), wxT("Constrain actions to the global XY plane."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	btnSelectionList = tbrFRED->AddTool( wxID_ANY, wxEmptyString, selectlist_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Selection List (H)"), wxT("Select object(s) from a list."), NULL ); 
	
	chkSelectionLock = tbrFRED->AddTool( wxID_ANY, wxEmptyString, selectlock_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Selection Lock (L)"), wxT("Lock the current selection from changes."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	btnWingForm = tbrFRED->AddTool( wxID_ANY, wxEmptyString, wingform_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Form Wing (Ctrl+W)"), wxT("Adds the current selection to a wing."), NULL ); 
	
	btnWingDisband = tbrFRED->AddTool( wxID_ANY, wxEmptyString, wingdisband_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Disband Wing (Ctrl+D)"), wxT("Removes the current selection from (any) wing."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	btnZoomSelected = tbrFRED->AddTool( wxID_ANY, wxT("tool"), zoomsel_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Zoom Selected (Alt+Z)"), wxT("Zoom to view current selection."), NULL ); 
	
	btnZoomExtents = tbrFRED->AddTool( wxID_ANY, wxT("tool"), zoomext_png_to_wx_bitmap(), wxNullBitmap, wxITEM_NORMAL, wxT("Zoom Extents (Shift+Z)"), wxT("Zoom to view all objects in the mission."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	chkShowDistances = tbrFRED->AddTool( wxID_ANY, wxEmptyString, showdist_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Show Distances (D)"), wxT("Show the distances between all selected objects."), NULL ); 
	
	chkOrbitSelected = tbrFRED->AddTool( wxID_ANY, wxT("tool"), orbitsel_png_to_wx_bitmap(), wxNullBitmap, wxITEM_CHECK, wxT("Rotate about Selection (Ctrl+V)"), wxT("Rotate the vieport camera about the current selection."), NULL ); 
	
	tbrFRED->AddSeparator(); 
	
	tbrFRED->Realize(); 
	
	
	this->Centre( wxBOTH );
}

frmFRED::~frmFRED()
{
}

frmShipsEditor::frmShipsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxColour( 226, 226, 226 ) );
	
	mbrShipsEditor = new wxMenuBar( 0 );
	selectShip = new wxMenu();
	mbrShipsEditor->Append( selectShip, wxT("Select Ship") ); 
	
	this->SetMenuBar( mbrShipsEditor );
	
	wxBoxSizer* bSizer126;
	bSizer126 = new wxBoxSizer( wxVERTICAL );
	
	wxGridBagSizer* gbSizer12;
	gbSizer12 = new wxGridBagSizer( 0, 0 );
	gbSizer12->SetFlexibleDirection( wxBOTH );
	gbSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblShipName = new wxStaticText( this, wxID_ANY, wxT("Ship Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipName->Wrap( -1 );
	gbSizer12->Add( lblShipName, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtShipName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtShipName->SetMaxLength( 0 ); 
	gbSizer12->Add( txtShipName, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblWing = new wxStaticText( this, wxID_ANY, wxT("Wing:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWing->Wrap( -1 );
	gbSizer12->Add( lblWing, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtWing = new wxTextCtrl( this, wxID_ANY, wxT("None"), wxDefaultPosition, wxDefaultSize, 0 );
	txtWing->SetMaxLength( 0 ); 
	txtWing->Enable( false );
	
	gbSizer12->Add( txtWing, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblShipClass = new wxStaticText( this, wxID_ANY, wxT("Ship Class:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipClass->Wrap( -1 );
	gbSizer12->Add( lblShipClass, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboShipClassChoices;
	cboShipClass = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboShipClassChoices, 0 );
	cboShipClass->SetSelection( 0 );
	gbSizer12->Add( cboShipClass, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblHotkey = new wxStaticText( this, wxID_ANY, wxT("Hotkey:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHotkey->Wrap( -1 );
	gbSizer12->Add( lblHotkey, wxGBPosition( 1, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboHotkeyChoices;
	cboHotkey = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboHotkeyChoices, 0 );
	cboHotkey->SetSelection( 0 );
	gbSizer12->Add( cboHotkey, wxGBPosition( 1, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblAIClass = new wxStaticText( this, wxID_ANY, wxT("AI Class:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAIClass->Wrap( -1 );
	gbSizer12->Add( lblAIClass, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboAIClassChoices;
	cboAIClass = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboAIClassChoices, 0 );
	cboAIClass->SetSelection( 0 );
	gbSizer12->Add( cboAIClass, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblPersona = new wxStaticText( this, wxID_ANY, wxT("Persona:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPersona->Wrap( -1 );
	gbSizer12->Add( lblPersona, wxGBPosition( 2, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboPersonaChoices;
	cboPersona = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboPersonaChoices, 0 );
	cboPersona->SetSelection( 0 );
	gbSizer12->Add( cboPersona, wxGBPosition( 2, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblTeam = new wxStaticText( this, wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTeam->Wrap( -1 );
	gbSizer12->Add( lblTeam, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboTeamChoices;
	cboTeam = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboTeamChoices, 0 );
	cboTeam->SetSelection( 0 );
	gbSizer12->Add( cboTeam, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblKillScore = new wxStaticText( this, wxID_ANY, wxT("Kill Score:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblKillScore->Wrap( -1 );
	gbSizer12->Add( lblKillScore, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtKillscore = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtKillscore->SetMaxLength( 0 ); 
	gbSizer12->Add( txtKillscore, wxGBPosition( 3, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblCargoCargo = new wxStaticText( this, wxID_ANY, wxT("Cargo:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCargoCargo->Wrap( -1 );
	gbSizer12->Add( lblCargoCargo, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboCargo = new wxComboBox( this, wxID_ANY, wxT("None"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer12->Add( cboCargo, wxGBPosition( 4, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblAssistPercentage = new wxStaticText( this, wxID_ANY, wxT("Assist %:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAssistPercentage->Wrap( -1 );
	gbSizer12->Add( lblAssistPercentage, wxGBPosition( 4, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAssistPercentage = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAssistPercentage->SetMaxLength( 0 ); 
	gbSizer12->Add( txtAssistPercentage, wxGBPosition( 4, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblAltName = new wxStaticText( this, wxID_ANY, wxT("Alt Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAltName->Wrap( -1 );
	gbSizer12->Add( lblAltName, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboAltName = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer12->Add( cboAltName, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	chkPlayerShip = new wxCheckBox( this, wxID_ANY, wxT("Player Ship"), wxDefaultPosition, wxDefaultSize, 0 );
	chkPlayerShip->Enable( false );
	
	gbSizer12->Add( chkPlayerShip, wxGBPosition( 5, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblCallsign = new wxStaticText( this, wxID_ANY, wxT("Callsign:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCallsign->Wrap( -1 );
	gbSizer12->Add( lblCallsign, wxGBPosition( 6, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboCallsign = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer12->Add( cboCallsign, wxGBPosition( 6, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnMakePlayerShip = new wxButton( this, wxID_ANY, wxT("Set As Player Ship"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnMakePlayerShip, wxGBPosition( 6, 3 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnTextureReplacement = new wxButton( this, wxID_ANY, wxT("Texture Replacement"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnTextureReplacement, wxGBPosition( 7, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnAltShipClass = new wxButton( this, wxID_ANY, wxT("Alt Ship Class"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnAltShipClass, wxGBPosition( 7, 3 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer127;
	bSizer127 = new wxBoxSizer( wxHORIZONTAL );
	
	btnPrevWing = new wxButton( this, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer127->Add( btnPrevWing, 0, wxALL|wxEXPAND, 3 );
	
	btnNextWing = new wxButton( this, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer127->Add( btnNextWing, 0, wxALL|wxEXPAND, 3 );
	
	
	gbSizer12->Add( bSizer127, wxGBPosition( 0, 4 ), wxGBSpan( 1, 1 ), wxEXPAND, 3 );
	
	btnDelete = new wxButton( this, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnDelete, wxGBPosition( 1, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnReset = new wxButton( this, wxID_ANY, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnReset, wxGBPosition( 2, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnWeapons = new wxButton( this, wxID_ANY, wxT("Weapons"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnWeapons, wxGBPosition( 3, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnPlayerOrders = new wxButton( this, wxID_ANY, wxT("Player Orders"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnPlayerOrders, wxGBPosition( 4, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnSpecialExplosion = new wxButton( this, wxID_ANY, wxT("Special Exp"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnSpecialExplosion, wxGBPosition( 5, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnSpecialHits = new wxButton( this, wxID_ANY, wxT("Special Hits"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer12->Add( btnSpecialHits, wxGBPosition( 6, 4 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	
	bSizer126->Add( gbSizer12, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer128;
	bSizer128 = new wxBoxSizer( wxHORIZONTAL );
	
	btnMiscOptions = new wxButton( this, wxID_ANY, wxT("Misc"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer128->Add( btnMiscOptions, 1, wxALL, 3 );
	
	btnInitialStatus = new wxButton( this, wxID_ANY, wxT("Initial Status"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer128->Add( btnInitialStatus, 1, wxALL, 3 );
	
	btnInitialOrders = new wxButton( this, wxID_ANY, wxT("Initial Orders"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer128->Add( btnInitialOrders, 1, wxALL, 3 );
	
	btnTBLInfo = new wxButton( this, wxID_ANY, wxT("TBL Info"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer128->Add( btnTBLInfo, 1, wxALL, 3 );
	
	btnHideCues = new wxButton( this, wxID_ANY, wxT("Hide Cues"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer128->Add( btnHideCues, 1, wxALL, 3 );
	
	
	bSizer126->Add( bSizer128, 0, wxALIGN_CENTER|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer129;
	bSizer129 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer49;
	sbSizer49 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Arrival") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer44;
	fgSizer44 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer44->SetFlexibleDirection( wxBOTH );
	fgSizer44->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblArrivalLocation = new wxStaticText( sbSizer49->GetStaticBox(), wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalLocation->Wrap( -1 );
	fgSizer44->Add( lblArrivalLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalLocationChoices;
	cboArrivalLocation = new wxChoice( sbSizer49->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboArrivalLocationChoices, 0 );
	cboArrivalLocation->SetSelection( 0 );
	fgSizer44->Add( cboArrivalLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalTarget = new wxStaticText( sbSizer49->GetStaticBox(), wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalTarget->Wrap( -1 );
	fgSizer44->Add( lblArrivalTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalTargetChoices;
	cboArrivalTarget = new wxChoice( sbSizer49->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboArrivalTargetChoices, 0 );
	cboArrivalTarget->SetSelection( 0 );
	fgSizer44->Add( cboArrivalTarget, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDistance = new wxStaticText( sbSizer49->GetStaticBox(), wxID_ANY, wxT("Distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDistance->Wrap( -1 );
	fgSizer44->Add( lblArrivalDistance, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtArrivalDistance = new wxTextCtrl( sbSizer49->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,21 ), 0 );
	txtArrivalDistance->SetMaxLength( 0 ); 
	fgSizer44->Add( txtArrivalDistance, 0, wxALL, 3 );
	
	lblArrivalDelay = new wxStaticText( sbSizer49->GetStaticBox(), wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelay->Wrap( -1 );
	fgSizer44->Add( lblArrivalDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer130;
	bSizer130 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay = new wxSpinCtrl( sbSizer49->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( -1,21 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer130->Add( spnArrivalDelay, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDelaySeconds = new wxStaticText( sbSizer49->GetStaticBox(), wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelaySeconds->Wrap( -1 );
	bSizer130->Add( lblArrivalDelaySeconds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer44->Add( bSizer130, 1, wxEXPAND, 5 );
	
	
	sbSizer49->Add( fgSizer44, 0, wxALL|wxEXPAND, 3 );
	
	btnRestrictArrivalPaths = new wxButton( sbSizer49->GetStaticBox(), wxID_ANY, wxT("Restrict Arrival Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer49->Add( btnRestrictArrivalPaths, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalCue = new wxStaticText( sbSizer49->GetStaticBox(), wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalCue->Wrap( -1 );
	sbSizer49->Add( lblArrivalCue, 0, wxALL, 3 );
	
	tctArrivalCues = new wxTreeCtrl( sbSizer49->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE );
	sbSizer49->Add( tctArrivalCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoArrivalWarp = new wxCheckBox( sbSizer49->GetStaticBox(), wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer49->Add( chkNoArrivalWarp, 0, wxALL, 5 );
	
	
	bSizer129->Add( sbSizer49, 1, wxEXPAND|wxRIGHT|wxTOP, 3 );
	
	wxStaticBoxSizer* sbSizer50;
	sbSizer50 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Departure") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer45;
	fgSizer45 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer45->SetFlexibleDirection( wxBOTH );
	fgSizer45->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDepatureLocation = new wxStaticText( sbSizer50->GetStaticBox(), wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepatureLocation->Wrap( -1 );
	fgSizer45->Add( lblDepatureLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureLocationChoices;
	cboDepartureLocation = new wxChoice( sbSizer50->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboDepartureLocationChoices, 0 );
	cboDepartureLocation->SetSelection( 0 );
	fgSizer45->Add( cboDepartureLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureTarget = new wxStaticText( sbSizer50->GetStaticBox(), wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureTarget->Wrap( -1 );
	fgSizer45->Add( lblDepartureTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureTargetChoices;
	cboDepartureTarget = new wxChoice( sbSizer50->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,21 ), cboDepartureTargetChoices, 0 );
	cboDepartureTarget->SetSelection( 0 );
	fgSizer45->Add( cboDepartureTarget, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer45->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer45->Add( 0, 27, 1, wxEXPAND, 5 );
	
	lblDepartureDelay = new wxStaticText( sbSizer50->GetStaticBox(), wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureDelay->Wrap( -1 );
	fgSizer45->Add( lblDepartureDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer131;
	bSizer131 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay1 = new wxSpinCtrl( sbSizer50->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( -1,21 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer131->Add( spnArrivalDelay1, 0, wxALL|wxEXPAND, 3 );
	
	m_staticText1711 = new wxStaticText( sbSizer50->GetStaticBox(), wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1711->Wrap( -1 );
	bSizer131->Add( m_staticText1711, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer45->Add( bSizer131, 0, wxALL, 3 );
	
	
	sbSizer50->Add( fgSizer45, 0, wxEXPAND, 5 );
	
	btnRestrictDeparturePaths = new wxButton( sbSizer50->GetStaticBox(), wxID_ANY, wxT("Restrict Departure Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer50->Add( btnRestrictDeparturePaths, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureCue = new wxStaticText( sbSizer50->GetStaticBox(), wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureCue->Wrap( -1 );
	sbSizer50->Add( lblDepartureCue, 0, wxALL, 3 );
	
	tctDepartureCues = new wxTreeCtrl( sbSizer50->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE );
	sbSizer50->Add( tctDepartureCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoDepartureWarp = new wxCheckBox( sbSizer50->GetStaticBox(), wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer50->Add( chkNoDepartureWarp, 0, wxALL, 5 );
	
	
	bSizer129->Add( sbSizer50, 1, wxEXPAND|wxLEFT|wxTOP, 3 );
	
	
	bSizer126->Add( bSizer129, 1, wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer126 );
	this->Layout();
	bSizer126->Fit( this );
	
	this->Centre( wxBOTH );
}

frmShipsEditor::~frmShipsEditor()
{
}

frmWingEditor::frmWingEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxColour( 226, 226, 226 ) );
	
	wxBoxSizer* bSizer132;
	bSizer132 = new wxBoxSizer( wxVERTICAL );
	
	pnlProperties = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxGridBagSizer* gbSizer13;
	gbSizer13 = new wxGridBagSizer( 0, 0 );
	gbSizer13->SetFlexibleDirection( wxBOTH );
	gbSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblWingName = new wxStaticText( pnlProperties, wxID_ANY, wxT("Wing Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWingName->Wrap( -1 );
	gbSizer13->Add( lblWingName, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	txtWingName = new wxTextCtrl( pnlProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtWingName->SetMaxLength( 0 ); 
	gbSizer13->Add( txtWingName, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblLeader = new wxStaticText( pnlProperties, wxID_ANY, wxT("Leader:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLeader->Wrap( -1 );
	gbSizer13->Add( lblLeader, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	wxArrayString cboWingLeaderChoices;
	cboWingLeader = new wxChoice( pnlProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboWingLeaderChoices, 0 );
	cboWingLeader->SetSelection( 0 );
	gbSizer13->Add( cboWingLeader, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblWaveNumber = new wxStaticText( pnlProperties, wxID_ANY, wxT("# of Waves"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWaveNumber->Wrap( -1 );
	gbSizer13->Add( lblWaveNumber, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnWaveNumber = new wxSpinCtrl( pnlProperties, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
	gbSizer13->Add( spnWaveNumber, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblWaveThreshold = new wxStaticText( pnlProperties, wxID_ANY, wxT("Wave Threshold:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWaveThreshold->Wrap( -1 );
	gbSizer13->Add( lblWaveThreshold, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnWaveThreshold = new wxSpinCtrl( pnlProperties, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer13->Add( spnWaveThreshold, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	lblHotkey = new wxStaticText( pnlProperties, wxID_ANY, wxT("Hotkey:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHotkey->Wrap( -1 );
	gbSizer13->Add( lblHotkey, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	wxArrayString cboHotkeyChoices;
	cboHotkey = new wxChoice( pnlProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboHotkeyChoices, 0 );
	cboHotkey->SetSelection( 0 );
	gbSizer13->Add( cboHotkey, wxGBPosition( 4, 1 ), wxGBSpan( 1, 1 ), wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	btnSquadLogo = new wxButton( pnlProperties, wxID_ANY, wxT("Squad Logo"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer13->Add( btnSquadLogo, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 3 );
	
	txtSquadLogo = new wxTextCtrl( pnlProperties, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSquadLogo->SetMaxLength( 0 ); 
	gbSizer13->Add( txtSquadLogo, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 3 );
	
	wxBoxSizer* bSizer133;
	bSizer133 = new wxBoxSizer( wxVERTICAL );
	
	chkReinforcement = new wxCheckBox( pnlProperties, wxID_ANY, wxT("Reinforcement Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer133->Add( chkReinforcement, 0, wxALL, 3 );
	
	chkIgnoreForGoals = new wxCheckBox( pnlProperties, wxID_ANY, wxT("Ignore for counting Goals"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer133->Add( chkIgnoreForGoals, 0, wxALL, 3 );
	
	chkNoArrivalMusic = new wxCheckBox( pnlProperties, wxID_ANY, wxT("No Arrival Music"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer133->Add( chkNoArrivalMusic, 0, wxALL, 3 );
	
	chkNoArrivalMessage = new wxCheckBox( pnlProperties, wxID_ANY, wxT("No Arrival Message"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer133->Add( chkNoArrivalMessage, 0, wxALL, 3 );
	
	chkNoDynamicGoals = new wxCheckBox( pnlProperties, wxID_ANY, wxT("No Dynamic Goals"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer133->Add( chkNoDynamicGoals, 0, wxALL, 3 );
	
	
	gbSizer13->Add( bSizer133, wxGBPosition( 0, 2 ), wxGBSpan( 5, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer134;
	bSizer134 = new wxBoxSizer( wxHORIZONTAL );
	
	btnPrev = new wxButton( pnlProperties, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer134->Add( btnPrev, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnNext = new wxButton( pnlProperties, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer134->Add( btnNext, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	
	gbSizer13->Add( bSizer134, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	btnDeleteWing = new wxButton( pnlProperties, wxID_ANY, wxT("Delete Wing"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer13->Add( btnDeleteWing, wxGBPosition( 1, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnDisbandWing = new wxButton( pnlProperties, wxID_ANY, wxT("Disband Wing"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer13->Add( btnDisbandWing, wxGBPosition( 2, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnInitialOrders = new wxButton( pnlProperties, wxID_ANY, wxT("Initial Orders"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer13->Add( btnInitialOrders, wxGBPosition( 3, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	btnHideCues = new wxToggleButton( pnlProperties, wxID_ANY, wxT("Hide Cues"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer13->Add( btnHideCues, wxGBPosition( 4, 3 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	
	gbSizer13->AddGrowableCol( 1 );
	
	pnlProperties->SetSizer( gbSizer13 );
	pnlProperties->Layout();
	gbSizer13->Fit( pnlProperties );
	bSizer132->Add( pnlProperties, 0, wxALL|wxEXPAND, 3 );
	
	pnlCues = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer135;
	bSizer135 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer51;
	sbSizer51 = new wxStaticBoxSizer( new wxStaticBox( pnlCues, wxID_ANY, wxT("Delay Between Waves (Seconds)") ), wxHORIZONTAL );
	
	lblMinWaveDelay = new wxStaticText( sbSizer51->GetStaticBox(), wxID_ANY, wxT("Min:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMinWaveDelay->Wrap( -1 );
	sbSizer51->Add( lblMinWaveDelay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnMinWaveDelay = new wxSpinCtrl( sbSizer51->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	sbSizer51->Add( spnMinWaveDelay, 1, wxALL, 3 );
	
	lblMaxWaveDelay = new wxStaticText( sbSizer51->GetStaticBox(), wxID_ANY, wxT("Max:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMaxWaveDelay->Wrap( -1 );
	sbSizer51->Add( lblMaxWaveDelay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 3 );
	
	spnMaxWaveDelay = new wxSpinCtrl( sbSizer51->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	sbSizer51->Add( spnMaxWaveDelay, 1, wxALL, 3 );
	
	
	bSizer135->Add( sbSizer51, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer136;
	bSizer136 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer52;
	sbSizer52 = new wxStaticBoxSizer( new wxStaticBox( pnlCues, wxID_ANY, wxT("Arrival") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer46;
	fgSizer46 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer46->SetFlexibleDirection( wxBOTH );
	fgSizer46->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblArrivalLocation = new wxStaticText( sbSizer52->GetStaticBox(), wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalLocation->Wrap( -1 );
	fgSizer46->Add( lblArrivalLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalLocationChoices;
	cboArrivalLocation = new wxChoice( sbSizer52->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboArrivalLocationChoices, 0 );
	cboArrivalLocation->SetSelection( 0 );
	fgSizer46->Add( cboArrivalLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalTarget = new wxStaticText( sbSizer52->GetStaticBox(), wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalTarget->Wrap( -1 );
	fgSizer46->Add( lblArrivalTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboArrivalTargetChoices;
	cboArrivalTarget = new wxChoice( sbSizer52->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboArrivalTargetChoices, 0 );
	cboArrivalTarget->SetSelection( 0 );
	fgSizer46->Add( cboArrivalTarget, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDistance = new wxStaticText( sbSizer52->GetStaticBox(), wxID_ANY, wxT("Distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDistance->Wrap( -1 );
	fgSizer46->Add( lblArrivalDistance, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtArrivalDistance = new wxTextCtrl( sbSizer52->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtArrivalDistance->SetMaxLength( 0 ); 
	fgSizer46->Add( txtArrivalDistance, 0, wxALL, 3 );
	
	lblArrivalDelay = new wxStaticText( sbSizer52->GetStaticBox(), wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelay->Wrap( -1 );
	fgSizer46->Add( lblArrivalDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer137;
	bSizer137 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay = new wxSpinCtrl( sbSizer52->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer137->Add( spnArrivalDelay, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalDelaySeconds = new wxStaticText( sbSizer52->GetStaticBox(), wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalDelaySeconds->Wrap( -1 );
	bSizer137->Add( lblArrivalDelaySeconds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer46->Add( bSizer137, 1, wxEXPAND, 5 );
	
	
	sbSizer52->Add( fgSizer46, 0, wxALL|wxEXPAND, 3 );
	
	btnRestrictArrivalPaths = new wxButton( sbSizer52->GetStaticBox(), wxID_ANY, wxT("Restrict Arrival Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer52->Add( btnRestrictArrivalPaths, 0, wxALL|wxEXPAND, 3 );
	
	lblArrivalCue = new wxStaticText( sbSizer52->GetStaticBox(), wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArrivalCue->Wrap( -1 );
	sbSizer52->Add( lblArrivalCue, 0, wxALL, 3 );
	
	tctArrivalCues = new wxTreeCtrl( sbSizer52->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,106 ), wxTR_DEFAULT_STYLE );
	sbSizer52->Add( tctArrivalCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoArrivalWarp = new wxCheckBox( sbSizer52->GetStaticBox(), wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer52->Add( chkNoArrivalWarp, 0, wxALL, 5 );
	
	
	bSizer136->Add( sbSizer52, 1, wxEXPAND|wxRIGHT|wxTOP, 3 );
	
	wxStaticBoxSizer* sbSizer53;
	sbSizer53 = new wxStaticBoxSizer( new wxStaticBox( pnlCues, wxID_ANY, wxT("Departure") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer47;
	fgSizer47 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer47->SetFlexibleDirection( wxBOTH );
	fgSizer47->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDepatureLocation = new wxStaticText( sbSizer53->GetStaticBox(), wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepatureLocation->Wrap( -1 );
	fgSizer47->Add( lblDepatureLocation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureLocationChoices;
	cboDepartureLocation = new wxChoice( sbSizer53->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboDepartureLocationChoices, 0 );
	cboDepartureLocation->SetSelection( 0 );
	fgSizer47->Add( cboDepartureLocation, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureTarget = new wxStaticText( sbSizer53->GetStaticBox(), wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureTarget->Wrap( -1 );
	fgSizer47->Add( lblDepartureTarget, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboDepartureTargetChoices;
	cboDepartureTarget = new wxChoice( sbSizer53->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboDepartureTargetChoices, 0 );
	cboDepartureTarget->SetSelection( 0 );
	fgSizer47->Add( cboDepartureTarget, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer47->Add( 0, 27, 0, 0, 5 );
	
	
	fgSizer47->Add( 0, 0, 0, 0, 5 );
	
	lblDepartureDelay = new wxStaticText( sbSizer53->GetStaticBox(), wxID_ANY, wxT("Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureDelay->Wrap( -1 );
	fgSizer47->Add( lblDepartureDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer138;
	bSizer138 = new wxBoxSizer( wxHORIZONTAL );
	
	spnArrivalDelay1 = new wxSpinCtrl( sbSizer53->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	bSizer138->Add( spnArrivalDelay1, 0, wxALL|wxEXPAND, 3 );
	
	m_staticText1711 = new wxStaticText( sbSizer53->GetStaticBox(), wxID_ANY, wxT("Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1711->Wrap( -1 );
	bSizer138->Add( m_staticText1711, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	fgSizer47->Add( bSizer138, 0, wxEXPAND, 3 );
	
	
	sbSizer53->Add( fgSizer47, 0, wxALL|wxEXPAND, 3 );
	
	btnRestrictDeparturePaths = new wxButton( sbSizer53->GetStaticBox(), wxID_ANY, wxT("Restrict Departure Paths"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer53->Add( btnRestrictDeparturePaths, 0, wxALL|wxEXPAND, 3 );
	
	lblDepartureCue = new wxStaticText( sbSizer53->GetStaticBox(), wxID_ANY, wxT("Cue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDepartureCue->Wrap( -1 );
	sbSizer53->Add( lblDepartureCue, 0, wxALL, 3 );
	
	tctDepartureCues = new wxTreeCtrl( sbSizer53->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,106 ), wxTR_DEFAULT_STYLE );
	sbSizer53->Add( tctDepartureCues, 1, wxALL|wxEXPAND, 3 );
	
	chkNoDepartureWarp = new wxCheckBox( sbSizer53->GetStaticBox(), wxID_ANY, wxT("No Warp Effect"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer53->Add( chkNoDepartureWarp, 0, wxALL, 5 );
	
	
	bSizer136->Add( sbSizer53, 1, wxEXPAND|wxLEFT|wxTOP, 3 );
	
	
	bSizer135->Add( bSizer136, 1, wxEXPAND, 5 );
	
	
	pnlCues->SetSizer( bSizer135 );
	pnlCues->Layout();
	bSizer135->Fit( pnlCues );
	bSizer132->Add( pnlCues, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer132 );
	this->Layout();
	bSizer132->Fit( this );
	mbrWingEditor = new wxMenuBar( 0 );
	mnuSelectWing = new wxMenu();
	mbrWingEditor->Append( mnuSelectWing, wxT("Select Wing") ); 
	
	this->SetMenuBar( mbrWingEditor );
	
	
	this->Centre( wxBOTH );
}

frmWingEditor::~frmWingEditor()
{
}

dlgObjectEditor::dlgObjectEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer139;
	bSizer139 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer54;
	sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Position") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer48;
	fgSizer48 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer48->SetFlexibleDirection( wxBOTH );
	fgSizer48->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText217 = new wxStaticText( sbSizer54->GetStaticBox(), wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText217->Wrap( -1 );
	fgSizer48->Add( m_staticText217, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPositionX = new wxSpinCtrl( sbSizer54->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer48->Add( spnPositionX, 0, wxALL, 3 );
	
	m_staticText218 = new wxStaticText( sbSizer54->GetStaticBox(), wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText218->Wrap( -1 );
	fgSizer48->Add( m_staticText218, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPositionY = new wxSpinCtrl( sbSizer54->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer48->Add( spnPositionY, 0, wxALL, 3 );
	
	m_staticText220 = new wxStaticText( sbSizer54->GetStaticBox(), wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText220->Wrap( -1 );
	fgSizer48->Add( m_staticText220, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPositionZ = new wxSpinCtrl( sbSizer54->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer48->Add( spnPositionZ, 0, wxALL, 3 );
	
	
	sbSizer54->Add( fgSizer48, 0, wxEXPAND, 5 );
	
	
	bSizer139->Add( sbSizer54, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer55;
	sbSizer55 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Orientation") ), wxVERTICAL );
	
	chkPointTo = new wxCheckBox( sbSizer55->GetStaticBox(), wxID_ANY, wxT("Face/Point towards..."), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer55->Add( chkPointTo, 0, wxALL, 3 );
	
	pnlOrientation = new wxPanel( sbSizer55->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxGridBagSizer* gbSizer14;
	gbSizer14 = new wxGridBagSizer( 0, 0 );
	gbSizer14->SetFlexibleDirection( wxBOTH );
	gbSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	optObject = new wxRadioButton( pnlOrientation, wxID_ANY, wxT("Object:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optObject->SetValue( true ); 
	gbSizer14->Add( optObject, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	wxString cbObjectChoices[] = { wxT("No object") };
	int cbObjectNChoices = sizeof( cbObjectChoices ) / sizeof( wxString );
	cbObject = new wxChoice( pnlOrientation, wxID_ANY, wxDefaultPosition, wxSize( 140,-1 ), cbObjectNChoices, cbObjectChoices, 0 );
	cbObject->SetSelection( 0 );
	gbSizer14->Add( cbObject, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_RIGHT|wxALL, 3 );
	
	optLocation = new wxRadioButton( pnlOrientation, wxID_ANY, wxT("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer14->Add( optLocation, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer49;
	fgSizer49 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer49->SetFlexibleDirection( wxBOTH );
	fgSizer49->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblLocationX = new wxStaticText( pnlOrientation, wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLocationX->Wrap( -1 );
	lblLocationX->Enable( false );
	
	fgSizer49->Add( lblLocationX, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnLocationX = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	spnLocationX->Enable( false );
	
	fgSizer49->Add( spnLocationX, 0, wxALL, 3 );
	
	lblLocationY = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLocationY->Wrap( -1 );
	lblLocationY->Enable( false );
	
	fgSizer49->Add( lblLocationY, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnLocationY = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	spnLocationY->Enable( false );
	
	fgSizer49->Add( spnLocationY, 0, wxALL, 3 );
	
	lblLocationZ = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLocationZ->Wrap( -1 );
	lblLocationZ->Enable( false );
	
	fgSizer49->Add( lblLocationZ, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnLocationZ = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	spnLocationZ->Enable( false );
	
	fgSizer49->Add( spnLocationZ, 0, wxALL, 3 );
	
	
	gbSizer14->Add( fgSizer49, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_RIGHT|wxALL, 3 );
	
	optAngle = new wxRadioButton( pnlOrientation, wxID_ANY, wxT("Angle:"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer14->Add( optAngle, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer50;
	fgSizer50 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer50->SetFlexibleDirection( wxBOTH );
	fgSizer50->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblPitch = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPitch->Wrap( -1 );
	lblPitch->Enable( false );
	
	fgSizer50->Add( lblPitch, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnPitch = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 360, 0 );
	spnPitch->Enable( false );
	
	fgSizer50->Add( spnPitch, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblBank = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBank->Wrap( -1 );
	lblBank->Enable( false );
	
	fgSizer50->Add( lblBank, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnBank = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 360, 0 );
	spnBank->Enable( false );
	
	fgSizer50->Add( spnBank, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblHeading = new wxStaticText( pnlOrientation, wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHeading->Wrap( -1 );
	lblHeading->Enable( false );
	
	fgSizer50->Add( lblHeading, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnHeading = new wxSpinCtrl( pnlOrientation, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	spnHeading->Enable( false );
	
	fgSizer50->Add( spnHeading, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	gbSizer14->Add( fgSizer50, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	
	pnlOrientation->SetSizer( gbSizer14 );
	pnlOrientation->Layout();
	gbSizer14->Fit( pnlOrientation );
	sbSizer55->Add( pnlOrientation, 0, wxEXPAND, 3 );
	
	
	bSizer139->Add( sbSizer55, 1, wxALL|wxEXPAND, 3 );
	
	m_sdbSizer6 = new wxStdDialogButtonSizer();
	m_sdbSizer6OK = new wxButton( this, wxID_OK );
	m_sdbSizer6->AddButton( m_sdbSizer6OK );
	m_sdbSizer6Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer6->AddButton( m_sdbSizer6Cancel );
	m_sdbSizer6->Realize();
	
	bSizer139->Add( m_sdbSizer6, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	this->SetSizer( bSizer139 );
	this->Layout();
	bSizer139->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgObjectEditor::~dlgObjectEditor()
{
}

frmWaypointEditor::frmWaypointEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxColour( 226, 226, 226 ) );
	
	menuWaypoint = new wxMenuBar( 0 );
	menuPaths = new wxMenu();
	menuWaypoint->Append( menuPaths, wxT("Select Waypoint Path") ); 
	
	this->SetMenuBar( menuWaypoint );
	
	wxBoxSizer* bSizer140;
	bSizer140 = new wxBoxSizer( wxHORIZONTAL );
	
	lblWaypointName = new wxStaticText( this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWaypointName->Wrap( -1 );
	bSizer140->Add( lblWaypointName, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtWaypointName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtWaypointName->SetMaxLength( 0 ); 
	bSizer140->Add( txtWaypointName, 1, wxALIGN_CENTER|wxALL, 3 );
	
	
	this->SetSizer( bSizer140 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

frmWaypointEditor::~frmWaypointEditor()
{
}

dlgMissionObjectivesEditor::dlgMissionObjectivesEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer141;
	bSizer141 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer142;
	bSizer142 = new wxBoxSizer( wxVERTICAL );
	
	tctObjectives = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE );
	bSizer142->Add( tctObjectives, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer141->Add( bSizer142, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer143;
	bSizer143 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer144;
	bSizer144 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText117 = new wxStaticText( this, wxID_ANY, wxT("Display Types"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText117->Wrap( -1 );
	bSizer144->Add( m_staticText117, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString m_choice27Choices[] = { wxT("Primary Goals"), wxT("Secondary Goals"), wxT("Bonus Goals") };
	int m_choice27NChoices = sizeof( m_choice27Choices ) / sizeof( wxString );
	m_choice27 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice27NChoices, m_choice27Choices, 0 );
	m_choice27->SetSelection( 0 );
	bSizer144->Add( m_choice27, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer143->Add( bSizer144, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer56;
	sbSizer56 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Current Objective") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer51;
	fgSizer51 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer51->AddGrowableCol( 1 );
	fgSizer51->SetFlexibleDirection( wxBOTH );
	fgSizer51->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblObjType = new wxStaticText( sbSizer56->GetStaticBox(), wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjType->Wrap( -1 );
	fgSizer51->Add( lblObjType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboObjTypeChoices[] = { wxT("Primary Goal"), wxT("Secondary Goal"), wxT("Bonus Goal") };
	int cboObjTypeNChoices = sizeof( cboObjTypeChoices ) / sizeof( wxString );
	cboObjType = new wxChoice( sbSizer56->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjTypeNChoices, cboObjTypeChoices, 0 );
	cboObjType->SetSelection( 0 );
	fgSizer51->Add( cboObjType, 0, wxALL|wxEXPAND, 3 );
	
	lblObjName = new wxStaticText( sbSizer56->GetStaticBox(), wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjName->Wrap( -1 );
	fgSizer51->Add( lblObjName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtObjName = new wxTextCtrl( sbSizer56->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtObjName->SetMaxLength( 0 ); 
	fgSizer51->Add( txtObjName, 0, wxALL|wxEXPAND, 3 );
	
	lblObjText = new wxStaticText( sbSizer56->GetStaticBox(), wxID_ANY, wxT("Objective Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjText->Wrap( -1 );
	fgSizer51->Add( lblObjText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtObjText = new wxTextCtrl( sbSizer56->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtObjText->SetMaxLength( 0 ); 
	fgSizer51->Add( txtObjText, 0, wxALL|wxEXPAND, 3 );
	
	lblObjScore = new wxStaticText( sbSizer56->GetStaticBox(), wxID_ANY, wxT("Score:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjScore->Wrap( -1 );
	fgSizer51->Add( lblObjScore, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtObjScore = new wxTextCtrl( sbSizer56->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtObjScore->SetMaxLength( 0 ); 
	fgSizer51->Add( txtObjScore, 0, wxALL|wxEXPAND, 3 );
	
	lblObjTeam = new wxStaticText( sbSizer56->GetStaticBox(), wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblObjTeam->Wrap( -1 );
	fgSizer51->Add( lblObjTeam, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjTeamChoices;
	cboObjTeam = new wxChoice( sbSizer56->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjTeamChoices, 0 );
	cboObjTeam->SetSelection( 0 );
	fgSizer51->Add( cboObjTeam, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer56->Add( fgSizer51, 0, wxEXPAND, 5 );
	
	cboCurrObjInvalid = new wxCheckBox( sbSizer56->GetStaticBox(), wxID_ANY, wxT("Objective Invalid"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer56->Add( cboCurrObjInvalid, 0, wxALL, 3 );
	
	cboCurrObjNoCompletionSound = new wxCheckBox( sbSizer56->GetStaticBox(), wxID_ANY, wxT("Don't Play Completion Sound"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer56->Add( cboCurrObjNoCompletionSound, 0, wxALL, 3 );
	
	
	bSizer143->Add( sbSizer56, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer145;
	bSizer145 = new wxBoxSizer( wxHORIZONTAL );
	
	btnNewObjective = new wxButton( this, wxID_ANY, wxT("New Obj."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer145->Add( btnNewObjective, 0, wxALL, 3 );
	
	btnConfirm = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer145->Add( btnConfirm, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer145->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer143->Add( bSizer145, 0, 0, 3 );
	
	
	bSizer141->Add( bSizer143, 1, wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer141 );
	this->Layout();
	bSizer141->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgMissionObjectivesEditor::~dlgMissionObjectivesEditor()
{
}

dlgEventsEditor::dlgEventsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer146;
	bSizer146 = new wxBoxSizer( wxHORIZONTAL );
	
	pnlEvents = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer147;
	bSizer147 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer148;
	bSizer148 = new wxBoxSizer( wxHORIZONTAL );
	
	trbSexp = new wxTreeCtrl( pnlEvents, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE );
	bSizer148->Add( trbSexp, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer149;
	bSizer149 = new wxBoxSizer( wxVERTICAL );
	
	btnNewEvent = new wxButton( pnlEvents, wxID_ANY, wxT("New Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer149->Add( btnNewEvent, 0, wxALL|wxEXPAND, 3 );
	
	btnInsertEvent = new wxButton( pnlEvents, wxID_ANY, wxT("Insert Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer149->Add( btnInsertEvent, 0, wxALL|wxEXPAND, 3 );
	
	btnDeleteEvent = new wxButton( pnlEvents, wxID_ANY, wxT("Delete Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer149->Add( btnDeleteEvent, 0, wxALL|wxEXPAND, 3 );
	
	lblRepeatCount = new wxStaticText( pnlEvents, wxID_ANY, wxT("Repeat Count:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRepeatCount->Wrap( -1 );
	bSizer149->Add( lblRepeatCount, 0, wxLEFT|wxTOP, 3 );
	
	txtRepeatCount = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtRepeatCount->SetMaxLength( 0 ); 
	bSizer149->Add( txtRepeatCount, 0, wxALL, 3 );
	
	lblTriggerCount = new wxStaticText( pnlEvents, wxID_ANY, wxT("Trigger Count:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTriggerCount->Wrap( -1 );
	bSizer149->Add( lblTriggerCount, 0, wxLEFT|wxTOP, 3 );
	
	txtTriggerCount = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtTriggerCount->SetMaxLength( 0 ); 
	bSizer149->Add( txtTriggerCount, 0, wxALL, 3 );
	
	lblIntervalTime = new wxStaticText( pnlEvents, wxID_ANY, wxT("Interval Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIntervalTime->Wrap( -1 );
	bSizer149->Add( lblIntervalTime, 0, wxLEFT|wxTOP, 3 );
	
	txtIntervalTime = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtIntervalTime->SetMaxLength( 0 ); 
	bSizer149->Add( txtIntervalTime, 0, wxALL, 3 );
	
	lblScore = new wxStaticText( pnlEvents, wxID_ANY, wxT("Score:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblScore->Wrap( -1 );
	bSizer149->Add( lblScore, 0, wxLEFT|wxTOP, 3 );
	
	txtScore = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtScore->SetMaxLength( 0 ); 
	bSizer149->Add( txtScore, 0, wxALL, 3 );
	
	lblTeam = new wxStaticText( pnlEvents, wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTeam->Wrap( -1 );
	bSizer149->Add( lblTeam, 0, wxLEFT|wxTOP, 3 );
	
	wxString cboTeamChoices[] = { wxT("Team 1"), wxT("Team 2") };
	int cboTeamNChoices = sizeof( cboTeamChoices ) / sizeof( wxString );
	cboTeam = new wxChoice( pnlEvents, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboTeamNChoices, cboTeamChoices, 0 );
	cboTeam->SetSelection( 0 );
	bSizer149->Add( cboTeam, 1, wxALL|wxEXPAND, 3 );
	
	chkChained = new wxCheckBox( pnlEvents, wxID_ANY, wxT("Chained Event"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer149->Add( chkChained, 1, wxALL, 5 );
	
	lblChainDelay = new wxStaticText( pnlEvents, wxID_ANY, wxT("Chain Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblChainDelay->Wrap( -1 );
	bSizer149->Add( lblChainDelay, 0, wxLEFT|wxTOP, 3 );
	
	txtChainDelay = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtChainDelay->SetMaxLength( 0 ); 
	bSizer149->Add( txtChainDelay, 0, wxALL, 3 );
	
	
	bSizer148->Add( bSizer149, 0, wxEXPAND, 5 );
	
	
	bSizer147->Add( bSizer148, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer52;
	fgSizer52 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer52->AddGrowableCol( 1 );
	fgSizer52->SetFlexibleDirection( wxBOTH );
	fgSizer52->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDirectiveText = new wxStaticText( pnlEvents, wxID_ANY, wxT("Directive Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDirectiveText->Wrap( -1 );
	fgSizer52->Add( lblDirectiveText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtDirectiveText = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtDirectiveText->SetMaxLength( 0 ); 
	fgSizer52->Add( txtDirectiveText, 1, wxALL|wxEXPAND, 3 );
	
	lblDirectiveKeypress = new wxStaticText( pnlEvents, wxID_ANY, wxT("Directive Keypress Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDirectiveKeypress->Wrap( -1 );
	fgSizer52->Add( lblDirectiveKeypress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtDirectiveKeypress = new wxTextCtrl( pnlEvents, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtDirectiveKeypress->SetMaxLength( 0 ); 
	fgSizer52->Add( txtDirectiveKeypress, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer147->Add( fgSizer52, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer57;
	sbSizer57 = new wxStaticBoxSizer( new wxStaticBox( pnlEvents, wxID_ANY, wxT("Event Logging") ), wxVERTICAL );
	
	wxBoxSizer* bSizer150;
	bSizer150 = new wxBoxSizer( wxVERTICAL );
	
	lblStateLogging = new wxStaticText( sbSizer57->GetStaticBox(), wxID_ANY, wxT("States to log to Event.log:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStateLogging->Wrap( -1 );
	bSizer150->Add( lblStateLogging, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer53;
	fgSizer53 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer53->SetFlexibleDirection( wxBOTH );
	fgSizer53->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkTrue = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("True"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkTrue, 0, wxALL, 3 );
	
	chkTrueAlways = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("Always True"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkTrueAlways, 0, wxALL, 3 );
	
	chkRepeatFirst = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("First Repeat"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkRepeatFirst, 0, wxALL, 3 );
	
	chkTriggerFirst = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("First Trigger"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkTriggerFirst, 0, wxALL, 3 );
	
	chkFalse = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("False"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkFalse, 0, wxALL, 3 );
	
	chkFalseAlways = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("Always False"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkFalseAlways, 0, wxALL, 3 );
	
	chkRepeatLast = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("Last Repeat"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkRepeatLast, 0, wxALL, 3 );
	
	chkTriggerLast = new wxCheckBox( sbSizer57->GetStaticBox(), wxID_ANY, wxT("Last Trigger"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer53->Add( chkTriggerLast, 0, wxALL, 3 );
	
	
	bSizer150->Add( fgSizer53, 1, wxEXPAND, 5 );
	
	
	sbSizer57->Add( bSizer150, 1, wxEXPAND, 5 );
	
	
	bSizer147->Add( sbSizer57, 0, wxALL|wxEXPAND, 3 );
	
	
	pnlEvents->SetSizer( bSizer147 );
	pnlEvents->Layout();
	bSizer147->Fit( pnlEvents );
	bSizer146->Add( pnlEvents, 1, wxEXPAND, 3 );
	
	wxBoxSizer* bSizer151;
	bSizer151 = new wxBoxSizer( wxVERTICAL );
	
	pnlMessages = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer58;
	sbSizer58 = new wxStaticBoxSizer( new wxStaticBox( pnlMessages, wxID_ANY, wxT("Messages") ), wxVERTICAL );
	
	wxBoxSizer* bSizer152;
	bSizer152 = new wxBoxSizer( wxHORIZONTAL );
	
	lstMessages = new wxListBox( sbSizer58->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	bSizer152->Add( lstMessages, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer153;
	bSizer153 = new wxBoxSizer( wxVERTICAL );
	
	
	bSizer153->Add( 0, 0, 1, wxEXPAND, 5 );
	
	btnNewMessage = new wxButton( sbSizer58->GetStaticBox(), wxID_ANY, wxT("New"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer153->Add( btnNewMessage, 0, wxALL|wxEXPAND, 3 );
	
	btnDeleteMessage = new wxButton( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer153->Add( btnDeleteMessage, 0, wxALL|wxEXPAND, 3 );
	
	
	bSizer152->Add( bSizer153, 0, wxEXPAND, 5 );
	
	
	sbSizer58->Add( bSizer152, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer154;
	bSizer154 = new wxBoxSizer( wxHORIZONTAL );
	
	lblMessageName = new wxStaticText( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageName->Wrap( -1 );
	bSizer154->Add( lblMessageName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtMessageName = new wxTextCtrl( sbSizer58->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtMessageName->SetMaxLength( 0 ); 
	bSizer154->Add( txtMessageName, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer58->Add( bSizer154, 0, wxEXPAND, 3 );
	
	lblMessageText = new wxStaticText( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Message Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageText->Wrap( -1 );
	sbSizer58->Add( lblMessageText, 0, wxLEFT|wxTOP, 3 );
	
	txtMessageText = new wxTextCtrl( sbSizer58->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,64 ), wxTE_MULTILINE );
	txtMessageText->SetMaxLength( 0 ); 
	sbSizer58->Add( txtMessageText, 0, wxALL|wxEXPAND, 3 );
	
	wxGridBagSizer* gbSizer15;
	gbSizer15 = new wxGridBagSizer( 0, 0 );
	gbSizer15->SetFlexibleDirection( wxBOTH );
	gbSizer15->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblMessageANI = new wxStaticText( sbSizer58->GetStaticBox(), wxID_ANY, wxT("ANI File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageANI->Wrap( -1 );
	gbSizer15->Add( lblMessageANI, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboMessageANI = new wxComboBox( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer15->Add( cboMessageANI, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	btnANIBrowse = new wxButton( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer15->Add( btnANIBrowse, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblMessageAudio = new wxStaticText( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Audio File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageAudio->Wrap( -1 );
	gbSizer15->Add( lblMessageAudio, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboMessageAudio = new wxComboBox( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer15->Add( cboMessageAudio, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer155;
	bSizer155 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAudioBrowse = new wxButton( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer155->Add( btnAudioBrowse, 0, wxALL, 3 );
	
	btnPlayAudio = new wxBitmapButton( sbSizer58->GetStaticBox(), wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	bSizer155->Add( btnPlayAudio, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	
	gbSizer15->Add( bSizer155, wxGBPosition( 1, 2 ), wxGBSpan( 1, 1 ), wxEXPAND, 3 );
	
	lblPersona = new wxStaticText( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Persona:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPersona->Wrap( -1 );
	gbSizer15->Add( lblPersona, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_comboBox9 = new wxComboBox( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer15->Add( m_comboBox9, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	btnUpdateStuff = new wxButton( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Update Stuff"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer15->Add( btnUpdateStuff, wxGBPosition( 2, 2 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	lblMessageTeam = new wxStaticText( sbSizer58->GetStaticBox(), wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessageTeam->Wrap( -1 );
	gbSizer15->Add( lblMessageTeam, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboTeamMessageChoices;
	cboTeamMessage = new wxChoice( sbSizer58->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboTeamMessageChoices, 0 );
	cboTeamMessage->SetSelection( 0 );
	gbSizer15->Add( cboTeamMessage, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	m_staticText139 = new wxStaticText( sbSizer58->GetStaticBox(), wxID_ANY, wxT("(TvT only)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText139->Wrap( -1 );
	gbSizer15->Add( m_staticText139, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer58->Add( gbSizer15, 0, wxALIGN_CENTER, 5 );
	
	
	pnlMessages->SetSizer( sbSizer58 );
	pnlMessages->Layout();
	sbSizer58->Fit( pnlMessages );
	bSizer151->Add( pnlMessages, 2, wxALL|wxEXPAND, 3 );
	
	m_sdbSizer7 = new wxStdDialogButtonSizer();
	m_sdbSizer7OK = new wxButton( this, wxID_OK );
	m_sdbSizer7->AddButton( m_sdbSizer7OK );
	m_sdbSizer7Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer7->AddButton( m_sdbSizer7Cancel );
	m_sdbSizer7->Realize();
	
	bSizer151->Add( m_sdbSizer7, 0, wxEXPAND, 5 );
	
	
	bSizer146->Add( bSizer151, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer146 );
	this->Layout();
	bSizer146->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgEventsEditor::~dlgEventsEditor()
{
}

frmTeamLoadoutEditor::frmTeamLoadoutEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxColour( 226, 226, 226 ) );
	
	mnbDialogMenu = new wxMenuBar( 0 );
	mnuSelectTeam = new wxMenu();
	wxMenuItem* mnuTeam1;
	mnuTeam1 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 1") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuTeam1 );
	
	wxMenuItem* mnuTeam2;
	mnuTeam2 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 2") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuTeam2 );
	
	mnbDialogMenu->Append( mnuSelectTeam, wxT("Select Team") ); 
	
	m_menu16 = new wxMenu();
	wxMenuItem* meuBalanceTeams;
	meuBalanceTeams = new wxMenuItem( m_menu16, wxID_ANY, wxString( wxT("Balance Teams") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu16->Append( meuBalanceTeams );
	
	mnbDialogMenu->Append( m_menu16, wxT("Options") ); 
	
	this->SetMenuBar( mnbDialogMenu );
	
	wxBoxSizer* bSizer156;
	bSizer156 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer157;
	bSizer157 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer158;
	bSizer158 = new wxBoxSizer( wxVERTICAL );
	
	lblAvailableStartShips = new wxStaticText( this, wxID_ANY, wxT("Available Starting Ships:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAvailableStartShips->Wrap( -1 );
	bSizer158->Add( lblAvailableStartShips, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblShipsFromVariable = new wxStaticText( this, wxID_ANY, wxT("From Variable:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipsFromVariable->Wrap( -1 );
	bSizer158->Add( lblShipsFromVariable, 0, wxALL, 3 );
	
	lbxStartShipsVariable = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_ALWAYS_SB|wxLB_SINGLE|wxLB_SORT ); 
	bSizer158->Add( lbxStartShipsVariable, 1, wxALL|wxEXPAND, 3 );
	
	lblShipsFromTbl = new wxStaticText( this, wxID_ANY, wxT("From Table Entry:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipsFromTbl->Wrap( -1 );
	bSizer158->Add( lblShipsFromTbl, 0, wxALL, 3 );
	
	wxArrayString cklShipsFromTblChoices;
	cklShipsFromTbl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cklShipsFromTblChoices, wxLB_ALWAYS_SB|wxLB_SINGLE );
	bSizer158->Add( cklShipsFromTbl, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer159;
	bSizer159 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText141 = new wxStaticText( this, wxID_ANY, wxT("Extra Available:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText141->Wrap( -1 );
	bSizer159->Add( m_staticText141, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnExtraShipsAvailable = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 99, 0 );
	bSizer159->Add( spnExtraShipsAvailable, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer158->Add( bSizer159, 0, wxEXPAND, 3 );
	
	lblSetShipAmountFromVariable = new wxStaticText( this, wxID_ANY, wxT("Set amount from variable:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSetShipAmountFromVariable->Wrap( -1 );
	bSizer158->Add( lblSetShipAmountFromVariable, 0, wxALL, 3 );
	
	wxArrayString cboSetShipAmountFromVariableChoices;
	cboSetShipAmountFromVariable = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboSetShipAmountFromVariableChoices, 0 );
	cboSetShipAmountFromVariable->SetSelection( 0 );
	bSizer158->Add( cboSetShipAmountFromVariable, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer160;
	bSizer160 = new wxBoxSizer( wxHORIZONTAL );
	
	lblAmountOfShipsInWings = new wxStaticText( this, wxID_ANY, wxT("Amount used in Wings:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmountOfShipsInWings->Wrap( -1 );
	bSizer160->Add( lblAmountOfShipsInWings, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAmountOfShipsInWings = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	txtAmountOfShipsInWings->SetMaxLength( 0 ); 
	bSizer160->Add( txtAmountOfShipsInWings, 0, wxALL, 3 );
	
	
	bSizer158->Add( bSizer160, 0, 0, 3 );
	
	
	bSizer157->Add( bSizer158, 1, wxALL|wxEXPAND, 3 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer157->Add( m_staticline2, 0, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer161;
	bSizer161 = new wxBoxSizer( wxVERTICAL );
	
	lblAvailableStartWeapons = new wxStaticText( this, wxID_ANY, wxT("Available Starting Weapons:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAvailableStartWeapons->Wrap( -1 );
	bSizer161->Add( lblAvailableStartWeapons, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblWeaponsFromVariable = new wxStaticText( this, wxID_ANY, wxT("From Variable:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWeaponsFromVariable->Wrap( -1 );
	bSizer161->Add( lblWeaponsFromVariable, 0, wxALL, 3 );
	
	lbxStartWeaponsVariable = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_ALWAYS_SB|wxLB_SINGLE ); 
	bSizer161->Add( lbxStartWeaponsVariable, 1, wxALL|wxEXPAND, 3 );
	
	lblWeaponsFromTbl = new wxStaticText( this, wxID_ANY, wxT("From Table Entry:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblWeaponsFromTbl->Wrap( -1 );
	bSizer161->Add( lblWeaponsFromTbl, 0, wxALL, 3 );
	
	wxArrayString cklWeaponsFromTblChoices;
	cklWeaponsFromTbl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cklWeaponsFromTblChoices, wxLB_ALWAYS_SB );
	bSizer161->Add( cklWeaponsFromTbl, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer162;
	bSizer162 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText1411 = new wxStaticText( this, wxID_ANY, wxT("Extra available"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1411->Wrap( -1 );
	bSizer162->Add( m_staticText1411, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnExtraWeaponsAvailable = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 99999, 0 );
	bSizer162->Add( spnExtraWeaponsAvailable, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer161->Add( bSizer162, 0, wxEXPAND, 3 );
	
	lblSetWeaponAmountFromVariable = new wxStaticText( this, wxID_ANY, wxT("Set amount from variable"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSetWeaponAmountFromVariable->Wrap( -1 );
	bSizer161->Add( lblSetWeaponAmountFromVariable, 0, wxALL, 3 );
	
	wxArrayString cboSetWeaponAmountFromVariableChoices;
	cboSetWeaponAmountFromVariable = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboSetWeaponAmountFromVariableChoices, 0 );
	cboSetWeaponAmountFromVariable->SetSelection( 0 );
	bSizer161->Add( cboSetWeaponAmountFromVariable, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer163;
	bSizer163 = new wxBoxSizer( wxHORIZONTAL );
	
	lblAmountOfWeaponsInWings = new wxStaticText( this, wxID_ANY, wxT("Amount used in Wings:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmountOfWeaponsInWings->Wrap( -1 );
	bSizer163->Add( lblAmountOfWeaponsInWings, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAmountOfWeaponsInWings = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	txtAmountOfWeaponsInWings->SetMaxLength( 0 ); 
	bSizer163->Add( txtAmountOfWeaponsInWings, 0, wxALL, 3 );
	
	
	bSizer161->Add( bSizer163, 0, 0, 3 );
	
	
	bSizer157->Add( bSizer161, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer156->Add( bSizer157, 1, wxEXPAND, 5 );
	
	m_sdbSizer8 = new wxStdDialogButtonSizer();
	m_sdbSizer8OK = new wxButton( this, wxID_OK );
	m_sdbSizer8->AddButton( m_sdbSizer8OK );
	m_sdbSizer8Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer8->AddButton( m_sdbSizer8Cancel );
	m_sdbSizer8->Realize();
	
	bSizer156->Add( m_sdbSizer8, 0, wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer156 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

frmTeamLoadoutEditor::~frmTeamLoadoutEditor()
{
}

dlgBackgroundEditor::dlgBackgroundEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer164;
	bSizer164 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer165;
	bSizer165 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer166;
	bSizer166 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString cboBackgroundPresetChoices[] = { wxT("Background 1"), wxT("Background 2") };
	int cboBackgroundPresetNChoices = sizeof( cboBackgroundPresetChoices ) / sizeof( wxString );
	cboBackgroundPreset = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboBackgroundPresetNChoices, cboBackgroundPresetChoices, 0 );
	cboBackgroundPreset->SetSelection( 0 );
	bSizer166->Add( cboBackgroundPreset, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnImportBackground = new wxButton( this, wxID_ANY, wxT("Import"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer166->Add( btnImportBackground, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer165->Add( bSizer166, 0, 0, 5 );
	
	wxStaticBoxSizer* sbSizer59;
	sbSizer59 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bitmaps") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer167;
	bSizer167 = new wxBoxSizer( wxVERTICAL );
	
	lclBGBitmaps = new wxListCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON|wxLC_SINGLE_SEL );
	bSizer167->Add( lclBGBitmaps, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer168;
	bSizer168 = new wxBoxSizer( wxHORIZONTAL );
	
	btnBitmapAdd = new wxButton( sbSizer59->GetStaticBox(), wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer168->Add( btnBitmapAdd, 1, wxALL|wxEXPAND, 3 );
	
	btnBitmapDelete = new wxButton( sbSizer59->GetStaticBox(), wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer168->Add( btnBitmapDelete, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer167->Add( bSizer168, 0, wxEXPAND, 3 );
	
	
	sbSizer59->Add( bSizer167, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer169;
	bSizer169 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer170;
	bSizer170 = new wxBoxSizer( wxHORIZONTAL );
	
	lblBitmap = new wxStaticText( sbSizer59->GetStaticBox(), wxID_ANY, wxT("Bitmap:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmap->Wrap( -1 );
	bSizer170->Add( lblBitmap, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString cboBitmapSelectChoices;
	cboBitmapSelect = new wxChoice( sbSizer59->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboBitmapSelectChoices, 0 );
	cboBitmapSelect->SetSelection( 0 );
	bSizer170->Add( cboBitmapSelect, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer169->Add( bSizer170, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer54;
	fgSizer54 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer54->SetFlexibleDirection( wxBOTH );
	fgSizer54->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch = new wxStaticText( sbSizer59->GetStaticBox(), wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch->Wrap( -1 );
	fgSizer54->Add( lblBitmapPitch, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank = new wxStaticText( sbSizer59->GetStaticBox(), wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank->Wrap( -1 );
	fgSizer54->Add( lblBitmapBank, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading = new wxStaticText( sbSizer59->GetStaticBox(), wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading->Wrap( -1 );
	fgSizer54->Add( lblBitmapHeading, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch = new wxSpinCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer54->Add( spnBitmapPitch, 0, wxALL, 3 );
	
	spnBitmapBank = new wxSpinCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer54->Add( spnBitmapBank, 0, wxALL, 3 );
	
	spnBitmapHeading = new wxSpinCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer54->Add( spnBitmapHeading, 0, wxALL, 3 );
	
	
	bSizer169->Add( fgSizer54, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxGridBagSizer* gbSizer16;
	gbSizer16 = new wxGridBagSizer( 0, 0 );
	gbSizer16->SetFlexibleDirection( wxBOTH );
	gbSizer16->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapScale = new wxStaticText( sbSizer59->GetStaticBox(), wxID_ANY, wxT("Scale (x/y):"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapScale->Wrap( -1 );
	gbSizer16->Add( lblBitmapScale, wxGBPosition( 0, 0 ), wxGBSpan( 1, 2 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapScaleX = new wxSpinCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer16->Add( spnBitmapScaleX, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapScaleY = new wxSpinCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer16->Add( spnBitmapScaleY, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer169->Add( gbSizer16, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxGridBagSizer* gbSizer17;
	gbSizer17 = new wxGridBagSizer( 0, 0 );
	gbSizer17->SetFlexibleDirection( wxBOTH );
	gbSizer17->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapDivisions = new wxStaticText( sbSizer59->GetStaticBox(), wxID_ANY, wxT("# of Divisions (x/y):"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapDivisions->Wrap( -1 );
	gbSizer17->Add( lblBitmapDivisions, wxGBPosition( 0, 0 ), wxGBSpan( 1, 2 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapDivisionsX = new wxSpinCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer17->Add( spnBitmapDivisionsX, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapDivisionsY = new wxSpinCtrl( sbSizer59->GetStaticBox(), wxID_ANY, wxT("1"), wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 18, 1 );
	gbSizer17->Add( spnBitmapDivisionsY, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer169->Add( gbSizer17, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	sbSizer59->Add( bSizer169, 0, wxEXPAND, 5 );
	
	
	bSizer165->Add( sbSizer59, 1, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer60;
	sbSizer60 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Nebula") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer171;
	bSizer171 = new wxBoxSizer( wxVERTICAL );
	
	chkFullNebula = new wxCheckBox( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Full Nebula"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer171->Add( chkFullNebula, 0, wxALL, 3 );
	
	chkToggleShipTrails = new wxCheckBox( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Toggle ship trails"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer171->Add( chkToggleShipTrails, 0, wxALL, 3 );
	
	wxFlexGridSizer* fgSizer55;
	fgSizer55 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer55->SetFlexibleDirection( wxBOTH );
	fgSizer55->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblNebulaRange = new wxStaticText( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Range:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaRange->Wrap( -1 );
	fgSizer55->Add( lblNebulaRange, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtNebulaRange = new wxTextCtrl( sbSizer60->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtNebulaRange->SetMaxLength( 0 ); 
	fgSizer55->Add( txtNebulaRange, 0, wxALL|wxEXPAND, 3 );
	
	lblNebulaPattern = new wxStaticText( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Pattern:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaPattern->Wrap( -1 );
	fgSizer55->Add( lblNebulaPattern, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboNebulaPatternChoices[] = { wxT("nbackblue1"), wxT("nbackblue2"), wxT("nbackcyan"), wxT("nbackgreen"), wxT("nbackpurp1"), wxT("nbackpurp2"), wxT("nbackred"), wxT("nbackblack") };
	int cboNebulaPatternNChoices = sizeof( cboNebulaPatternChoices ) / sizeof( wxString );
	cboNebulaPattern = new wxChoice( sbSizer60->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboNebulaPatternNChoices, cboNebulaPatternChoices, 0 );
	cboNebulaPattern->SetSelection( 0 );
	fgSizer55->Add( cboNebulaPattern, 0, wxALL|wxEXPAND, 3 );
	
	lblLightningStorm = new wxStaticText( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Lightning Storm:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLightningStorm->Wrap( -1 );
	fgSizer55->Add( lblLightningStorm, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboLightningStormChoices[] = { wxT("none"), wxT("s&standard"), wxT("s&active"), wxT("s&emp"), wxT("s&medium") };
	int cboLightningStormNChoices = sizeof( cboLightningStormChoices ) / sizeof( wxString );
	cboLightningStorm = new wxChoice( sbSizer60->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboLightningStormNChoices, cboLightningStormChoices, 0 );
	cboLightningStorm->SetSelection( 0 );
	fgSizer55->Add( cboLightningStorm, 0, wxALL|wxEXPAND, 3 );
	
	lblNebulaFogNear = new wxStaticText( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Fog Near:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaFogNear->Wrap( -1 );
	fgSizer55->Add( lblNebulaFogNear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinCtrl49 = new wxSpinCtrl( sbSizer60->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer55->Add( m_spinCtrl49, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblNebulaFogMultiplier = new wxStaticText( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Fog Multiplier:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaFogMultiplier->Wrap( -1 );
	fgSizer55->Add( lblNebulaFogMultiplier, 0, wxALL, 5 );
	
	m_spinCtrl50 = new wxSpinCtrl( sbSizer60->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer55->Add( m_spinCtrl50, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer171->Add( fgSizer55, 0, wxEXPAND, 5 );
	
	
	sbSizer60->Add( bSizer171, 0, 0, 5 );
	
	wxBoxSizer* bSizer172;
	bSizer172 = new wxBoxSizer( wxVERTICAL );
	
	lblNebulaPoofs = new wxStaticText( sbSizer60->GetStaticBox(), wxID_ANY, wxT("Nebula Poofs:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNebulaPoofs->Wrap( -1 );
	bSizer172->Add( lblNebulaPoofs, 0, wxALL, 3 );
	
	wxString clbNebulaPoofsChoices[] = { wxT("PoofGreen01"), wxT("PoofGreen02"), wxT("PoofRed01"), wxT("PoofRed02"), wxT("PoofPurp01"), wxT("PoofPurp02") };
	int clbNebulaPoofsNChoices = sizeof( clbNebulaPoofsChoices ) / sizeof( wxString );
	clbNebulaPoofs = new wxCheckListBox( sbSizer60->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, clbNebulaPoofsNChoices, clbNebulaPoofsChoices, wxLB_ALWAYS_SB );
	bSizer172->Add( clbNebulaPoofs, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer60->Add( bSizer172, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer165->Add( sbSizer60, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer61;
	sbSizer61 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("FS1 Nebula") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer56;
	fgSizer56 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer56->SetFlexibleDirection( wxBOTH );
	fgSizer56->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText160 = new wxStaticText( sbSizer61->GetStaticBox(), wxID_ANY, wxT("Pattern:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText160->Wrap( -1 );
	fgSizer56->Add( m_staticText160, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboNebulaPattern1Choices[] = { wxT("None"), wxT("Pattern01"), wxT("Pattern02"), wxT("Pattern03") };
	int cboNebulaPattern1NChoices = sizeof( cboNebulaPattern1Choices ) / sizeof( wxString );
	cboNebulaPattern1 = new wxChoice( sbSizer61->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboNebulaPattern1NChoices, cboNebulaPattern1Choices, 0 );
	cboNebulaPattern1->SetSelection( 0 );
	fgSizer56->Add( cboNebulaPattern1, 0, wxALL|wxEXPAND, 3 );
	
	m_staticText161 = new wxStaticText( sbSizer61->GetStaticBox(), wxID_ANY, wxT("Color"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText161->Wrap( -1 );
	fgSizer56->Add( m_staticText161, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString cboNebulaColourChoices[] = { wxT("Red"), wxT("Blue"), wxT("Gold"), wxT("Purple"), wxT("Maroon"), wxT("Green"), wxT("Grey Blue"), wxT("Violet"), wxT("Grey Green") };
	int cboNebulaColourNChoices = sizeof( cboNebulaColourChoices ) / sizeof( wxString );
	cboNebulaColour = new wxChoice( sbSizer61->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboNebulaColourNChoices, cboNebulaColourChoices, 0 );
	cboNebulaColour->SetSelection( 0 );
	fgSizer56->Add( cboNebulaColour, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer61->Add( fgSizer56, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	wxFlexGridSizer* fgSizer57;
	fgSizer57 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer57->SetFlexibleDirection( wxBOTH );
	fgSizer57->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch2 = new wxStaticText( sbSizer61->GetStaticBox(), wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch2->Wrap( -1 );
	fgSizer57->Add( lblBitmapPitch2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank2 = new wxStaticText( sbSizer61->GetStaticBox(), wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank2->Wrap( -1 );
	fgSizer57->Add( lblBitmapBank2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading2 = new wxStaticText( sbSizer61->GetStaticBox(), wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading2->Wrap( -1 );
	fgSizer57->Add( lblBitmapHeading2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch2 = new wxSpinCtrl( sbSizer61->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer57->Add( spnBitmapPitch2, 0, wxALL, 3 );
	
	spnBitmapBank2 = new wxSpinCtrl( sbSizer61->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer57->Add( spnBitmapBank2, 0, wxALL, 3 );
	
	spnBitmapHeading2 = new wxSpinCtrl( sbSizer61->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer57->Add( spnBitmapHeading2, 0, wxALL, 3 );
	
	
	sbSizer61->Add( fgSizer57, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer165->Add( sbSizer61, 0, wxEXPAND, 3 );
	
	
	bSizer164->Add( bSizer165, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer173;
	bSizer173 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer174;
	bSizer174 = new wxBoxSizer( wxHORIZONTAL );
	
	btnBGSwap = new wxButton( this, wxID_ANY, wxT("Swap with"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer174->Add( btnBGSwap, 0, wxALL, 3 );
	
	wxString cboBackgroundSwapChoices[] = { wxT("Background 1"), wxT("Background 2") };
	int cboBackgroundSwapNChoices = sizeof( cboBackgroundSwapChoices ) / sizeof( wxString );
	cboBackgroundSwap = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, cboBackgroundSwapNChoices, cboBackgroundSwapChoices, 0 );
	cboBackgroundSwap->SetSelection( 0 );
	bSizer174->Add( cboBackgroundSwap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer173->Add( bSizer174, 0, wxALIGN_RIGHT, 5 );
	
	wxStaticBoxSizer* sbSizer62;
	sbSizer62 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Suns") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer175;
	bSizer175 = new wxBoxSizer( wxVERTICAL );
	
	lclBGSunbitmaps = new wxListCtrl( sbSizer62->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON );
	bSizer175->Add( lclBGSunbitmaps, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer176;
	bSizer176 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAddSunBitmap = new wxButton( sbSizer62->GetStaticBox(), wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer176->Add( btnAddSunBitmap, 1, wxALL|wxEXPAND, 3 );
	
	btnDeleteSunBitmap = new wxButton( sbSizer62->GetStaticBox(), wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer176->Add( btnDeleteSunBitmap, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer175->Add( bSizer176, 0, wxEXPAND, 3 );
	
	
	sbSizer62->Add( bSizer175, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer177;
	bSizer177 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer178;
	bSizer178 = new wxBoxSizer( wxHORIZONTAL );
	
	lblSun = new wxStaticText( sbSizer62->GetStaticBox(), wxID_ANY, wxT("Sun:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSun->Wrap( -1 );
	bSizer178->Add( lblSun, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxArrayString cboSunSelectChoices;
	cboSunSelect = new wxChoice( sbSizer62->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboSunSelectChoices, 0 );
	cboSunSelect->SetSelection( 0 );
	bSizer178->Add( cboSunSelect, 0, wxALL, 3 );
	
	
	bSizer177->Add( bSizer178, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxFlexGridSizer* fgSizer58;
	fgSizer58 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer58->SetFlexibleDirection( wxBOTH );
	fgSizer58->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch1 = new wxStaticText( sbSizer62->GetStaticBox(), wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch1->Wrap( -1 );
	fgSizer58->Add( lblBitmapPitch1, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank1 = new wxStaticText( sbSizer62->GetStaticBox(), wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank1->Wrap( -1 );
	fgSizer58->Add( lblBitmapBank1, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading1 = new wxStaticText( sbSizer62->GetStaticBox(), wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading1->Wrap( -1 );
	fgSizer58->Add( lblBitmapHeading1, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch1 = new wxSpinCtrl( sbSizer62->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer58->Add( spnBitmapPitch1, 0, wxALL, 3 );
	
	spnBitmapBank1 = new wxSpinCtrl( sbSizer62->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer58->Add( spnBitmapBank1, 0, wxALL, 3 );
	
	spnBitmapHeading1 = new wxSpinCtrl( sbSizer62->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer58->Add( spnBitmapHeading1, 0, wxALL, 3 );
	
	
	bSizer177->Add( fgSizer58, 1, wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer179;
	bSizer179 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText179 = new wxStaticText( sbSizer62->GetStaticBox(), wxID_ANY, wxT("Scale:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText179->Wrap( -1 );
	bSizer179->Add( m_staticText179, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnSunScale = new wxSpinCtrl( sbSizer62->GetStaticBox(), wxID_ANY, wxT("10"), wxDefaultPosition, wxSize( 74,-1 ), wxSP_ARROW_KEYS, 0, 10, 10 );
	bSizer179->Add( spnSunScale, 0, wxALL, 3 );
	
	
	bSizer177->Add( bSizer179, 0, wxALIGN_CENTER|wxALL|wxSHAPED, 3 );
	
	
	sbSizer62->Add( bSizer177, 1, 0, 5 );
	
	
	bSizer173->Add( sbSizer62, 1, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer63;
	sbSizer63 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Ambient Light") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer59;
	fgSizer59 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer59->AddGrowableCol( 1 );
	fgSizer59->SetFlexibleDirection( wxBOTH );
	fgSizer59->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblAmbientRed = new wxStaticText( sbSizer63->GetStaticBox(), wxID_ANY, wxT("Red:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmbientRed->Wrap( -1 );
	fgSizer59->Add( lblAmbientRed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldAmbientRed = new wxSlider( sbSizer63->GetStaticBox(), wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer59->Add( sldAmbientRed, 1, wxALL|wxEXPAND, 3 );
	
	spnAmbientRed = new wxSpinCtrl( sbSizer63->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 48,-1 ), wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer59->Add( spnAmbientRed, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblAmbientGreen = new wxStaticText( sbSizer63->GetStaticBox(), wxID_ANY, wxT("Green:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmbientGreen->Wrap( -1 );
	fgSizer59->Add( lblAmbientGreen, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldAmbientGreen = new wxSlider( sbSizer63->GetStaticBox(), wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer59->Add( sldAmbientGreen, 0, wxALL|wxEXPAND, 3 );
	
	spnAmbientGreen = new wxSpinCtrl( sbSizer63->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 48,-1 ), wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer59->Add( spnAmbientGreen, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblAmbientBlue = new wxStaticText( sbSizer63->GetStaticBox(), wxID_ANY, wxT("Blue:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAmbientBlue->Wrap( -1 );
	fgSizer59->Add( lblAmbientBlue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldAmbientBlue = new wxSlider( sbSizer63->GetStaticBox(), wxID_ANY, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer59->Add( sldAmbientBlue, 0, wxALL|wxEXPAND, 3 );
	
	spnAmbientBlue = new wxSpinCtrl( sbSizer63->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 48,-1 ), wxSP_ARROW_KEYS, 0, 255, 0 );
	fgSizer59->Add( spnAmbientBlue, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	sbSizer63->Add( fgSizer59, 0, wxEXPAND, 5 );
	
	
	bSizer173->Add( sbSizer63, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer64;
	sbSizer64 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Skybox") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer60;
	fgSizer60 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer60->AddGrowableCol( 1 );
	fgSizer60->SetFlexibleDirection( wxBOTH );
	fgSizer60->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnSkyboxSelect = new wxButton( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Skybox Model"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer60->Add( btnSkyboxSelect, 0, wxALL|wxEXPAND, 3 );
	
	txtSkybox = new wxTextCtrl( sbSizer64->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSkybox->SetMaxLength( 0 ); 
	fgSizer60->Add( txtSkybox, 0, wxALL|wxEXPAND, 3 );
	
	btnSkyboxMap = new wxButton( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Skybox Map"), wxDefaultPosition, wxDefaultSize, 0 );
	btnSkyboxMap->Enable( false );
	
	fgSizer60->Add( btnSkyboxMap, 0, wxALL|wxEXPAND, 3 );
	
	m_textCtrl73 = new wxTextCtrl( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Coming Soon!"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrl73->SetMaxLength( 0 ); 
	m_textCtrl73->Enable( false );
	
	fgSizer60->Add( m_textCtrl73, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer64->Add( fgSizer60, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer180;
	bSizer180 = new wxBoxSizer( wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer61;
	fgSizer61 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer61->SetFlexibleDirection( wxBOTH );
	fgSizer61->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBitmapPitch21 = new wxStaticText( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Pitch:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapPitch21->Wrap( -1 );
	fgSizer61->Add( lblBitmapPitch21, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapBank21 = new wxStaticText( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Bank:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapBank21->Wrap( -1 );
	fgSizer61->Add( lblBitmapBank21, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblBitmapHeading21 = new wxStaticText( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Heading:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBitmapHeading21->Wrap( -1 );
	fgSizer61->Add( lblBitmapHeading21, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnBitmapPitch21 = new wxSpinCtrl( sbSizer64->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer61->Add( spnBitmapPitch21, 0, wxALL, 3 );
	
	spnBitmapBank21 = new wxSpinCtrl( sbSizer64->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer61->Add( spnBitmapBank21, 0, wxALL, 3 );
	
	spnBitmapHeading21 = new wxSpinCtrl( sbSizer64->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 58,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 359, 0 );
	fgSizer61->Add( spnBitmapHeading21, 0, wxALL, 3 );
	
	
	bSizer180->Add( fgSizer61, 0, 0, 5 );
	
	wxFlexGridSizer* fgSizer62;
	fgSizer62 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer62->SetFlexibleDirection( wxBOTH );
	fgSizer62->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	chkSBNoCull = new wxCheckBox( sbSizer64->GetStaticBox(), wxID_ANY, wxT("No Cull"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer62->Add( chkSBNoCull, 0, wxALL, 3 );
	
	chkSBNoGlowmaps = new wxCheckBox( sbSizer64->GetStaticBox(), wxID_ANY, wxT("No Glowmaps"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer62->Add( chkSBNoGlowmaps, 0, wxALL, 3 );
	
	chkSBNoLighting = new wxCheckBox( sbSizer64->GetStaticBox(), wxID_ANY, wxT("No Lighting"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer62->Add( chkSBNoLighting, 0, wxALL, 3 );
	
	chkSBNoZBuffer = new wxCheckBox( sbSizer64->GetStaticBox(), wxID_ANY, wxT("No Z-Buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer62->Add( chkSBNoZBuffer, 0, wxALL, 3 );
	
	chkSBForceClamp = new wxCheckBox( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Force Clamp"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer62->Add( chkSBForceClamp, 0, wxALL, 3 );
	
	chkSBTransparent = new wxCheckBox( sbSizer64->GetStaticBox(), wxID_ANY, wxT("Transparent"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer62->Add( chkSBTransparent, 0, wxALL, 3 );
	
	
	bSizer180->Add( fgSizer62, 0, wxEXPAND, 5 );
	
	
	sbSizer64->Add( bSizer180, 1, wxEXPAND, 5 );
	
	
	bSizer173->Add( sbSizer64, 1, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer65;
	sbSizer65 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Misc.") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer63;
	fgSizer63 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer63->AddGrowableCol( 1 );
	fgSizer63->SetFlexibleDirection( wxBOTH );
	fgSizer63->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText186 = new wxStaticText( sbSizer65->GetStaticBox(), wxID_ANY, wxT("Number of Stars:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText186->Wrap( -1 );
	fgSizer63->Add( m_staticText186, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	sldNumStars = new wxSlider( sbSizer65->GetStaticBox(), wxID_ANY, 500, 0, 2000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer63->Add( sldNumStars, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	m_spinCtrl43 = new wxSpinCtrl( sbSizer65->GetStaticBox(), wxID_ANY, wxT("500"), wxDefaultPosition, wxSize( 62,-1 ), wxSP_ARROW_KEYS, 0, 2000, 500 );
	fgSizer63->Add( m_spinCtrl43, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	sbSizer65->Add( fgSizer63, 0, wxEXPAND, 5 );
	
	m_checkBox48 = new wxCheckBox( sbSizer65->GetStaticBox(), wxID_ANY, wxT("Takes place inside Subspace"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer65->Add( m_checkBox48, 0, wxALL, 3 );
	
	wxFlexGridSizer* fgSizer64;
	fgSizer64 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer64->AddGrowableCol( 1 );
	fgSizer64->SetFlexibleDirection( wxBOTH );
	fgSizer64->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnEnvMap = new wxButton( sbSizer65->GetStaticBox(), wxID_ANY, wxT("Environment Map"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer64->Add( btnEnvMap, 0, wxALL|wxEXPAND, 3 );
	
	txtEnvMap = new wxTextCtrl( sbSizer65->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtEnvMap->SetMaxLength( 0 ); 
	fgSizer64->Add( txtEnvMap, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer65->Add( fgSizer64, 0, wxEXPAND, 5 );
	
	
	bSizer173->Add( sbSizer65, 0, wxEXPAND, 3 );
	
	
	bSizer164->Add( bSizer173, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer164 );
	this->Layout();
	bSizer164->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgBackgroundEditor::~dlgBackgroundEditor()
{
}

dlgReinforcementsEditor::dlgReinforcementsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxGridBagSizer* gbSizer18;
	gbSizer18 = new wxGridBagSizer( 0, 0 );
	gbSizer18->SetFlexibleDirection( wxBOTH );
	gbSizer18->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblReinforcements = new wxStaticText( this, wxID_ANY, wxT("Reinforced Wings:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblReinforcements->Wrap( -1 );
	gbSizer18->Add( lblReinforcements, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	lstReinforcements = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_ALWAYS_SB ); 
	gbSizer18->Add( lstReinforcements, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer181;
	bSizer181 = new wxBoxSizer( wxVERTICAL );
	
	btnAdd = new wxButton( this, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer181->Add( btnAdd, 0, wxALL, 3 );
	
	btnDelete = new wxButton( this, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer181->Add( btnDelete, 0, wxALL, 3 );
	
	m_staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer181->Add( m_staticline3, 0, wxEXPAND|wxALL, 3 );
	
	btnOk = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer181->Add( btnOk, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer181->Add( btnCancel, 0, wxALL, 3 );
	
	
	gbSizer18->Add( bSizer181, wxGBPosition( 0, 1 ), wxGBSpan( 2, 1 ), 0, 5 );
	
	wxFlexGridSizer* fgSizer65;
	fgSizer65 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer65->SetFlexibleDirection( wxBOTH );
	fgSizer65->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblUses = new wxStaticText( this, wxID_ANY, wxT("Uses:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblUses->Wrap( -1 );
	fgSizer65->Add( lblUses, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	spnUses = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 64,-1 ), wxSP_ARROW_KEYS, 0, 1000, 0 );
	fgSizer65->Add( spnUses, 0, wxALL, 3 );
	
	
	fgSizer65->Add( 50, 0, 1, wxEXPAND, 5 );
	
	lblDelayAfterArrival = new wxStaticText( this, wxID_ANY, wxT("Delay After Arrival:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDelayAfterArrival->Wrap( -1 );
	fgSizer65->Add( lblDelayAfterArrival, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDelayAfterArrival = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 64,-1 ), wxSP_ARROW_KEYS, 0, 1000, 0 );
	fgSizer65->Add( spnDelayAfterArrival, 0, wxALL, 3 );
	
	
	gbSizer18->Add( fgSizer65, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	
	gbSizer18->AddGrowableCol( 0 );
	gbSizer18->AddGrowableRow( 0 );
	
	this->SetSizer( gbSizer18 );
	this->Layout();
	gbSizer18->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgReinforcementsEditor::~dlgReinforcementsEditor()
{
}

dlgReinforcementsPicker::dlgReinforcementsPicker( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer182;
	bSizer182 = new wxBoxSizer( wxHORIZONTAL );
	
	lstReincforcements = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( 200,200 ), wxLC_LIST );
	bSizer182->Add( lstReincforcements, 0, wxALL|wxEXPAND, 8 );
	
	wxBoxSizer* bSizer183;
	bSizer183 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer183->Add( btnOK, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer183->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer182->Add( bSizer183, 1, wxALIGN_BOTTOM|wxBOTTOM|wxRIGHT, 8 );
	
	
	this->SetSizer( bSizer182 );
	this->Layout();
	bSizer182->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgReinforcementsPicker::~dlgReinforcementsPicker()
{
}

dlgAsteroidFieldEditor::dlgAsteroidFieldEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer184;
	bSizer184 = new wxBoxSizer( wxVERTICAL );
	
	chkAsteroidsEnabled = new wxCheckBox( this, wxID_ANY, wxT("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer184->Add( chkAsteroidsEnabled, 1, wxALL, 5 );
	
	wxBoxSizer* bSizer185;
	bSizer185 = new wxBoxSizer( wxHORIZONTAL );
	
	pFieldProperties = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer66;
	sbSizer66 = new wxStaticBoxSizer( new wxStaticBox( pFieldProperties, wxID_ANY, wxT("Field Properties") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer67;
	sbSizer67 = new wxStaticBoxSizer( new wxStaticBox( sbSizer66->GetStaticBox(), wxID_ANY, wxT("Mode") ), wxHORIZONTAL );
	
	optFieldActive = new wxRadioButton( sbSizer67->GetStaticBox(), wxID_ANY, wxT("Active Field"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optFieldActive->SetValue( true ); 
	sbSizer67->Add( optFieldActive, 1, wxALL|wxEXPAND, 3 );
	
	optFieldPassive = new wxRadioButton( sbSizer67->GetStaticBox(), wxID_ANY, wxT("Passive Field"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer67->Add( optFieldPassive, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer66->Add( sbSizer67, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer68;
	sbSizer68 = new wxStaticBoxSizer( new wxStaticBox( sbSizer66->GetStaticBox(), wxID_ANY, wxT("Content") ), wxVERTICAL );
	
	wxGridBagSizer* gbSizer19;
	gbSizer19 = new wxGridBagSizer( 0, 0 );
	gbSizer19->SetFlexibleDirection( wxBOTH );
	gbSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	optFieldtypeAsteroid = new wxRadioButton( sbSizer68->GetStaticBox(), wxID_ANY, wxT("Asteroid"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	gbSizer19->Add( optFieldtypeAsteroid, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	optFieldTypeShip = new wxRadioButton( sbSizer68->GetStaticBox(), wxID_ANY, wxT("Ship"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer19->Add( optFieldTypeShip, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	chkBrownRocks = new wxCheckBox( sbSizer68->GetStaticBox(), wxID_ANY, wxT("Brown"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer19->Add( chkBrownRocks, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjType1Choices;
	cboObjType1 = new wxChoice( sbSizer68->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjType1Choices, 0 );
	cboObjType1->SetSelection( 0 );
	gbSizer19->Add( cboObjType1, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	chkBlueRocks = new wxCheckBox( sbSizer68->GetStaticBox(), wxID_ANY, wxT("Blue"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer19->Add( chkBlueRocks, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjType2Choices;
	cboObjType2 = new wxChoice( sbSizer68->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjType2Choices, 0 );
	cboObjType2->SetSelection( 0 );
	gbSizer19->Add( cboObjType2, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	chkOrangeRocks = new wxCheckBox( sbSizer68->GetStaticBox(), wxID_ANY, wxT("Orange"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer19->Add( chkOrangeRocks, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboObjType3Choices;
	cboObjType3 = new wxChoice( sbSizer68->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboObjType3Choices, 0 );
	cboObjType3->SetSelection( 0 );
	gbSizer19->Add( cboObjType3, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	m_staticline1 = new wxStaticLine( sbSizer68->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	gbSizer19->Add( m_staticline1, wxGBPosition( 4, 0 ), wxGBSpan( 1, 2 ), wxEXPAND|wxALL, 5 );
	
	lblNumberObjects = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("Number:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblNumberObjects->Wrap( -1 );
	gbSizer19->Add( lblNumberObjects, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnObjects = new wxSpinCtrl( sbSizer68->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer19->Add( spnObjects, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	m_staticText68 = new wxStaticText( sbSizer68->GetStaticBox(), wxID_ANY, wxT("Avg. Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText68->Wrap( -1 );
	gbSizer19->Add( m_staticText68, wxGBPosition( 6, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	numAvgSpeed = new wxTextCtrl( sbSizer68->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numAvgSpeed->SetMaxLength( 0 ); 
	gbSizer19->Add( numAvgSpeed, wxGBPosition( 6, 1 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	
	sbSizer68->Add( gbSizer19, 1, wxEXPAND, 5 );
	
	
	sbSizer66->Add( sbSizer68, 0, wxALL|wxEXPAND, 3 );
	
	
	pFieldProperties->SetSizer( sbSizer66 );
	pFieldProperties->Layout();
	sbSizer66->Fit( pFieldProperties );
	bSizer185->Add( pFieldProperties, 1, wxEXPAND|wxALL, 3 );
	
	pBoundingBoxes = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer186;
	bSizer186 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer69;
	sbSizer69 = new wxStaticBoxSizer( new wxStaticBox( pBoundingBoxes, wxID_ANY, wxT("Outer Box") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer66;
	fgSizer66 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer66->SetFlexibleDirection( wxBOTH );
	fgSizer66->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer66->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblOuterMinimum = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Minimum:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterMinimum->Wrap( -1 );
	fgSizer66->Add( lblOuterMinimum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblOuterMaximum = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Maximum:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterMaximum->Wrap( -1 );
	fgSizer66->Add( lblOuterMaximum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblOuterX = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterX->Wrap( -1 );
	fgSizer66->Add( lblOuterX, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtOuterMinimumX = new wxTextCtrl( sbSizer69->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtOuterMinimumX->SetMaxLength( 0 ); 
	fgSizer66->Add( txtOuterMinimumX, 0, wxALL, 3 );
	
	txtOuterMaximumX = new wxTextCtrl( sbSizer69->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtOuterMaximumX->SetMaxLength( 0 ); 
	fgSizer66->Add( txtOuterMaximumX, 0, wxALL, 3 );
	
	lblOuterY = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterY->Wrap( -1 );
	fgSizer66->Add( lblOuterY, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtOuterMinimumY = new wxTextCtrl( sbSizer69->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtOuterMinimumY->SetMaxLength( 0 ); 
	fgSizer66->Add( txtOuterMinimumY, 0, wxALL, 3 );
	
	txtOuterMaximumY = new wxTextCtrl( sbSizer69->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtOuterMaximumY->SetMaxLength( 0 ); 
	fgSizer66->Add( txtOuterMaximumY, 0, wxALL, 3 );
	
	lblOuterZ = new wxStaticText( sbSizer69->GetStaticBox(), wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOuterZ->Wrap( -1 );
	fgSizer66->Add( lblOuterZ, 0, wxALIGN_CENTER|wxALL, 3 );
	
	txtOuterMinimumZ = new wxTextCtrl( sbSizer69->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtOuterMinimumZ->SetMaxLength( 0 ); 
	fgSizer66->Add( txtOuterMinimumZ, 0, wxALL, 3 );
	
	txtOuterMaximumZ = new wxTextCtrl( sbSizer69->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtOuterMaximumZ->SetMaxLength( 0 ); 
	fgSizer66->Add( txtOuterMaximumZ, 0, wxALL, 3 );
	
	
	sbSizer69->Add( fgSizer66, 0, wxEXPAND, 3 );
	
	
	bSizer186->Add( sbSizer69, 0, wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer70;
	sbSizer70 = new wxStaticBoxSizer( new wxStaticBox( pBoundingBoxes, wxID_ANY, wxT("Inner Box") ), wxVERTICAL );
	
	chkInnerBoxEnable = new wxCheckBox( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer70->Add( chkInnerBoxEnable, 0, wxALL, 3 );
	
	wxFlexGridSizer* fgSizer67;
	fgSizer67 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer67->SetFlexibleDirection( wxBOTH );
	fgSizer67->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer67->Add( 0, 0, 1, wxEXPAND, 3 );
	
	lblInnerMinimum = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Minimum:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	lblInnerMinimum->Wrap( -1 );
	fgSizer67->Add( lblInnerMinimum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblInnerMaximum = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Maximum:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	lblInnerMaximum->Wrap( -1 );
	fgSizer67->Add( lblInnerMaximum, 0, wxALIGN_CENTER|wxALL, 3 );
	
	lblInnerX = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("X:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblInnerX->Wrap( -1 );
	fgSizer67->Add( lblInnerX, 0, wxALIGN_CENTER|wxALL, 3 );
	
	numInnerBoxMinX = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMinX->SetMaxLength( 0 ); 
	fgSizer67->Add( numInnerBoxMinX, 0, wxALL, 3 );
	
	numInnerBoxMaxX = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMaxX->SetMaxLength( 0 ); 
	fgSizer67->Add( numInnerBoxMaxX, 0, wxALL, 3 );
	
	lblInnerY = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblInnerY->Wrap( -1 );
	fgSizer67->Add( lblInnerY, 0, wxALIGN_CENTER|wxALL, 3 );
	
	numInnerBoxMinY = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMinY->SetMaxLength( 0 ); 
	fgSizer67->Add( numInnerBoxMinY, 0, wxALL, 3 );
	
	numInnerBoxMaxY = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMaxY->SetMaxLength( 0 ); 
	fgSizer67->Add( numInnerBoxMaxY, 0, wxALL, 3 );
	
	lblInnerZ = new wxStaticText( sbSizer70->GetStaticBox(), wxID_ANY, wxT("Z:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblInnerZ->Wrap( -1 );
	fgSizer67->Add( lblInnerZ, 0, wxALIGN_CENTER|wxALL, 3 );
	
	numInnerBoxMinZ = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMinZ->SetMaxLength( 0 ); 
	fgSizer67->Add( numInnerBoxMinZ, 0, wxALL, 3 );
	
	numInnerBoxMaxZ = new wxTextCtrl( sbSizer70->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	numInnerBoxMaxZ->SetMaxLength( 0 ); 
	fgSizer67->Add( numInnerBoxMaxZ, 0, wxALL, 3 );
	
	
	sbSizer70->Add( fgSizer67, 0, wxEXPAND, 3 );
	
	
	bSizer186->Add( sbSizer70, 0, wxEXPAND, 3 );
	
	
	pBoundingBoxes->SetSizer( bSizer186 );
	pBoundingBoxes->Layout();
	bSizer186->Fit( pBoundingBoxes );
	bSizer185->Add( pBoundingBoxes, 1, wxEXPAND|wxALL, 3 );
	
	
	bSizer184->Add( bSizer185, 0, wxEXPAND, 5 );
	
	m_sdbSizer9 = new wxStdDialogButtonSizer();
	m_sdbSizer9OK = new wxButton( this, wxID_OK );
	m_sdbSizer9->AddButton( m_sdbSizer9OK );
	m_sdbSizer9Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer9->AddButton( m_sdbSizer9Cancel );
	m_sdbSizer9->Realize();
	
	bSizer184->Add( m_sdbSizer9, 1, wxALIGN_RIGHT|wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer184 );
	this->Layout();
	bSizer184->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgAsteroidFieldEditor::~dlgAsteroidFieldEditor()
{
}

dlgMissionSpecsEditor::dlgMissionSpecsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer187;
	bSizer187 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer68;
	fgSizer68 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer68->AddGrowableCol( 0 );
	fgSizer68->AddGrowableCol( 1 );
	fgSizer68->AddGrowableCol( 2 );
	fgSizer68->SetFlexibleDirection( wxBOTH );
	fgSizer68->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer188;
	bSizer188 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer71;
	sbSizer71 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Meta Info") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer69;
	fgSizer69 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer69->AddGrowableCol( 1 );
	fgSizer69->AddGrowableRow( 0 );
	fgSizer69->SetFlexibleDirection( wxBOTH );
	fgSizer69->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblTitle = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTitle->Wrap( 0 );
	fgSizer69->Add( lblTitle, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3 );
	
	txtTitle = new wxTextCtrl( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Untitled"), wxDefaultPosition, wxDefaultSize, 0 );
	txtTitle->SetMaxLength( 0 ); 
	fgSizer69->Add( txtTitle, 1, wxALL|wxEXPAND, 3 );
	
	lblDesigner = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Designer:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDesigner->Wrap( 0 );
	fgSizer69->Add( lblDesigner, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3 );
	
	txtDesigner = new wxTextCtrl( sbSizer71->GetStaticBox(), wxID_ANY, wxT("wxFRED"), wxDefaultPosition, wxDefaultSize, 0 );
	txtDesigner->SetMaxLength( 0 ); 
	fgSizer69->Add( txtDesigner, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	lblCreated = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Created:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCreated->Wrap( -1 );
	fgSizer69->Add( lblCreated, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtCreated = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("xx/xx/xx at 00:00 AM"), wxDefaultPosition, wxDefaultSize, 0 );
	txtCreated->Wrap( -1 );
	fgSizer69->Add( txtCreated, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblModified = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("Last Modified:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblModified->Wrap( -1 );
	fgSizer69->Add( lblModified, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtModified = new wxStaticText( sbSizer71->GetStaticBox(), wxID_ANY, wxT("xx/xx/xx at 00:00 AM"), wxDefaultPosition, wxDefaultSize, 0 );
	txtModified->Wrap( -1 );
	fgSizer69->Add( txtModified, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer71->Add( fgSizer69, 0, wxALL|wxEXPAND, 0 );
	
	
	bSizer188->Add( sbSizer71, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer72;
	sbSizer72 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mission Type") ), wxVERTICAL );
	
	wxBoxSizer* bSizer189;
	bSizer189 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer190;
	bSizer190 = new wxBoxSizer( wxVERTICAL );
	
	optSinglePlayer = new wxRadioButton( sbSizer72->GetStaticBox(), wxID_ANY, wxT("Single Player"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optSinglePlayer->SetValue( true ); 
	bSizer190->Add( optSinglePlayer, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND|wxRIGHT, 2 );
	
	optMultiPlayer = new wxRadioButton( sbSizer72->GetStaticBox(), wxID_ANY, wxT("Multi Player"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer190->Add( optMultiPlayer, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	optTraining = new wxRadioButton( sbSizer72->GetStaticBox(), wxID_ANY, wxT("Training"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer190->Add( optTraining, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	
	bSizer189->Add( bSizer190, 1, wxEXPAND, 5 );
	
	pnlMultiplayer = new wxPanel( sbSizer72->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlMultiplayer->Enable( false );
	
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );
	
	optCooperative = new wxRadioButton( pnlMultiplayer, wxID_ANY, wxT("Cooperative"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optCooperative->SetValue( true ); 
	optCooperative->Enable( false );
	
	bSizer191->Add( optCooperative, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	optTeamVsTeam = new wxRadioButton( pnlMultiplayer, wxID_ANY, wxT("Team Vs. Team"), wxDefaultPosition, wxDefaultSize, 0 );
	optTeamVsTeam->Enable( false );
	
	bSizer191->Add( optTeamVsTeam, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	optDogfight = new wxRadioButton( pnlMultiplayer, wxID_ANY, wxT("Dogfight"), wxDefaultPosition, wxDefaultSize, 0 );
	optDogfight->Enable( false );
	
	bSizer191->Add( optDogfight, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL|wxEXPAND, 2 );
	
	
	pnlMultiplayer->SetSizer( bSizer191 );
	pnlMultiplayer->Layout();
	bSizer191->Fit( pnlMultiplayer );
	bSizer189->Add( pnlMultiplayer, 1, wxEXPAND|wxALL, 5 );
	
	
	sbSizer72->Add( bSizer189, 1, wxEXPAND, 0 );
	
	
	bSizer188->Add( sbSizer72, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer73;
	sbSizer73 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Multiplayer") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer70;
	fgSizer70 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer70->AddGrowableCol( 1 );
	fgSizer70->SetFlexibleDirection( wxBOTH );
	fgSizer70->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText75 = new wxStaticText( sbSizer73->GetStaticBox(), wxID_ANY, wxT("Max Respawns:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText75->Wrap( -1 );
	fgSizer70->Add( m_staticText75, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxLEFT, 3 );
	
	spnMaxRespawns = new wxSpinCtrl( sbSizer73->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 0 );
	spnMaxRespawns->Enable( false );
	
	fgSizer70->Add( spnMaxRespawns, 0, wxALL, 3 );
	
	m_staticText76 = new wxStaticText( sbSizer73->GetStaticBox(), wxID_ANY, wxT("Max Respawn Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText76->Wrap( -1 );
	fgSizer70->Add( m_staticText76, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxLEFT, 3 );
	
	spnMaxRespawnDelay = new wxSpinCtrl( sbSizer73->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1, 999, 0 );
	spnMaxRespawnDelay->Enable( false );
	
	fgSizer70->Add( spnMaxRespawnDelay, 0, wxALL, 3 );
	
	
	sbSizer73->Add( fgSizer70, 1, wxSHAPED, 0 );
	
	
	bSizer188->Add( sbSizer73, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer74;
	sbSizer74 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Squadron Reassignment") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer71;
	fgSizer71 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer71->AddGrowableCol( 1 );
	fgSizer71->SetFlexibleDirection( wxBOTH );
	fgSizer71->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText74 = new wxStaticText( sbSizer74->GetStaticBox(), wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText74->Wrap( -1 );
	fgSizer71->Add( m_staticText74, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 3 );
	
	txtSquadronName = new wxTextCtrl( sbSizer74->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSquadronName->SetMaxLength( 0 ); 
	fgSizer71->Add( txtSquadronName, 0, wxALL|wxEXPAND, 3 );
	
	btnSquadronLogo = new wxButton( sbSizer74->GetStaticBox(), wxID_ANY, wxT("Logo"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	fgSizer71->Add( btnSquadronLogo, 1, wxALIGN_LEFT|wxALL|wxEXPAND, 3 );
	
	txtSquadronLogo = new wxTextCtrl( sbSizer74->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtSquadronLogo->SetMaxLength( 0 ); 
	fgSizer71->Add( txtSquadronLogo, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer74->Add( fgSizer71, 1, wxEXPAND, 0 );
	
	
	bSizer188->Add( sbSizer74, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	
	fgSizer68->Add( bSizer188, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer192;
	bSizer192 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer75;
	sbSizer75 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Support Ships") ), wxVERTICAL );
	
	wxBoxSizer* bSizer193;
	bSizer193 = new wxBoxSizer( wxVERTICAL );
	
	chkDisallowSupportShips = new wxCheckBox( sbSizer75->GetStaticBox(), wxID_ANY, wxT("Disallow Support Ships"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer193->Add( chkDisallowSupportShips, 0, wxALL, 3 );
	
	chkSupportShipsRepairHull = new wxCheckBox( sbSizer75->GetStaticBox(), wxID_ANY, wxT("Support Ships repair hull"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer193->Add( chkSupportShipsRepairHull, 0, wxALL, 3 );
	
	
	sbSizer75->Add( bSizer193, 0, 0, 5 );
	
	pnlRepairHull = new wxPanel( sbSizer75->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlRepairHull->Enable( false );
	
	wxFlexGridSizer* fgSizer72;
	fgSizer72 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer72->AddGrowableCol( 2 );
	fgSizer72->SetFlexibleDirection( wxBOTH );
	fgSizer72->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer72->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblRepairCeiling = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("Repair Ceiling:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRepairCeiling->Wrap( -1 );
	lblRepairCeiling->Enable( false );
	
	fgSizer72->Add( lblRepairCeiling, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	fgSizer72->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblHullRepairCeiling = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("Hull:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHullRepairCeiling->Wrap( -1 );
	lblHullRepairCeiling->Enable( false );
	
	fgSizer72->Add( lblHullRepairCeiling, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALIGN_RIGHT|wxLEFT, 4 );
	
	spnHullRepairCeiling = new wxSpinCtrl( pnlRepairHull, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 96,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	spnHullRepairCeiling->Enable( false );
	
	fgSizer72->Add( spnHullRepairCeiling, 0, wxALL, 3 );
	
	lblHullPercent = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("%"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHullPercent->Wrap( -1 );
	lblHullPercent->Enable( false );
	
	fgSizer72->Add( lblHullPercent, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
	
	lblSubsystemRepairCeiling = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("Subsystem:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSubsystemRepairCeiling->Wrap( -1 );
	lblSubsystemRepairCeiling->Enable( false );
	
	fgSizer72->Add( lblSubsystemRepairCeiling, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxLEFT, 4 );
	
	spnSubsystemRepairCeiling = new wxSpinCtrl( pnlRepairHull, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 96,-1 ), wxSP_ARROW_KEYS, 0, 10, 0 );
	spnSubsystemRepairCeiling->Enable( false );
	
	fgSizer72->Add( spnSubsystemRepairCeiling, 0, wxALL, 3 );
	
	lblSubstemPercent = new wxStaticText( pnlRepairHull, wxID_ANY, wxT("%"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSubstemPercent->Wrap( -1 );
	lblSubstemPercent->Enable( false );
	
	fgSizer72->Add( lblSubstemPercent, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxALL, 0 );
	
	
	pnlRepairHull->SetSizer( fgSizer72 );
	pnlRepairHull->Layout();
	fgSizer72->Fit( pnlRepairHull );
	sbSizer75->Add( pnlRepairHull, 1, wxEXPAND|wxALL, 5 );
	
	
	bSizer192->Add( sbSizer75, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer76;
	sbSizer76 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Ship Trails") ), wxVERTICAL );
	
	wxBoxSizer* bSizer194;
	bSizer194 = new wxBoxSizer( wxVERTICAL );
	
	chkToggleNebula = new wxCheckBox( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Toggle (off in nebula; on elsewhere)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer194->Add( chkToggleNebula, 0, wxALL, 3 );
	
	wxBoxSizer* bSizer195;
	bSizer195 = new wxBoxSizer( wxHORIZONTAL );
	
	chkMinimumTrailSpeed = new wxCheckBox( sbSizer76->GetStaticBox(), wxID_ANY, wxT("Minimum Speed to display"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer195->Add( chkMinimumTrailSpeed, 0, wxALIGN_CENTER|wxALL, 3 );
	
	spnMinimumTrailSpeed = new wxSpinCtrl( sbSizer76->GetStaticBox(), wxID_ANY, wxT("45"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 999, 45 );
	spnMinimumTrailSpeed->Enable( false );
	
	bSizer195->Add( spnMinimumTrailSpeed, 0, wxALL, 3 );
	
	
	bSizer194->Add( bSizer195, 1, wxEXPAND, 5 );
	
	
	sbSizer76->Add( bSizer194, 0, wxALIGN_CENTER_HORIZONTAL, 0 );
	
	
	bSizer192->Add( sbSizer76, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer77;
	sbSizer77 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Built-in Command Messages") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer73;
	fgSizer73 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer73->AddGrowableCol( 1 );
	fgSizer73->SetFlexibleDirection( wxBOTH );
	fgSizer73->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText71 = new wxStaticText( sbSizer77->GetStaticBox(), wxID_ANY, wxT("Sender:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText71->Wrap( -1 );
	fgSizer73->Add( m_staticText71, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	cboMessageSender = new wxComboBox( sbSizer77->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer73->Add( cboMessageSender, 1, wxALL|wxEXPAND, 3 );
	
	m_staticText72 = new wxStaticText( sbSizer77->GetStaticBox(), wxID_ANY, wxT("Persona:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText72->Wrap( -1 );
	fgSizer73->Add( m_staticText72, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 0 );
	
	wxArrayString cboPersonaChoices;
	cboPersona = new wxChoice( sbSizer77->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboPersonaChoices, 0 );
	cboPersona->SetSelection( 0 );
	fgSizer73->Add( cboPersona, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer77->Add( fgSizer73, 0, wxEXPAND, 5 );
	
	
	bSizer192->Add( sbSizer77, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer78;
	sbSizer78 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Music and Sound") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer74;
	fgSizer74 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer74->AddGrowableCol( 1 );
	fgSizer74->SetFlexibleDirection( wxBOTH );
	fgSizer74->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText73 = new wxStaticText( sbSizer78->GetStaticBox(), wxID_ANY, wxT("Default:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText73->Wrap( -1 );
	fgSizer74->Add( m_staticText73, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxArrayString cboMusicChoices;
	cboMusic = new wxChoice( sbSizer78->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboMusicChoices, 0 );
	cboMusic->SetSelection( 0 );
	fgSizer74->Add( cboMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText741 = new wxStaticText( sbSizer78->GetStaticBox(), wxID_ANY, wxT("If Music pack is present:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText741->Wrap( 70 );
	fgSizer74->Add( m_staticText741, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	cboMusicPackPresent = new wxComboBox( sbSizer78->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer74->Add( cboMusicPackPresent, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	
	sbSizer78->Add( fgSizer74, 0, wxEXPAND, 5 );
	
	btnSoundEnvironment = new wxButton( sbSizer78->GetStaticBox(), wxID_ANY, wxT("Sound Environment"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer78->Add( btnSoundEnvironment, 0, wxALL|wxEXPAND, 3 );
	
	
	bSizer192->Add( sbSizer78, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer68->Add( bSizer192, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer196;
	bSizer196 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer79;
	sbSizer79 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Flags") ), wxVERTICAL );
	
	wxString m_checkList4Choices[] = { wxT("All Teams at War"), wxT("Red Alert Mission"), wxT("Scramble Mission"), wxT("Disallow Promotion/Badges"), wxT("Disable Built-In Messages"), wxT("Disable Built-In Command Messages"), wxT("No Traitor"), wxT("All Ships Beam-Freed by Default"), wxT("Allow Daisy-Chained Docking"), wxT("No Briefing"), wxT("Toggle Debriefing (On/Off)"), wxT("Use Autopilot Cinematics"), wxT("Deactivate Hardcoded Autopilot"), wxT("Player Starts Under AI Control (NO MULTI)"), wxT("2D Mission") };
	int m_checkList4NChoices = sizeof( m_checkList4Choices ) / sizeof( wxString );
	m_checkList4 = new wxCheckListBox( sbSizer79->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,140 ), m_checkList4NChoices, m_checkList4Choices, wxLB_ALWAYS_SB|wxLB_HSCROLL );
	sbSizer79->Add( m_checkList4, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer196->Add( sbSizer79, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer80;
	sbSizer80 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("AI Options") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer75;
	fgSizer75 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer75->AddGrowableCol( 1 );
	fgSizer75->SetFlexibleDirection( wxBOTH );
	fgSizer75->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText771 = new wxStaticText( sbSizer80->GetStaticBox(), wxID_ANY, wxT("AI Profile:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText771->Wrap( -1 );
	fgSizer75->Add( m_staticText771, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3 );
	
	wxArrayString cboAIProfileChoices;
	cboAIProfile = new wxChoice( sbSizer80->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboAIProfileChoices, 0 );
	cboAIProfile->SetSelection( 0 );
	fgSizer75->Add( cboAIProfile, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	
	sbSizer80->Add( fgSizer75, 1, wxEXPAND, 5 );
	
	
	bSizer196->Add( sbSizer80, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer81;
	sbSizer81 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Loading Screen") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer76;
	fgSizer76 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer76->AddGrowableCol( 1 );
	fgSizer76->SetFlexibleDirection( wxBOTH );
	fgSizer76->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	btnLoadingScreen640x480 = new wxButton( sbSizer81->GetStaticBox(), wxID_ANY, wxT("640x480"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	fgSizer76->Add( btnLoadingScreen640x480, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	txtLoadingScreen640x480 = new wxTextCtrl( sbSizer81->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtLoadingScreen640x480->SetMaxLength( 0 ); 
	fgSizer76->Add( txtLoadingScreen640x480, 0, wxALL|wxEXPAND, 3 );
	
	btnLoadingScreen1024x768 = new wxButton( sbSizer81->GetStaticBox(), wxID_ANY, wxT("1024x768"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	fgSizer76->Add( btnLoadingScreen1024x768, 1, wxALIGN_CENTER|wxALL|wxEXPAND, 3 );
	
	txtLoadingScreen1024x768 = new wxTextCtrl( sbSizer81->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtLoadingScreen1024x768->SetMaxLength( 0 ); 
	fgSizer76->Add( txtLoadingScreen1024x768, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer81->Add( fgSizer76, 1, wxEXPAND, 0 );
	
	
	bSizer196->Add( sbSizer81, 0, wxALL|wxEXPAND, 3 );
	
	btnCustomWingNames = new wxButton( this, wxID_ANY, wxT("Custom Wing Names"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer196->Add( btnCustomWingNames, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer197;
	bSizer197 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText159 = new wxStaticText( this, wxID_ANY, wxT("Player Entry Delay:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText159->Wrap( -1 );
	bSizer197->Add( m_staticText159, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinCtrl18 = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 30, 0 );
	bSizer197->Add( m_spinCtrl18, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer196->Add( bSizer197, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer68->Add( bSizer196, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer187->Add( fgSizer68, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer198;
	bSizer198 = new wxBoxSizer( wxVERTICAL );
	
	lblMissionDescription = new wxStaticText( this, wxID_ANY, wxT("Mission Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMissionDescription->Wrap( -1 );
	bSizer198->Add( lblMissionDescription, 0, wxALIGN_BOTTOM|wxLEFT, 9 );
	
	txtMissionDescription = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtMissionDescription->SetMaxLength( 0 ); 
	bSizer198->Add( txtMissionDescription, 1, wxALL|wxEXPAND, 3 );
	
	lblDesignerNotes = new wxStaticText( this, wxID_ANY, wxT("Designer Notes:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDesignerNotes->Wrap( -1 );
	bSizer198->Add( lblDesignerNotes, 0, wxLEFT, 9 );
	
	txtDesignerNotes = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtDesignerNotes->SetMaxLength( 0 ); 
	bSizer198->Add( txtDesignerNotes, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer187->Add( bSizer198, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer187 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgMissionSpecsEditor::~dlgMissionSpecsEditor()
{
}

dlgSoundEnvironment::dlgSoundEnvironment( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer199;
	bSizer199 = new wxBoxSizer( wxHORIZONTAL );
	
	wxGridBagSizer* gbSizer20;
	gbSizer20 = new wxGridBagSizer( 0, 0 );
	gbSizer20->SetFlexibleDirection( wxBOTH );
	gbSizer20->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblEnvironment = new wxStaticText( this, wxID_ANY, wxT("Environment:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblEnvironment->Wrap( -1 );
	gbSizer20->Add( lblEnvironment, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboEnvironment = new wxComboBox( this, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	gbSizer20->Add( cboEnvironment, wxGBPosition( 0, 1 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 3 );
	
	lblVolume = new wxStaticText( this, wxID_ANY, wxT("Volume:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVolume->Wrap( -1 );
	gbSizer20->Add( lblVolume, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnVolume = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer20->Add( spnVolume, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblDamping = new wxStaticText( this, wxID_ANY, wxT("Damping:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDamping->Wrap( -1 );
	gbSizer20->Add( lblDamping, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDamping = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer20->Add( spnDamping, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblDecayTime = new wxStaticText( this, wxID_ANY, wxT("Decay Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDecayTime->Wrap( -1 );
	gbSizer20->Add( lblDecayTime, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDecayTime = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gbSizer20->Add( spnDecayTime, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	
	lblDecaySeconds = new wxStaticText( this, wxID_ANY, wxT("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDecaySeconds->Wrap( -1 );
	gbSizer20->Add( lblDecaySeconds, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer199->Add( gbSizer20, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer200;
	bSizer200 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer201;
	bSizer201 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer201->Add( btnOK, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer201->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer200->Add( bSizer201, 1, wxALIGN_RIGHT, 5 );
	
	wxStaticBoxSizer* sbSizer82;
	sbSizer82 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Preview") ), wxHORIZONTAL );
	
	m_bpButton7 = new wxBitmapButton( sbSizer82->GetStaticBox(), wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 24,24 ), wxBU_AUTODRAW );
	sbSizer82->Add( m_bpButton7, 0, wxALIGN_CENTER|wxALL, 3 );
	
	m_filePicker2 = new wxFilePickerCtrl( sbSizer82->GetStaticBox(), wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxSize( -1,24 ), wxFLP_FILE_MUST_EXIST|wxFLP_OPEN );
	sbSizer82->Add( m_filePicker2, 0, wxALIGN_CENTER|wxALL, 3 );
	
	
	bSizer200->Add( sbSizer82, 1, wxALIGN_RIGHT|wxLEFT|wxTOP, 5 );
	
	
	bSizer199->Add( bSizer200, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer199 );
	this->Layout();
	bSizer199->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgSoundEnvironment::~dlgSoundEnvironment()
{
}

frmBriefingEditor::frmBriefingEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menubar8 = new wxMenuBar( 0 );
	mnuSelectTeam = new wxMenu();
	wxMenuItem* mnuTeam1;
	mnuTeam1 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 1") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuTeam1 );
	
	wxMenuItem* mnuTeam2;
	mnuTeam2 = new wxMenuItem( mnuSelectTeam, wxID_ANY, wxString( wxT("Team 2") ) , wxEmptyString, wxITEM_NORMAL );
	mnuSelectTeam->Append( mnuTeam2 );
	
	m_menubar8->Append( mnuSelectTeam, wxT("Select Team") ); 
	
	mnuOptions = new wxMenu();
	wxMenuItem* mnuBalanceTeams;
	mnuBalanceTeams = new wxMenuItem( mnuOptions, wxID_ANY, wxString( wxT("Balance Teams") ) , wxEmptyString, wxITEM_NORMAL );
	mnuOptions->Append( mnuBalanceTeams );
	
	m_menubar8->Append( mnuOptions, wxT("Options") ); 
	
	this->SetMenuBar( m_menubar8 );
	
	wxBoxSizer* bSizer202;
	bSizer202 = new wxBoxSizer( wxVERTICAL );
	
	m_panel13 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer203;
	bSizer203 = new wxBoxSizer( wxVERTICAL );
	
	wxGridBagSizer* gbSizer21;
	gbSizer21 = new wxGridBagSizer( 0, 0 );
	gbSizer21->SetFlexibleDirection( wxBOTH );
	gbSizer21->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer204;
	bSizer204 = new wxBoxSizer( wxVERTICAL );
	
	lblStage = new wxStaticText( m_panel13, wxID_ANY, wxT("No stages"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStage->Wrap( -1 );
	bSizer204->Add( lblStage, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer205;
	bSizer205 = new wxBoxSizer( wxHORIZONTAL );
	
	lblCameraTransisitonTime = new wxStaticText( m_panel13, wxID_ANY, wxT("Camera Transition Time:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCameraTransisitonTime->Wrap( -1 );
	bSizer205->Add( lblCameraTransisitonTime, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinCtrl53 = new wxSpinCtrl( m_panel13, wxID_ANY, wxT("505"), wxDefaultPosition, wxSize( 70,-1 ), wxSP_ARROW_KEYS, 0, 999999, 505 );
	bSizer205->Add( m_spinCtrl53, 0, wxALL, 3 );
	
	m_staticText192 = new wxStaticText( m_panel13, wxID_ANY, wxT("ms"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText192->Wrap( -1 );
	bSizer205->Add( m_staticText192, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer204->Add( bSizer205, 0, wxEXPAND, 5 );
	
	chkCutToNextStage = new wxCheckBox( m_panel13, wxID_ANY, wxT("Cut to Next Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer204->Add( chkCutToNextStage, 0, wxALL, 3 );
	
	chkCutToPreviousStage = new wxCheckBox( m_panel13, wxID_ANY, wxT("Cut to Previous Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer204->Add( chkCutToPreviousStage, 0, wxALL, 3 );
	
	
	gbSizer21->Add( bSizer204, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	lblText = new wxStaticText( m_panel13, wxID_ANY, wxT("Briefing Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblText->Wrap( -1 );
	gbSizer21->Add( lblText, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_BOTTOM|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_textCtrl75 = new wxTextCtrl( m_panel13, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,60 ), wxTE_MULTILINE );
	m_textCtrl75->SetMaxLength( 0 ); 
	gbSizer21->Add( m_textCtrl75, wxGBPosition( 2, 0 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer206;
	bSizer206 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer207;
	bSizer207 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer208;
	bSizer208 = new wxBoxSizer( wxHORIZONTAL );
	
	btnPreviousStage = new wxButton( m_panel13, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizer208->Add( btnPreviousStage, 0, wxALL|wxEXPAND, 2 );
	
	btnNextStage = new wxButton( m_panel13, wxID_ANY, wxT("Next"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	bSizer208->Add( btnNextStage, 0, wxALL|wxEXPAND, 2 );
	
	
	bSizer207->Add( bSizer208, 1, wxEXPAND, 3 );
	
	btnAddStage = new wxButton( m_panel13, wxID_ANY, wxT("Add Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer207->Add( btnAddStage, 1, wxALL|wxEXPAND, 2 );
	
	btnInsertStage = new wxButton( m_panel13, wxID_ANY, wxT("Insert Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer207->Add( btnInsertStage, 1, wxALL|wxEXPAND, 2 );
	
	btnDeleteStage = new wxButton( m_panel13, wxID_ANY, wxT("Delete Stage"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer207->Add( btnDeleteStage, 1, wxALL|wxEXPAND, 2 );
	
	
	bSizer206->Add( bSizer207, 1, 0, 5 );
	
	wxBoxSizer* bSizer209;
	bSizer209 = new wxBoxSizer( wxVERTICAL );
	
	btnSaveView = new wxButton( m_panel13, wxID_ANY, wxT("Save View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer209->Add( btnSaveView, 1, wxALL|wxEXPAND, 2 );
	
	btnGoToView = new wxButton( m_panel13, wxID_ANY, wxT("Go To View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer209->Add( btnGoToView, 1, wxALL|wxEXPAND, 2 );
	
	btnCopyView = new wxButton( m_panel13, wxID_ANY, wxT("Copy View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer209->Add( btnCopyView, 1, wxALL|wxEXPAND, 2 );
	
	btnPasteView = new wxButton( m_panel13, wxID_ANY, wxT("Paste View"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer209->Add( btnPasteView, 1, wxALL|wxEXPAND, 2 );
	
	
	bSizer206->Add( bSizer209, 1, wxEXPAND, 5 );
	
	
	gbSizer21->Add( bSizer206, wxGBPosition( 0, 1 ), wxGBSpan( 2, 1 ), wxALIGN_RIGHT, 3 );
	
	
	gbSizer21->AddGrowableCol( 0 );
	gbSizer21->AddGrowableCol( 1 );
	
	bSizer203->Add( gbSizer21, 0, wxEXPAND|wxRIGHT, 3 );
	
	wxBoxSizer* bSizer210;
	bSizer210 = new wxBoxSizer( wxHORIZONTAL );
	
	lblVoiceFile = new wxStaticText( m_panel13, wxID_ANY, wxT("Voice File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVoiceFile->Wrap( -1 );
	bSizer210->Add( lblVoiceFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_filePicker1 = new wxFilePickerCtrl( m_panel13, wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	bSizer210->Add( m_filePicker1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayVoice = new wxBitmapButton( m_panel13, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	bSizer210->Add( btnPlayVoice, 0, wxALL, 3 );
	
	
	bSizer203->Add( bSizer210, 0, 0, 5 );
	
	wxStaticBoxSizer* sbSizer83;
	sbSizer83 = new wxStaticBoxSizer( new wxStaticBox( m_panel13, wxID_ANY, wxT("Briefing Music") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer77;
	fgSizer77 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer77->SetFlexibleDirection( wxBOTH );
	fgSizer77->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblDefaultMusic = new wxStaticText( sbSizer83->GetStaticBox(), wxID_ANY, wxT("Default:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDefaultMusic->Wrap( -1 );
	fgSizer77->Add( lblDefaultMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString m_choice42Choices[] = { wxT("None") };
	int m_choice42NChoices = sizeof( m_choice42Choices ) / sizeof( wxString );
	m_choice42 = new wxChoice( sbSizer83->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 120,-1 ), m_choice42NChoices, m_choice42Choices, 0 );
	m_choice42->SetSelection( 0 );
	fgSizer77->Add( m_choice42, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayMusic = new wxBitmapButton( sbSizer83->GetStaticBox(), wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	fgSizer77->Add( btnPlayMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText196 = new wxStaticText( sbSizer83->GetStaticBox(), wxID_ANY, wxT("If music pack is present:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText196->Wrap( 70 );
	fgSizer77->Add( m_staticText196, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxString m_choice43Choices[] = { wxT("None") };
	int m_choice43NChoices = sizeof( m_choice43Choices ) / sizeof( wxString );
	m_choice43 = new wxChoice( sbSizer83->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( 120,-1 ), m_choice43NChoices, m_choice43Choices, 0 );
	m_choice43->SetSelection( 0 );
	fgSizer77->Add( m_choice43, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayMusicFromPack = new wxBitmapButton( sbSizer83->GetStaticBox(), wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	fgSizer77->Add( btnPlayMusicFromPack, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer83->Add( fgSizer77, 1, wxEXPAND, 5 );
	
	
	bSizer203->Add( sbSizer83, 0, 0, 5 );
	
	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxVERTICAL );
	
	lblUsageFormula = new wxStaticText( m_panel13, wxID_ANY, wxT("Usage Formula:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblUsageFormula->Wrap( -1 );
	bSizer211->Add( lblUsageFormula, 0, wxALL, 5 );
	
	m_treeCtrl9 = new wxTreeCtrl( m_panel13, wxID_ANY, wxDefaultPosition, wxSize( -1,60 ), wxTR_DEFAULT_STYLE );
	bSizer211->Add( m_treeCtrl9, 0, wxEXPAND, 3 );
	
	chkDrawLines = new wxCheckBox( m_panel13, wxID_ANY, wxT("Draw Lines Between Marked Icons"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer211->Add( chkDrawLines, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer203->Add( bSizer211, 0, wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	wxStaticBoxSizer* sbSizer84;
	sbSizer84 = new wxStaticBoxSizer( new wxStaticBox( m_panel13, wxID_ANY, wxT("Selected Icon Info") ), wxHORIZONTAL );
	
	wxGridBagSizer* gbSizer22;
	gbSizer22 = new wxGridBagSizer( 0, 0 );
	gbSizer22->SetFlexibleDirection( wxBOTH );
	gbSizer22->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblIconLabel = new wxStaticText( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Label:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconLabel->Wrap( -1 );
	gbSizer22->Add( lblIconLabel, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtIconLabel = new wxTextCtrl( sbSizer84->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	txtIconLabel->SetMaxLength( 0 ); 
	gbSizer22->Add( txtIconLabel, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblIconImage = new wxStaticText( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Icon Image:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconImage->Wrap( -1 );
	gbSizer22->Add( lblIconImage, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboIconImage = new wxComboBox( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 ); 
	gbSizer22->Add( cboIconImage, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblShipType = new wxStaticText( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Ship Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShipType->Wrap( -1 );
	gbSizer22->Add( lblShipType, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	cboShipType = new wxComboBox( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 ); 
	gbSizer22->Add( cboShipType, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblIconTeam = new wxStaticText( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Team:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconTeam->Wrap( -1 );
	gbSizer22->Add( lblIconTeam, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_comboBox13 = new wxComboBox( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, 0 ); 
	gbSizer22->Add( m_comboBox13, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblIconId = new wxStaticText( sbSizer84->GetStaticBox(), wxID_ANY, wxT("ID:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconId->Wrap( -1 );
	gbSizer22->Add( lblIconId, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	txtIconID = new wxTextCtrl( sbSizer84->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0 );
	txtIconID->SetMaxLength( 0 ); 
	gbSizer22->Add( txtIconID, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer212;
	bSizer212 = new wxBoxSizer( wxVERTICAL );
	
	chkHighlightIcon = new wxCheckBox( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Highlight"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer212->Add( chkHighlightIcon, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	chkChangeLocally = new wxCheckBox( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Change Locally"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer212->Add( chkChangeLocally, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	chkFlipIconLR = new wxCheckBox( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Flip Icon (L/R)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer212->Add( chkFlipIconLR, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	gbSizer22->Add( bSizer212, wxGBPosition( 1, 3 ), wxGBSpan( 3, 1 ), wxEXPAND, 5 );
	
	btnMakeIcon = new wxButton( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Make Icon"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer22->Add( btnMakeIcon, wxGBPosition( 0, 4 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	btnDeleteIcon = new wxButton( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Delete Icon"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer22->Add( btnDeleteIcon, wxGBPosition( 1, 4 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	btnPropagate = new wxButton( sbSizer84->GetStaticBox(), wxID_ANY, wxT("Propagate"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer22->Add( btnPropagate, wxGBPosition( 2, 4 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	
	sbSizer84->Add( gbSizer22, 0, wxEXPAND, 3 );
	
	
	bSizer203->Add( sbSizer84, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer213;
	bSizer213 = new wxBoxSizer( wxVERTICAL );
	
	lblIconText = new wxStaticText( m_panel13, wxID_ANY, wxT("Icon Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIconText->Wrap( -1 );
	bSizer213->Add( lblIconText, 0, wxALL, 3 );
	
	txtIconText = new wxTextCtrl( m_panel13, wxID_ANY, wxT("1\n2\n3"), wxDefaultPosition, wxSize( -1,60 ), wxTE_MULTILINE|wxTE_WORDWRAP );
	txtIconText->SetMaxLength( 0 ); 
	bSizer213->Add( txtIconText, 0, wxBOTTOM|wxEXPAND, 4 );
	
	
	bSizer203->Add( bSizer213, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 3 );
	
	
	m_panel13->SetSizer( bSizer203 );
	m_panel13->Layout();
	bSizer203->Fit( m_panel13 );
	bSizer202->Add( m_panel13, 1, wxEXPAND | wxALL, 5 );
	
	
	this->SetSizer( bSizer202 );
	this->Layout();
	bSizer202->Fit( this );
	
	this->Centre( wxBOTH );
}

frmBriefingEditor::~frmBriefingEditor()
{
}

frmDebriefingEditor::frmDebriefingEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer214;
	bSizer214 = new wxBoxSizer( wxVERTICAL );
	
	pnlMain = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer215;
	bSizer215 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer216;
	bSizer216 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer217;
	bSizer217 = new wxBoxSizer( wxHORIZONTAL );
	
	txtStages = new wxStaticText( pnlMain, wxID_ANY, wxT("No stages"), wxDefaultPosition, wxDefaultSize, 0 );
	txtStages->Wrap( -1 );
	bSizer217->Add( txtStages, 2, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	btnPrev = new wxButton( pnlMain, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer217->Add( btnPrev, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnNext = new wxButton( pnlMain, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer217->Add( btnNext, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer216->Add( bSizer217, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer218;
	bSizer218 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAdd = new wxButton( pnlMain, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer218->Add( btnAdd, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnInsert = new wxButton( pnlMain, wxID_ANY, wxT("Insert"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer218->Add( btnInsert, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnDelete = new wxButton( pnlMain, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer218->Add( btnDelete, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer216->Add( bSizer218, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	lblUsageFormula = new wxStaticText( pnlMain, wxID_ANY, wxT("Usage Formula:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblUsageFormula->Wrap( -1 );
	bSizer216->Add( lblUsageFormula, 0, wxALIGN_LEFT|wxALL|wxEXPAND|wxTOP, 3 );
	
	treeUsageFormula = new wxTreeCtrl( pnlMain, wxID_ANY, wxDefaultPosition, wxSize( 250,200 ), wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_MULTIPLE|wxSUNKEN_BORDER );
	bSizer216->Add( treeUsageFormula, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer215->Add( bSizer216, 0, wxALIGN_TOP|wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer219;
	bSizer219 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer220;
	bSizer220 = new wxBoxSizer( wxHORIZONTAL );
	
	lblVoiceWaveFile = new wxStaticText( pnlMain, wxID_ANY, wxT("Voice Wave File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVoiceWaveFile->Wrap( -1 );
	bSizer220->Add( lblVoiceWaveFile, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	txtVoiceWaveFile = new wxTextCtrl( pnlMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtVoiceWaveFile->SetMaxLength( 0 ); 
	bSizer220->Add( txtVoiceWaveFile, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnBrowse = new wxButton( pnlMain, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer220->Add( btnBrowse, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnPlayVoice = new wxBitmapButton( pnlMain, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	bSizer220->Add( btnPlayVoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer219->Add( bSizer220, 0, wxEXPAND|wxALL, 0 );
	
	lblStageText = new wxStaticText( pnlMain, wxID_ANY, wxT("Stage Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStageText->Wrap( -1 );
	bSizer219->Add( lblStageText, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 3 );
	
	txtStageText = new wxTextCtrl( pnlMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 320,100 ), wxTE_MULTILINE );
	txtStageText->SetMaxLength( 0 ); 
	bSizer219->Add( txtStageText, 0, wxALL|wxEXPAND, 3 );
	
	lblRecommendationText = new wxStaticText( pnlMain, wxID_ANY, wxT("Recommendation Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRecommendationText->Wrap( -1 );
	bSizer219->Add( lblRecommendationText, 0, wxALIGN_LEFT|wxALL|wxEXPAND, 3 );
	
	txtRecommendationText = new wxTextCtrl( pnlMain, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 320,100 ), wxTE_MULTILINE );
	txtRecommendationText->SetMaxLength( 0 ); 
	bSizer219->Add( txtRecommendationText, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer215->Add( bSizer219, 1, wxALIGN_TOP|wxALL|wxEXPAND, 3 );
	
	
	pnlMain->SetSizer( bSizer215 );
	pnlMain->Layout();
	bSizer215->Fit( pnlMain );
	bSizer214->Add( pnlMain, 0, wxEXPAND, 0 );
	
	pnlMusic = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer221;
	bSizer221 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer85;
	sbSizer85 = new wxStaticBoxSizer( new wxStaticBox( pnlMusic, wxID_ANY, wxT("Debriefing Music") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer78;
	fgSizer78 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer78->SetFlexibleDirection( wxBOTH );
	fgSizer78->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblMusicSuccess = new wxStaticText( sbSizer85->GetStaticBox(), wxID_ANY, wxT("Success Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMusicSuccess->Wrap( -1 );
	fgSizer78->Add( lblMusicSuccess, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cbMusicSuccessChoices;
	cbMusicSuccess = new wxChoice( sbSizer85->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cbMusicSuccessChoices, 0 );
	cbMusicSuccess->SetSelection( 0 );
	fgSizer78->Add( cbMusicSuccess, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	btnPlaySuccess = new wxBitmapButton( sbSizer85->GetStaticBox(), wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer78->Add( btnPlaySuccess, 0, wxALL, 3 );
	
	m_staticText210 = new wxStaticText( sbSizer85->GetStaticBox(), wxID_ANY, wxT("Nuetral Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText210->Wrap( -1 );
	fgSizer78->Add( m_staticText210, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString m_choice45Choices;
	m_choice45 = new wxChoice( sbSizer85->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice45Choices, 0 );
	m_choice45->SetSelection( 0 );
	fgSizer78->Add( m_choice45, 0, wxALL, 5 );
	
	btnPlayNuetral = new wxBitmapButton( sbSizer85->GetStaticBox(), wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer78->Add( btnPlayNuetral, 0, wxALL, 3 );
	
	m_staticText211 = new wxStaticText( sbSizer85->GetStaticBox(), wxID_ANY, wxT("Failure Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText211->Wrap( -1 );
	fgSizer78->Add( m_staticText211, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString m_choice46Choices;
	m_choice46 = new wxChoice( sbSizer85->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice46Choices, 0 );
	m_choice46->SetSelection( 0 );
	fgSizer78->Add( m_choice46, 0, wxALL, 5 );
	
	btnPlayFailure = new wxBitmapButton( sbSizer85->GetStaticBox(), wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer78->Add( btnPlayFailure, 0, wxALL, 3 );
	
	
	sbSizer85->Add( fgSizer78, 1, wxEXPAND, 5 );
	
	
	bSizer221->Add( sbSizer85, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	pnlMusic->SetSizer( bSizer221 );
	pnlMusic->Layout();
	bSizer221->Fit( pnlMusic );
	bSizer214->Add( pnlMusic, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer214 );
	this->Layout();
	bSizer214->Fit( this );
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
	
	wxBoxSizer* bSizer222;
	bSizer222 = new wxBoxSizer( wxVERTICAL );
	
	m_panel5 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer223;
	bSizer223 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer224;
	bSizer224 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer225;
	bSizer225 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer226;
	bSizer226 = new wxBoxSizer( wxHORIZONTAL );
	
	txtNumCBStages = new wxStaticText( m_panel5, wxID_ANY, wxT("No stages"), wxDefaultPosition, wxDefaultSize, 0 );
	txtNumCBStages->Wrap( -1 );
	bSizer226->Add( txtNumCBStages, 2, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	btnPrev = new wxButton( m_panel5, wxID_ANY, wxT("Prev"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer226->Add( btnPrev, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnNext = new wxButton( m_panel5, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	bSizer226->Add( btnNext, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer225->Add( bSizer226, 0, wxEXPAND, 3 );
	
	wxBoxSizer* bSizer227;
	bSizer227 = new wxBoxSizer( wxHORIZONTAL );
	
	btnAdd = new wxButton( m_panel5, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer227->Add( btnAdd, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnInsert = new wxButton( m_panel5, wxID_ANY, wxT("Insert"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer227->Add( btnInsert, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxLEFT|wxRIGHT, 3 );
	
	btnDelete = new wxButton( m_panel5, wxID_ANY, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer227->Add( btnDelete, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer225->Add( bSizer227, 0, wxALIGN_CENTER_HORIZONTAL, 3 );
	
	
	bSizer224->Add( bSizer225, 0, wxALIGN_TOP, 3 );
	
	
	bSizer224->Add( 5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer228;
	bSizer228 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( m_panel5, wxID_ANY, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer228->Add( btnOK, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	btnCancel = new wxButton( m_panel5, wxID_ANY, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer228->Add( btnCancel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer224->Add( bSizer228, 1, wxEXPAND, 1 );
	
	
	bSizer223->Add( bSizer224, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 3 );
	
	wxID_STATIC1 = new wxStaticText( m_panel5, wxID_ANY, wxT("Stage Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	wxID_STATIC1->Wrap( -1 );
	bSizer223->Add( wxID_STATIC1, 0, wxALIGN_LEFT|wxALL, 3 );
	
	txtStageText = new wxTextCtrl( m_panel5, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtStageText->SetMaxLength( 0 ); 
	bSizer223->Add( txtStageText, 2, wxALL|wxEXPAND, 3 );
	
	wxFlexGridSizer* fgSizer79;
	fgSizer79 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer79->AddGrowableCol( 1 );
	fgSizer79->SetFlexibleDirection( wxBOTH );
	fgSizer79->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblAniFile = new wxStaticText( m_panel5, wxID_ANY, wxT("Ani File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAniFile->Wrap( -1 );
	fgSizer79->Add( lblAniFile, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	fpAniFile = new wxFilePickerCtrl( m_panel5, wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer79->Add( fpAniFile, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer79->Add( 0, 0, 1, wxEXPAND, 5 );
	
	lblVoiceWaveFile = new wxStaticText( m_panel5, wxID_ANY, wxT("Voice Wave File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVoiceWaveFile->Wrap( -1 );
	fgSizer79->Add( lblVoiceWaveFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpVoiceWave = new wxFilePickerCtrl( m_panel5, wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer79->Add( fpVoiceWave, 0, wxALL|wxEXPAND, 3 );
	
	btnVoicePlay = new wxBitmapButton( m_panel5, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW|wxBU_EXACTFIT );
	fgSizer79->Add( btnVoicePlay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer223->Add( fgSizer79, 1, wxEXPAND, 3 );
	
	
	m_panel5->SetSizer( bSizer223 );
	m_panel5->Layout();
	bSizer223->Fit( m_panel5 );
	bSizer222->Add( m_panel5, 1, wxEXPAND | wxALL, 5 );
	
	
	this->SetSizer( bSizer222 );
	this->Layout();
	bSizer222->Fit( this );
	
	this->Centre( wxBOTH );
}

frmCommandBriefingEditor::~frmCommandBriefingEditor()
{
}

dlgFictionViewer::dlgFictionViewer( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer229;
	bSizer229 = new wxBoxSizer( wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer80;
	fgSizer80 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer80->SetFlexibleDirection( wxBOTH );
	fgSizer80->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblStoryFile = new wxStaticText( this, wxID_ANY, wxT("Story File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblStoryFile->Wrap( -1 );
	fgSizer80->Add( lblStoryFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpStoryFile = new wxFilePickerCtrl( this, wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a story file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer80->Add( fpStoryFile, 0, wxALL, 3 );
	
	lblFontFile = new wxStaticText( this, wxID_ANY, wxT("Font File:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblFontFile->Wrap( -1 );
	fgSizer80->Add( lblFontFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	fpFontFile = new wxFilePickerCtrl( this, wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a font file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer80->Add( fpFontFile, 0, wxALL, 3 );
	
	lblMusic = new wxStaticText( this, wxID_ANY, wxT("Music:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMusic->Wrap( -1 );
	fgSizer80->Add( lblMusic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxBoxSizer* bSizer230;
	bSizer230 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString cbMusicChoices;
	cbMusic = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,23 ), cbMusicChoices, 0 );
	cbMusic->SetSelection( 0 );
	bSizer230->Add( cbMusic, 1, wxALL|wxEXPAND, 3 );
	
	btnPlayMusic = new wxBitmapButton( this, wxID_ANY, wxBitmap( play_xpm ), wxDefaultPosition, wxSize( 23,23 ), wxBU_AUTODRAW );
	bSizer230->Add( btnPlayMusic, 0, wxALL, 2 );
	
	
	fgSizer80->Add( bSizer230, 1, wxEXPAND, 5 );
	
	
	bSizer229->Add( fgSizer80, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer231;
	bSizer231 = new wxBoxSizer( wxVERTICAL );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer231->Add( btnOK, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer231->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer229->Add( bSizer231, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer229 );
	this->Layout();
	bSizer229->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgFictionViewer::~dlgFictionViewer()
{
}

dlgShieldSystemEditor::dlgShieldSystemEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer232;
	bSizer232 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer233;
	bSizer233 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer86;
	sbSizer86 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("All ships of type") ), wxVERTICAL );
	
	wxArrayString cboShipTypeChoices;
	cboShipType = new wxChoice( sbSizer86->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboShipTypeChoices, 0 );
	cboShipType->SetSelection( 0 );
	sbSizer86->Add( cboShipType, 0, wxEXPAND|wxALL, 3 );
	
	optShipTypeHasShieldSystem = new wxRadioButton( sbSizer86->GetStaticBox(), wxID_ANY, wxT("Have shield systems"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optShipTypeHasShieldSystem->SetValue( true ); 
	sbSizer86->Add( optShipTypeHasShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	optShipTypeNoShieldSystem = new wxRadioButton( sbSizer86->GetStaticBox(), wxID_ANY, wxT("No shield systems"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer86->Add( optShipTypeNoShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	
	bSizer233->Add( sbSizer86, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer87;
	sbSizer87 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("All ships on team") ), wxVERTICAL );
	
	wxArrayString cboShipTeamChoices;
	cboShipTeam = new wxChoice( sbSizer87->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboShipTeamChoices, 0 );
	cboShipTeam->SetSelection( 0 );
	sbSizer87->Add( cboShipTeam, 0, wxEXPAND|wxALL, 3 );
	
	optShipTeamHasShieldSystem = new wxRadioButton( sbSizer87->GetStaticBox(), wxID_ANY, wxT("Have shield systems"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	optShipTeamHasShieldSystem->SetValue( true ); 
	sbSizer87->Add( optShipTeamHasShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	optShipTeamNoShieldSystem = new wxRadioButton( sbSizer87->GetStaticBox(), wxID_ANY, wxT("No shield systems"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer87->Add( optShipTeamNoShieldSystem, 0, wxALIGN_LEFT|wxALL, 3 );
	
	
	bSizer233->Add( sbSizer87, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer232->Add( bSizer233, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	m_sdbSizer10 = new wxStdDialogButtonSizer();
	m_sdbSizer10OK = new wxButton( this, wxID_OK );
	m_sdbSizer10->AddButton( m_sdbSizer10OK );
	m_sdbSizer10Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer10->AddButton( m_sdbSizer10Cancel );
	m_sdbSizer10->Realize();
	
	bSizer232->Add( m_sdbSizer10, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer232 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgShieldSystemEditor::~dlgShieldSystemEditor()
{
}

dlgSetGlobalShipFlagsEditor::dlgSetGlobalShipFlagsEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer234;
	bSizer234 = new wxBoxSizer( wxVERTICAL );
	
	btnGlobalNoShields = new wxButton( this, wxID_ANY, wxT("Global No-Shields"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer234->Add( btnGlobalNoShields, 0, wxEXPAND|wxALL, 5 );
	
	btnGlobalNoSubspaceDrive = new wxButton( this, wxID_ANY, wxT("Global No-Subspace-Drive"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer234->Add( btnGlobalNoSubspaceDrive, 0, wxEXPAND|wxALL, 5 );
	
	btnGlobalPrimitiveSensors = new wxButton( this, wxID_ANY, wxT("Global Primitive-Sensors"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer234->Add( btnGlobalPrimitiveSensors, 0, wxEXPAND|wxALL, 5 );
	
	btnGlobalAffectedByGravity = new wxButton( this, wxID_ANY, wxT("Global Affected-By-Gravity"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer234->Add( btnGlobalAffectedByGravity, 0, wxEXPAND|wxALL, 5 );
	
	
	this->SetSizer( bSizer234 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgSetGlobalShipFlagsEditor::~dlgSetGlobalShipFlagsEditor()
{
}

dlgVoiceActingManager::dlgVoiceActingManager( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer235;
	bSizer235 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer81;
	fgSizer81 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer81->AddGrowableCol( 0 );
	fgSizer81->AddGrowableCol( 1 );
	fgSizer81->AddGrowableRow( 0 );
	fgSizer81->AddGrowableRow( 1 );
	fgSizer81->SetFlexibleDirection( wxBOTH );
	fgSizer81->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer88;
	sbSizer88 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("File Name Options") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer89;
	sbSizer89 = new wxStaticBoxSizer( new wxStaticBox( sbSizer88->GetStaticBox(), wxID_ANY, wxT("Abbreviations") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer82;
	fgSizer82 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer82->SetFlexibleDirection( wxBOTH );
	fgSizer82->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblCampaign = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Campaign:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaign->Wrap( -1 );
	fgSizer82->Add( lblCampaign, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevCampaign = new wxTextCtrl( sbSizer89->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevCampaign->SetMaxLength( 0 ); 
	fgSizer82->Add( txtAbbrevCampaign, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblMission = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Mission:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMission->Wrap( -1 );
	fgSizer82->Add( lblMission, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevMission = new wxTextCtrl( sbSizer89->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevMission->SetMaxLength( 0 ); 
	fgSizer82->Add( txtAbbrevMission, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblCmdBriefingStage = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Cmd. Briefing Stage:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCmdBriefingStage->Wrap( -1 );
	fgSizer82->Add( lblCmdBriefingStage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevCB = new wxTextCtrl( sbSizer89->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevCB->SetMaxLength( 0 ); 
	fgSizer82->Add( txtAbbrevCB, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblBriefingStage = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Briefing Stage:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBriefingStage->Wrap( -1 );
	fgSizer82->Add( lblBriefingStage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevBriefing = new wxTextCtrl( sbSizer89->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevBriefing->SetMaxLength( 0 ); 
	fgSizer82->Add( txtAbbrevBriefing, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblDebriefingStage = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Debriefing Stage:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDebriefingStage->Wrap( -1 );
	fgSizer82->Add( lblDebriefingStage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevDebrief = new wxTextCtrl( sbSizer89->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevDebrief->SetMaxLength( 0 ); 
	fgSizer82->Add( txtAbbrevDebrief, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	lblMessage = new wxStaticText( sbSizer89->GetStaticBox(), wxID_ANY, wxT("Message:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMessage->Wrap( -1 );
	fgSizer82->Add( lblMessage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtAbbrevMessage = new wxTextCtrl( sbSizer89->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtAbbrevMessage->SetMaxLength( 0 ); 
	fgSizer82->Add( txtAbbrevMessage, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );
	
	
	sbSizer89->Add( fgSizer82, 1, wxEXPAND, 5 );
	
	
	sbSizer88->Add( sbSizer89, 0, wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer90;
	sbSizer90 = new wxStaticBoxSizer( new wxStaticBox( sbSizer88->GetStaticBox(), wxID_ANY, wxT("Other") ), wxHORIZONTAL );
	
	lblAudioFileExtension = new wxStaticText( sbSizer90->GetStaticBox(), wxID_ANY, wxT("File Extension:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAudioFileExtension->Wrap( -1 );
	sbSizer90->Add( lblAudioFileExtension, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString cboVAFileExtChoices;
	cboVAFileExt = new wxChoice( sbSizer90->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, cboVAFileExtChoices, 0 );
	cboVAFileExt->SetSelection( 0 );
	sbSizer90->Add( cboVAFileExt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer88->Add( sbSizer90, 0, wxALL, 3 );
	
	wxBoxSizer* bSizer236;
	bSizer236 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText125 = new wxStaticText( sbSizer88->GetStaticBox(), wxID_ANY, wxT("Example"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText125->Wrap( -1 );
	bSizer236->Add( m_staticText125, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	txtExampleFileName = new wxTextCtrl( sbSizer88->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtExampleFileName->SetMaxLength( 0 ); 
	txtExampleFileName->Enable( false );
	
	bSizer236->Add( txtExampleFileName, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	sbSizer88->Add( bSizer236, 0, wxALL|wxEXPAND, 3 );
	
	chkVANoReplaceExistingFiles = new wxCheckBox( sbSizer88->GetStaticBox(), wxID_ANY, wxT("Don't replace existing files"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer88->Add( chkVANoReplaceExistingFiles, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer88->Add( 0, 21, 1, wxEXPAND, 5 );
	
	btnGenerateFileNames = new wxButton( sbSizer88->GetStaticBox(), wxID_ANY, wxT("Generate File Names"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer88->Add( btnGenerateFileNames, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer81->Add( sbSizer88, 1, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer91;
	sbSizer91 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Script options") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer92;
	sbSizer92 = new wxStaticBoxSizer( new wxStaticBox( sbSizer91->GetStaticBox(), wxID_ANY, wxT("Script Entry Format") ), wxHORIZONTAL );
	
	txtScriptEntryFormat = new wxTextCtrl( sbSizer92->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	txtScriptEntryFormat->SetMaxLength( 0 ); 
	sbSizer92->Add( txtScriptEntryFormat, 1, wxALL|wxEXPAND, 3 );
	
	lblScriptHelp = new wxStaticText( sbSizer92->GetStaticBox(), wxID_ANY, wxT("$filename - Name of the message file\n$message - Text of the message\n$persona - Persona of the sender\n$sender - Name of the sender\n\nNote: $persona and $sender will only appear for the Message section"), wxDefaultPosition, wxDefaultSize, 0 );
	lblScriptHelp->Wrap( 190 );
	sbSizer92->Add( lblScriptHelp, 1, wxALL, 3 );
	
	
	sbSizer91->Add( sbSizer92, 0, wxALL|wxEXPAND, 3 );
	
	wxStaticBoxSizer* sbSizer93;
	sbSizer93 = new wxStaticBoxSizer( new wxStaticBox( sbSizer91->GetStaticBox(), wxID_ANY, wxT("Export...") ), wxVERTICAL );
	
	optEverything = new wxRadioButton( sbSizer93->GetStaticBox(), wxID_ANY, wxT("Everything"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer93->Add( optEverything, 0, wxALL, 3 );
	
	optJustCommandBriefings = new wxRadioButton( sbSizer93->GetStaticBox(), wxID_ANY, wxT("Just Command Briefings"), wxDefaultPosition, wxDefaultSize, 0 );
	optJustCommandBriefings->SetValue( true ); 
	sbSizer93->Add( optJustCommandBriefings, 0, wxALL, 3 );
	
	optJustBriefings = new wxRadioButton( sbSizer93->GetStaticBox(), wxID_ANY, wxT("Just Briefings"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer93->Add( optJustBriefings, 0, wxALL, 3 );
	
	optJustDebriefings = new wxRadioButton( sbSizer93->GetStaticBox(), wxID_ANY, wxT("Just Debriefings"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer93->Add( optJustDebriefings, 0, wxALL, 3 );
	
	optJustMessages = new wxRadioButton( sbSizer93->GetStaticBox(), wxID_ANY, wxT("Just Messages"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer93->Add( optJustMessages, 0, wxALL, 3 );
	
	wxBoxSizer* bSizer237;
	bSizer237 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer237->Add( 21, 0, 1, wxEXPAND, 5 );
	
	chkGroupMessageList = new wxCheckBox( sbSizer93->GetStaticBox(), wxID_ANY, wxT("Group send-message-list messages before others"), wxDefaultPosition, wxDefaultSize, 0 );
	chkGroupMessageList->Enable( false );
	
	bSizer237->Add( chkGroupMessageList, 0, wxALL, 3 );
	
	
	sbSizer93->Add( bSizer237, 1, 0, 5 );
	
	
	sbSizer91->Add( sbSizer93, 0, wxALL|wxEXPAND, 3 );
	
	
	sbSizer91->Add( 0, 0, 1, wxEXPAND, 5 );
	
	btnGenerateScript = new wxButton( sbSizer91->GetStaticBox(), wxID_ANY, wxT("Generate Script"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer91->Add( btnGenerateScript, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer81->Add( sbSizer91, 1, wxALL|wxEXPAND, 3 );
	
	
	bSizer235->Add( fgSizer81, 1, 0, 5 );
	
	
	this->SetSizer( bSizer235 );
	this->Layout();
	bSizer235->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgVoiceActingManager::~dlgVoiceActingManager()
{
}

frmCampaignEditor::frmCampaignEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxColour( 226, 226, 226 ) );
	
	wxBoxSizer* bSizer238;
	bSizer238 = new wxBoxSizer( wxHORIZONTAL );
	
	m_splitter1 = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_splitter1->SetSashGravity( 0 );
	m_splitter1->Connect( wxEVT_IDLE, wxIdleEventHandler( frmCampaignEditor::m_splitter1OnIdle ), NULL, this );
	
	pnlCampaign = new wxScrolledWindow( m_splitter1, wxID_ANY, wxDefaultPosition, wxSize( 400,-1 ), wxHSCROLL|wxSUNKEN_BORDER|wxVSCROLL );
	pnlCampaign->SetScrollRate( 5, 5 );
	wxBoxSizer* bSizer239;
	bSizer239 = new wxBoxSizer( wxVERTICAL );
	
	lblAvailableMissions = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Available missions:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAvailableMissions->Wrap( -1 );
	bSizer239->Add( lblAvailableMissions, 0, wxALIGN_LEFT|wxALL|wxTOP, 3 );
	
	lstAvailableMissions = new wxListCtrl( pnlCampaign, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_LIST|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING );
	bSizer239->Add( lstAvailableMissions, 0, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer240;
	bSizer240 = new wxBoxSizer( wxHORIZONTAL );
	
	lblCampaignName = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Campaign Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaignName->Wrap( -1 );
	bSizer240->Add( lblCampaignName, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	txtCampaignName = new wxTextCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	txtCampaignName->SetMaxLength( 0 ); 
	bSizer240->Add( txtCampaignName, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblCampaignType = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaignType->Wrap( 0 );
	bSizer240->Add( lblCampaignType, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	wxString cbCampaignTypeChoices[] = { wxT("single"), wxT("multi coop"), wxT("multi teams") };
	int cbCampaignTypeNChoices = sizeof( cbCampaignTypeChoices ) / sizeof( wxString );
	cbCampaignType = new wxChoice( pnlCampaign, wxID_ANY, wxDefaultPosition, wxDefaultSize, cbCampaignTypeNChoices, cbCampaignTypeChoices, 0 );
	cbCampaignType->SetSelection( 0 );
	bSizer240->Add( cbCampaignType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	
	bSizer239->Add( bSizer240, 0, wxALL|wxEXPAND, 3 );
	
	lblCampaignDescription = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Campaign Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCampaignDescription->Wrap( -1 );
	bSizer239->Add( lblCampaignDescription, 0, wxALIGN_LEFT|wxALL, 3 );
	
	txtCampaignDescription = new wxTextCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,55 ), wxTE_MULTILINE );
	txtCampaignDescription->SetMaxLength( 0 ); 
	bSizer239->Add( txtCampaignDescription, 0, wxALL|wxEXPAND, 3 );
	
	chkUsesCustomTechDatabase = new wxCheckBox( pnlCampaign, wxID_ANY, wxT("Uses custom tech database"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
	bSizer239->Add( chkUsesCustomTechDatabase, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer94;
	sbSizer94 = new wxStaticBoxSizer( new wxStaticBox( pnlCampaign, wxID_ANY, wxT("Mission options") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer83;
	fgSizer83 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer83->SetFlexibleDirection( wxBOTH );
	fgSizer83->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblBriefingCutscene = new wxStaticText( sbSizer94->GetStaticBox(), wxID_ANY, wxT("Briefing Cutscene:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBriefingCutscene->Wrap( -1 );
	fgSizer83->Add( lblBriefingCutscene, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	fpBriefingCutscene = new wxFilePickerCtrl( sbSizer94->GetStaticBox(), wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer83->Add( fpBriefingCutscene, 0, wxALL|wxEXPAND, 3 );
	
	
	fgSizer83->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer84;
	fgSizer84 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer84->SetFlexibleDirection( wxBOTH );
	fgSizer84->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblMainhallIndex = new wxStaticText( sbSizer94->GetStaticBox(), wxID_ANY, wxT("Mainhall Index:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMainhallIndex->Wrap( -1 );
	fgSizer84->Add( lblMainhallIndex, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	spnMainHallIndex = new wxSpinCtrl( sbSizer94->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 9, 0 );
	fgSizer84->Add( spnMainHallIndex, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	lblDebriefingPersonaIndex = new wxStaticText( sbSizer94->GetStaticBox(), wxID_ANY, wxT("Debriefing Persona Index:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDebriefingPersonaIndex->Wrap( -1 );
	fgSizer84->Add( lblDebriefingPersonaIndex, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	spnDebriefingPersonaIndex = new wxSpinCtrl( sbSizer94->GetStaticBox(), wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 9, 0 );
	fgSizer84->Add( spnDebriefingPersonaIndex, 0, wxALL, 3 );
	
	
	fgSizer83->Add( fgSizer84, 1, wxEXPAND, 5 );
	
	
	sbSizer94->Add( fgSizer83, 0, 0, 5 );
	
	
	bSizer239->Add( sbSizer94, 0, wxALIGN_CENTER|wxALL, 3 );
	
	wxBoxSizer* bSizer241;
	bSizer241 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer242;
	bSizer242 = new wxBoxSizer( wxVERTICAL );
	
	lblBranches = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Branches:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBranches->Wrap( 0 );
	bSizer242->Add( lblBranches, 0, wxALIGN_LEFT|wxALL|wxLEFT|wxRIGHT|wxTOP, 3 );
	
	treeBranches = new wxTreeCtrl( pnlCampaign, wxID_ANY, wxDefaultPosition, wxSize( 200,200 ), wxTR_EDIT_LABELS|wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_SINGLE|wxSUNKEN_BORDER );
	bSizer242->Add( treeBranches, 0, wxALL|wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 3 );
	
	
	bSizer241->Add( bSizer242, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 3 );
	
	wxBoxSizer* bSizer243;
	bSizer243 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer95;
	sbSizer95 = new wxStaticBoxSizer( new wxStaticBox( pnlCampaign, wxID_ANY, wxT("Branch Options") ), wxVERTICAL );
	
	btnMoveUp = new wxButton( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Move Up"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	sbSizer95->Add( btnMoveUp, 0, wxALL, 3 );
	
	btnMoveDown = new wxButton( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Move Down"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	sbSizer95->Add( btnMoveDown, 0, wxALL, 3 );
	
	btnToggleLoop = new wxButton( sbSizer95->GetStaticBox(), wxID_ANY, wxT("Toggle Loop"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	sbSizer95->Add( btnToggleLoop, 0, wxALL, 3 );
	
	
	bSizer243->Add( sbSizer95, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	btnRealignTree = new wxButton( pnlCampaign, wxID_ANY, wxT("Realign Tree"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer243->Add( btnRealignTree, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	btnLoadMission = new wxButton( pnlCampaign, wxID_ANY, wxT("Load Mission"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer243->Add( btnLoadMission, 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT, 3 );
	
	btnClose = new wxButton( pnlCampaign, wxID_ANY, wxT("Close"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	bSizer243->Add( btnClose, 0, wxALIGN_RIGHT|wxALL, 3 );
	
	
	bSizer241->Add( bSizer243, 0, wxALIGN_TOP|wxALL|wxEXPAND, 3 );
	
	
	bSizer239->Add( bSizer241, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	lblDesignerNotes = new wxStaticText( pnlCampaign, wxID_ANY, wxT("Designer Notes:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblDesignerNotes->Wrap( -1 );
	bSizer239->Add( lblDesignerNotes, 0, wxALL, 3 );
	
	txtDesignerNotes = new wxTextCtrl( pnlCampaign, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,50 ), wxTE_MULTILINE|wxTE_READONLY );
	txtDesignerNotes->SetMaxLength( 0 ); 
	bSizer239->Add( txtDesignerNotes, 0, wxEXPAND|wxALL, 3 );
	
	wxStaticBoxSizer* sbSizer96;
	sbSizer96 = new wxStaticBoxSizer( new wxStaticBox( pnlCampaign, wxID_ANY, wxT("Mission Loop Options") ), wxVERTICAL );
	
	lblMissionLoopDiscription = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Discription:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblMissionLoopDiscription->Wrap( -1 );
	sbSizer96->Add( lblMissionLoopDiscription, 0, wxALIGN_LEFT|wxALL, 3 );
	
	txtMissionLoopDescription = new wxTextCtrl( sbSizer96->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,50 ), wxTE_MULTILINE );
	txtMissionLoopDescription->SetMaxLength( 0 ); 
	sbSizer96->Add( txtMissionLoopDescription, 0, wxALL|wxEXPAND, 3 );
	
	wxFlexGridSizer* fgSizer85;
	fgSizer85 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer85->AddGrowableCol( 1 );
	fgSizer85->SetFlexibleDirection( wxBOTH );
	fgSizer85->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	lblLoopBriefAni = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Briefing Animation:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblLoopBriefAni->Wrap( -1 );
	fgSizer85->Add( lblLoopBriefAni, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	fpLoopBriefAni = new wxFilePickerCtrl( sbSizer96->GetStaticBox(), wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select an animation file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer85->Add( fpLoopBriefAni, 1, wxALL|wxEXPAND, 3 );
	
	lblBriefVoice = new wxStaticText( sbSizer96->GetStaticBox(), wxID_ANY, wxT("Briefing Voice:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblBriefVoice->Wrap( -1 );
	fgSizer85->Add( lblBriefVoice, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 3 );
	
	fpLoopBriefVoice = new wxFilePickerCtrl( sbSizer96->GetStaticBox(), wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Selec a voice file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	fgSizer85->Add( fpLoopBriefVoice, 1, wxALL|wxEXPAND, 3 );
	
	
	sbSizer96->Add( fgSizer85, 1, wxEXPAND, 5 );
	
	
	bSizer239->Add( sbSizer96, 1, wxEXPAND, 5 );
	
	
	pnlCampaign->SetSizer( bSizer239 );
	pnlCampaign->Layout();
	pnlCampaignGraph = new wxPanel( m_splitter1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	m_splitter1->SplitVertically( pnlCampaign, pnlCampaignGraph, 0 );
	bSizer238->Add( m_splitter1, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer238 );
	this->Layout();
	m_menubar2 = new wxMenuBar( 0 );
	mnuFile = new wxMenu();
	wxMenuItem* mnuFileNew;
	mnuFileNew = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("New") ) + wxT('\t') + wxT("Ctrl-N"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileNew );
	
	wxMenuItem* mnuFileOpen;
	mnuFileOpen = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Open") ) + wxT('\t') + wxT("Ctrl-O"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileOpen );
	
	wxMenuItem* mnuFileSave;
	mnuFileSave = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Save") ) + wxT('\t') + wxT("Ctrl-S"), wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSave );
	
	wxMenuItem* mnuFileSaveAs;
	mnuFileSaveAs = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Save As...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileSaveAs );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileExit;
	mnuFileExit = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileExit );
	
	m_menubar2->Append( mnuFile, wxT("File") ); 
	
	other = new wxMenu();
	wxMenuItem* errorChecker;
	errorChecker = new wxMenuItem( other, wxID_ANY, wxString( wxT("Error Checker") ) + wxT('\t') + wxT("Alt-H"), wxEmptyString, wxITEM_NORMAL );
	other->Append( errorChecker );
	
	m_menubar2->Append( other, wxT("Other") ); 
	
	initialStatus = new wxMenu();
	wxMenuItem* ships;
	ships = new wxMenuItem( initialStatus, wxID_ANY, wxString( wxT("Ships") ) , wxEmptyString, wxITEM_NORMAL );
	initialStatus->Append( ships );
	
	wxMenuItem* weapons;
	weapons = new wxMenuItem( initialStatus, wxID_ANY, wxString( wxT("Weapons") ) , wxEmptyString, wxITEM_NORMAL );
	initialStatus->Append( weapons );
	
	m_menubar2->Append( initialStatus, wxT("Initial Status") ); 
	
	this->SetMenuBar( m_menubar2 );
	
	
	this->Centre( wxBOTH );
}

frmCampaignEditor::~frmCampaignEditor()
{
}

dlgMissionStats::dlgMissionStats( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer244;
	bSizer244 = new wxBoxSizer( wxVERTICAL );
	
	txtMissionStats = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	txtMissionStats->SetMaxLength( 0 ); 
	txtMissionStats->Enable( false );
	
	bSizer244->Add( txtMissionStats, 1, wxALL|wxEXPAND, 3 );
	
	wxBoxSizer* bSizer245;
	bSizer245 = new wxBoxSizer( wxHORIZONTAL );
	
	btnDumpToFile = new wxButton( this, wxID_ANY, wxT("Dump to File"), wxDefaultPosition, wxSize( -1,30 ), 0 );
	bSizer245->Add( btnDumpToFile, 0, wxALL, 3 );
	
	btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxSize( -1,30 ), 0 );
	bSizer245->Add( btnCancel, 0, wxALL, 3 );
	
	
	bSizer244->Add( bSizer245, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer244 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgMissionStats::~dlgMissionStats()
{
}

dlgAboutBox::dlgAboutBox( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
	
	wxBoxSizer* bSizer246;
	bSizer246 = new wxBoxSizer( wxHORIZONTAL );
	
	bmpLogo = new wxStaticBitmap( this, wxID_ANY, wxBitmap( fred_splash_xpm ), wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
	bmpLogo->SetBackgroundColour( wxColour( 255, 255, 255 ) );
	
	bSizer246->Add( bmpLogo, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer247;
	bSizer247 = new wxBoxSizer( wxVERTICAL );
	
	lblAppTitle = new wxStaticText( this, wxID_ANY, wxT("FRED2_OPEN - FreeSpace Editor, Version 3.6.19"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAppTitle->Wrap( -1 );
	bSizer247->Add( lblAppTitle, 0, wxALL, 3 );
	
	lblCopywrite = new wxStaticText( this, wxID_ANY, wxT("Copyright 1999 Volition, Inc.  All Rights Reserved"), wxDefaultPosition, wxDefaultSize, 0 );
	lblCopywrite->Wrap( -1 );
	bSizer247->Add( lblCopywrite, 0, wxALL, 3 );
	
	
	bSizer247->Add( 0, 0, 1, wxALL|wxEXPAND, 5 );
	
	lblDevelopers = new wxStaticText( this, wxID_ANY, wxT("Bravely maintained by:\n\tGoober5000, taylor, Karajorma and the SCP Team.\n\nPorted to OpenGL by:\n\tRandomTiger, Phreak, and Fry_Day.\n\nPorted to wxWidgets by:\n\tGoober5000, taylor, The E, and z64555."), wxDefaultPosition, wxSize( 300,-1 ), 0 );
	lblDevelopers->Wrap( -1 );
	lblDevelopers->SetMinSize( wxSize( -1,150 ) );
	
	bSizer247->Add( lblDevelopers, 0, wxALL, 3 );
	
	
	bSizer247->Add( 0, 0, 1, wxALL|wxEXPAND, 5 );
	
	lblQuote = new wxStaticText( this, wxID_ANY, wxT("\"Fred2 is the omega of all giant unwieldy pieces of code. It's big, it's horrifying, and it just doesn't care. View it at your own risk\" - Dave Baranec"), wxDefaultPosition, wxDefaultSize, 0 );
	lblQuote->Wrap( 370 );
	bSizer247->Add( lblQuote, 0, wxALL, 3 );
	
	
	bSizer247->Add( 0, 0, 1, wxALL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer86;
	fgSizer86 = new wxFlexGridSizer( 0, 5, 0, 5 );
	fgSizer86->AddGrowableCol( 1 );
	fgSizer86->AddGrowableCol( 2 );
	fgSizer86->AddGrowableCol( 3 );
	fgSizer86->SetFlexibleDirection( wxBOTH );
	fgSizer86->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer86->Add( 0, 0, 1, wxEXPAND, 5 );
	
	btnOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	btnOK->SetDefault(); 
	fgSizer86->Add( btnOK, 0, wxEXPAND, 3 );
	
	btnReportBug = new wxButton( this, wxID_ANY, wxT("Report a Bug"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer86->Add( btnReportBug, 0, wxEXPAND, 3 );
	
	btnVisitForums = new wxButton( this, wxID_ANY, wxT("Visit Forums"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer86->Add( btnVisitForums, 0, wxALIGN_CENTER|wxEXPAND, 3 );
	
	
	fgSizer86->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	bSizer247->Add( fgSizer86, 0, wxEXPAND, 3 );
	
	
	bSizer246->Add( bSizer247, 1, wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer246 );
	this->Layout();
	bSizer246->Fit( this );
	
	this->Centre( wxBOTH );
}

dlgAboutBox::~dlgAboutBox()
{
}

dlgSexpHelp::dlgSexpHelp( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer248;
	bSizer248 = new wxBoxSizer( wxVERTICAL );
	
	pnlSexpHelp = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer249;
	bSizer249 = new wxBoxSizer( wxVERTICAL );
	
	lblArgInfo = new wxStaticText( pnlSexpHelp, wxID_ANY, wxT("Argument Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArgInfo->Wrap( -1 );
	bSizer249->Add( lblArgInfo, 0, wxALL, 3 );
	
	txtArgInfo = new wxTextCtrl( pnlSexpHelp, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	txtArgInfo->SetMaxLength( 0 ); 
	bSizer249->Add( txtArgInfo, 0, wxALL|wxEXPAND, 3 );
	
	lblSexpInfo = new wxStaticText( pnlSexpHelp, wxID_ANY, wxT("Sexp Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSexpInfo->Wrap( -1 );
	bSizer249->Add( lblSexpInfo, 0, wxALL, 3 );
	
	txtSexpInfo = new wxTextCtrl( pnlSexpHelp, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	txtSexpInfo->SetMaxLength( 0 ); 
	bSizer249->Add( txtSexpInfo, 1, wxALL|wxEXPAND, 3 );
	
	
	pnlSexpHelp->SetSizer( bSizer249 );
	pnlSexpHelp->Layout();
	bSizer249->Fit( pnlSexpHelp );
	bSizer248->Add( pnlSexpHelp, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer248 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

dlgSexpHelp::~dlgSexpHelp()
{
}

pnlSexpHelp::pnlSexpHelp( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer250;
	bSizer250 = new wxBoxSizer( wxVERTICAL );
	
	lblArgInfo = new wxStaticText( this, wxID_ANY, wxT("Argument Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblArgInfo->Wrap( -1 );
	bSizer250->Add( lblArgInfo, 0, wxALL, 3 );
	
	txtArgInfo = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	txtArgInfo->SetMaxLength( 0 ); 
	bSizer250->Add( txtArgInfo, 0, wxALL|wxEXPAND, 3 );
	
	lblSexpInfo = new wxStaticText( this, wxID_ANY, wxT("Sexp Info:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSexpInfo->Wrap( -1 );
	bSizer250->Add( lblSexpInfo, 0, wxALL, 3 );
	
	txtSexpInfo = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	txtSexpInfo->SetMaxLength( 0 ); 
	bSizer250->Add( txtSexpInfo, 1, wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer250 );
	this->Layout();
	bSizer250->Fit( this );
}

pnlSexpHelp::~pnlSexpHelp()
{
}
