///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WXFRED_BASE_H__
#define __WXFRED_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/toolbar.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/spinctrl.h>
#include <wx/treectrl.h>
#include <wx/statbox.h>
#include <wx/tglbtn.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/bmpbuttn.h>
#include <wx/checklst.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/slider.h>
#include <wx/filepicker.h>
#include <wx/scrolwin.h>
#include <wx/splitter.h>
#include <wx/statbmp.h>

///////////////////////////////////////////////////////////////////////////

namespace fredBase
{
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmFRED
	///////////////////////////////////////////////////////////////////////////////
	class frmFRED : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			void _wxFB_OnFileNew( wxCommandEvent& event ){ OnFileNew( event ); }
			void _wxFB_OnFileOpen( wxCommandEvent& event ){ OnFileOpen( event ); }
			void _wxFB_OnFileSave( wxCommandEvent& event ){ OnFileSave( event ); }
			void _wxFB_OnFileSaveAs( wxCommandEvent& event ){ OnFileSaveAs( event ); }
			void _wxFB_OnFileRevert( wxCommandEvent& event ){ OnFileRevert( event ); }
			void _wxFB_OnFileSaveFormatFs2Open( wxCommandEvent& event ){ OnFileSaveFormatFs2Open( event ); }
			void _wxFB_OnFileSaveFormatFs2Retail( wxCommandEvent& event ){ OnFileSaveFormatFs2Retail( event ); }
			void _wxFB_OnFileImportFs1Mission( wxCommandEvent& event ){ OnFileImportFs1Mission( event ); }
			void _wxFB_OnFileImportWeaponLoadouts( wxCommandEvent& event ){ OnFileImportWeaponLoadouts( event ); }
			void _wxFB_OnFileRunFreespace( wxCommandEvent& event ){ OnFileRunFreespace( event ); }
			void _wxFB_OnFileRecentFiles( wxCommandEvent& event ){ OnFileRecentFiles( event ); }
			void _wxFB_OnFileExit( wxCommandEvent& event ){ OnFileExit( event ); }
			void _wxFB_OnEditUndo( wxCommandEvent& event ){ OnEditUndo( event ); }
			void _wxFB_OnEditDelete( wxCommandEvent& event ){ OnEditDelete( event ); }
			void _wxFB_OnEditDeleteWing( wxCommandEvent& event ){ OnEditDeleteWing( event ); }
			void _wxFB_OnEditDisableUndo( wxCommandEvent& event ){ OnEditDisableUndo( event ); }
			void _wxFB_OnViewToolbar( wxCommandEvent& event ){ OnViewToolbar( event ); }
			void _wxFB_OnViewStatusBar( wxCommandEvent& event ){ OnViewStatusBar( event ); }
			void _wxFB_OnViewDisplayFilterShowShips( wxCommandEvent& event ){ OnViewDisplayFilterShowShips( event ); }
			void _wxFB_OnViewDisplayFilterShowPlayerStarts( wxCommandEvent& event ){ OnViewDisplayFilterShowPlayerStarts( event ); }
			void _wxFB_OnViewDisplayFilterShowWaypoints( wxCommandEvent& event ){ OnViewDisplayFilterShowWaypoints( event ); }
			void _wxFB_OnViewDisplayFilterShowFriendly( wxCommandEvent& event ){ OnViewDisplayFilterShowFriendly( event ); }
			void _wxFB_OnViewDisplayFilterShowHostile( wxCommandEvent& event ){ OnViewDisplayFilterShowHostile( event ); }
			void _wxFB_OnViewHideMarkedObjects( wxCommandEvent& event ){ OnViewHideMarkedObjects( event ); }
			void _wxFB_OnViewShowHiddenObjects( wxCommandEvent& event ){ OnViewShowHiddenObjects( event ); }
			void _wxFB_OnViewShowShipModels( wxCommandEvent& event ){ OnViewShowShipModels( event ); }
			void _wxFB_OnViewShowOutlines( wxCommandEvent& event ){ OnViewShowOutlines( event ); }
			void _wxFB_OnViewShowShipInfo( wxCommandEvent& event ){ OnViewShowShipInfo( event ); }
			void _wxFB_OnViewShowCoordinates( wxCommandEvent& event ){ OnViewShowCoordinates( event ); }
			void _wxFB_OnViewShowGridPositions( wxCommandEvent& event ){ OnViewShowGridPositions( event ); }
			void _wxFB_OnViewShowDistances( wxCommandEvent& event ){ OnViewShowDistances( event ); }
			void _wxFB_OnViewShowModelPaths( wxCommandEvent& event ){ OnViewShowModelPaths( event ); }
			void _wxFB_OnViewShowModelDockPoints( wxCommandEvent& event ){ OnViewShowModelDockPoints( event ); }
			void _wxFB_OnViewShowGrid( wxCommandEvent& event ){ OnViewShowGrid( event ); }
			void _wxFB_OnViewShowHorizon( wxCommandEvent& event ){ OnViewShowHorizon( event ); }
			void _wxFB_OnViewDoubleFineGridlines( wxCommandEvent& event ){ OnViewDoubleFineGridlines( event ); }
			void _wxFB_OnViewAntiAliasedGridlines( wxCommandEvent& event ){ OnViewAntiAliasedGridlines( event ); }
			void _wxFB_OnViewShow3DCompass( wxCommandEvent& event ){ OnViewShow3DCompass( event ); }
			void _wxFB_OnViewShowBackground( wxCommandEvent& event ){ OnViewShowBackground( event ); }
			void _wxFB_OnViewViewpointCamera( wxCommandEvent& event ){ OnViewViewpointCamera( event ); }
			void _wxFB_OnViewViewpointCurrentShip( wxCommandEvent& event ){ OnViewViewpointCurrentShip( event ); }
			void _wxFB_OnViewSaveCameraPos( wxCommandEvent& event ){ OnViewSaveCameraPos( event ); }
			void _wxFB_OnViewRestoreCameraPos( wxCommandEvent& event ){ OnViewRestoreCameraPos( event ); }
			void _wxFB_OnViewLightingFromSuns( wxCommandEvent& event ){ OnViewLightingFromSuns( event ); }
			void _wxFB_OnSpeedMovement( wxCommandEvent& event ){ OnSpeedMovement( event ); }
			void _wxFB_OnSpeedRotation( wxCommandEvent& event ){ OnSpeedRotation( event ); }
			void _wxFB_OnEditorsShips( wxCommandEvent& event ){ OnEditorsShips( event ); }
			void _wxFB_OnEditorsWings( wxCommandEvent& event ){ OnEditorsWings( event ); }
			void _wxFB_OnEditorsObjects( wxCommandEvent& event ){ OnEditorsObjects( event ); }
			void _wxFB_OnEditorsWaypointPaths( wxCommandEvent& event ){ OnEditorsWaypointPaths( event ); }
			void _wxFB_OnEditorsMissionObjectives( wxCommandEvent& event ){ OnEditorsMissionObjectives( event ); }
			void _wxFB_OnEditorsEvents( wxCommandEvent& event ){ OnEditorsEvents( event ); }
			void _wxFB_OnEditorsTeamLoadout( wxCommandEvent& event ){ OnEditorsTeamLoadout( event ); }
			void _wxFB_OnEditorsBackground( wxCommandEvent& event ){ OnEditorsBackground( event ); }
			void _wxFB_OnEditorsReinforcements( wxCommandEvent& event ){ OnEditorsReinforcements( event ); }
			void _wxFB_OnEditorsAsteroidField( wxCommandEvent& event ){ OnEditorsAsteroidField( event ); }
			void _wxFB_OnEditorsMissionSpecs( wxCommandEvent& event ){ OnEditorsMissionSpecs( event ); }
			void _wxFB_OnEditorsBriefing( wxCommandEvent& event ){ OnEditorsBriefing( event ); }
			void _wxFB_OnEditorsDebriefing( wxCommandEvent& event ){ OnEditorsDebriefing( event ); }
			void _wxFB_OnEditorsCommandBriefing( wxCommandEvent& event ){ OnEditorsCommandBriefing( event ); }
			void _wxFB_OnEditorsFictionViewer( wxCommandEvent& event ){ OnEditorsFictionViewer( event ); }
			void _wxFB_OnEditorsShieldSystem( wxCommandEvent& event ){ OnEditorsShieldSystem( event ); }
			void _wxFB_OnEditorsSetGlobalShipFlags( wxCommandEvent& event ){ OnEditorsSetGlobalShipFlags( event ); }
			void _wxFB_OnEditorsVoiceActingManager( wxCommandEvent& event ){ OnEditorsVoiceActingManager( event ); }
			void _wxFB_OnEditorsCampaign( wxCommandEvent& event ){ OnEditorsCampaign( event ); }
			void _wxFB_OnGroupsGroup( wxCommandEvent& event ){ OnGroupsGroup( event ); }
			void _wxFB_OnGroupsSetGroup( wxCommandEvent& event ){ OnGroupsSetGroup( event ); }
			void _wxFB_OnMiscLevelObject( wxCommandEvent& event ){ OnMiscLevelObject( event ); }
			void _wxFB_OnMiscAlignObject( wxCommandEvent& event ){ OnMiscAlignObject( event ); }
			void _wxFB_OnMiscMarkWing( wxCommandEvent& event ){ OnMiscMarkWing( event ); }
			void _wxFB_OnMiscControlObject( wxCommandEvent& event ){ OnMiscControlObject( event ); }
			void _wxFB_OnMiscNextObject( wxCommandEvent& event ){ OnMiscNextObject( event ); }
			void _wxFB_OnMiscPreviousObject( wxCommandEvent& event ){ OnMiscPreviousObject( event ); }
			void _wxFB_OnMiscAdjustGrid( wxCommandEvent& event ){ OnMiscAdjustGrid( event ); }
			void _wxFB_OnMiscNextSubsystem( wxCommandEvent& event ){ OnMiscNextSubsystem( event ); }
			void _wxFB_OnMiscPrevSubsystem( wxCommandEvent& event ){ OnMiscPrevSubsystem( event ); }
			void _wxFB_OnMiscCancelSubsystem( wxCommandEvent& event ){ OnMiscCancelSubsystem( event ); }
			void _wxFB_OnMiscMissionStatistics( wxCommandEvent& event ){ OnMiscMissionStatistics( event ); }
			void _wxFB_OnMiscErrorChecker( wxCommandEvent& event ){ OnMiscErrorChecker( event ); }
			void _wxFB_OnHelpHelpTopics( wxCommandEvent& event ){ OnHelpHelpTopics( event ); }
			void _wxFB_OnHelpShowSexpHelp( wxCommandEvent& event ){ OnHelpShowSexpHelp( event ); }
			void _wxFB_OnHelpAbout( wxCommandEvent& event ){ OnHelpAbout( event ); }
			void _wxFB_OnSelect( wxCommandEvent& event ){ OnSelect( event ); }
			void _wxFB_OnSelectMove( wxCommandEvent& event ){ OnSelectMove( event ); }
			void _wxFB_OnSelectRotate( wxCommandEvent& event ){ OnSelectRotate( event ); }
			void _wxFB_OnRotateLocally( wxCommandEvent& event ){ OnRotateLocally( event ); }
			void _wxFB_OnConstraintX( wxCommandEvent& event ){ OnConstraintX( event ); }
			void _wxFB_OnConstraintY( wxCommandEvent& event ){ OnConstraintY( event ); }
			void _wxFB_OnConstraintZ( wxCommandEvent& event ){ OnConstraintZ( event ); }
			void _wxFB_OnConstraintXZ( wxCommandEvent& event ){ OnConstraintXZ( event ); }
			void _wxFB_OnConstraintYZ( wxCommandEvent& event ){ OnConstraintYZ( event ); }
			void _wxFB_OnConstraintXY( wxCommandEvent& event ){ OnConstraintXY( event ); }
			void _wxFB_OnSelectionList( wxCommandEvent& event ){ OnSelectionList( event ); }
			void _wxFB_OnSelectionLock( wxCommandEvent& event ){ OnSelectionLock( event ); }
			void _wxFB_OnWingForm( wxCommandEvent& event ){ OnWingForm( event ); }
			void _wxFB_OnWingDisband( wxCommandEvent& event ){ OnWingDisband( event ); }
			void _wxFB_OnZoomSelected( wxCommandEvent& event ){ OnZoomSelected( event ); }
			void _wxFB_OnZoomExtents( wxCommandEvent& event ){ OnZoomExtents( event ); }
			void _wxFB_OnShowDistances( wxCommandEvent& event ){ OnShowDistances( event ); }
			void _wxFB_OnOrbitSelected( wxCommandEvent& event ){ OnOrbitSelected( event ); }
			
		
		protected:
			enum
			{
				ID_frmFRED = 1000,
				ID_mnuFileNew,
				ID_mnuFileOpen,
				ID_mnuFileSave,
				ID_mnuFileSaveAs,
				ID_mnuFileRevert,
				ID_mnuFileSaveFormatFs2Open,
				ID_mnuFileSaveFormatFs2Retail,
				ID_mnuFileImportFs1Mission,
				ID_mnuFileImportWeaponLoadouts,
				ID_mnuFileRunFreespace,
				ID_mnuFileRecentFiles,
				ID_mnuFileExit,
				ID_mnuEditUndo,
				ID_mnuEditDelete,
				ID_mnuEditDeleteWing,
				ID_mnuEditDisableUndo,
				ID_mnuViewToolbar,
				ID_mnuViewStatusBar,
				ID_mnuViewDispayFilterShowShips,
				ID_mnuViewDisplayFilterShowPlayerStarts,
				ID_mnuViewDisplayFilterShowWaypoints,
				ID_mnuViewDisplayFilterShowFriendly,
				ID_mnuViewDisplayFilterShowHostile,
				ID_mnuViewHideMarkedObjects,
				ID_mnuViewShowHiddenObjects,
				ID_mnuViewShowShipModels,
				ID_mnuViewShowOutlines,
				ID_mnuViewShowShipInfo,
				ID_mnuViewShowCoordinates,
				ID_mnuViewShowGridPositions,
				ID_ViewShowDistances,
				ID_mnuViewShowModelPaths,
				ID_mnuViewShowModelDockPoints,
				ID_mnuViewShowGrid,
				ID_mnuViewShowHorizon,
				ID_mnuViewDoubleFineGridlines,
				ID_mnuViewAntiAliasedGridlines,
				ID_mnuViewShow3DCompass,
				ID_mnuViewShowBackground,
				ID_ViewViewpointCamera,
				ID_mnuViewViewpointCurrentShip,
				ID_mnuViewSaveCameraPos,
				ID_mnuViewRestoreCameraPos,
				ID_mnuViewLightingFromSuns,
				ID_mnuSpeedMovementX1,
				ID_mnuSpeedMovementX2,
				ID_mnuSpeedMovementX3,
				ID_mnuSpeedMovementX5,
				ID_mnuSpeedMovementX8,
				ID_mnuSpeedMovementX10,
				ID_mnuSpeedMovementX50,
				ID_mnuSpeedMovementX100,
				ID_mnuSpeedRotationX1,
				ID_mnuSpeedRotationX5,
				ID_mnuSpeedRotationX12,
				ID_mnuSpeedRotationX25,
				ID_mnuSpeedRotationX50,
				ID_mnuEditorsShips,
				ID_mnuEditorsWings,
				ID_mnuEditorsObjects,
				ID_mnuEditorsWaypointPaths,
				ID_mnuEditorsMissionObjectives,
				ID_mnuEditorsEvents,
				ID_mnuEditorsTeamLoadout,
				ID_mnuEditorsBackground,
				ID_mnuEditorsReinforcements,
				ID_mnuEditorsAsteroidField,
				ID_mnuEditorsMissionSpecs,
				ID_mnuEditorsBriefing,
				ID_mnuEditorsDebriefing,
				ID_mnuEditorsCommandBriefing,
				ID_mnuEditorsFictionViewer,
				ID_mnuEditorsShieldSystem,
				ID_mnuEditorsSetGlobalShipFlags,
				ID_mnuEditorsVoiceActingManager,
				ID_mnuEditorsCampaign,
				ID_mnuGroupsGroup1,
				ID_mnuGroupsGroup2,
				ID_mnuGroupsGroup3,
				ID_mnuGroupsGroup4,
				ID_mnuGroupsGroup5,
				ID_mnuGroupsGroup6,
				ID_mnuGroupsGroup7,
				ID_mnuGroupsGroup8,
				ID_mnuGroupsGroup9,
				ID_mnuGroupsSetGroupGroup1,
				ID_mnuGroupsSetGroupGroup2,
				ID_mnuGroupsSetGroupGroup3,
				ID_mnuGroupsSetGroupGroup4,
				ID_mnuGroupsSetGroupGroup5,
				ID_mnuGroupsSetGroupGroup6,
				ID_mnuGroupsSetGroupGroup7,
				ID_mnuGroupsSetGroupGroup8,
				ID_mnuGroupsSetGroupGroup9,
				ID_mnuMiscLevelObject,
				ID_mnuMiscAlignObject,
				ID_mnuMiscMarkWing,
				ID_mnuMiscControlObject,
				ID_mnuMiscNextObject,
				ID_mnuMiscPreviousObject,
				ID_mnuMiscAdjustGrid,
				ID_mnuMiscNextSubsystem,
				ID_mnuMiscPrevSubsystem,
				ID_mnuMiscCancelSubsystem,
				ID_mnuMiscMissionStatistics,
				ID_mnuMiscErrorChecker,
				ID_mnuHelpHelpTopics,
				ID_mnuHelpShowSexpHelp,
				ID_mnuHelpAbout,
				ID_optSelect,
				ID_optSelectMove,
				ID_optSelectRotate,
				ID_chkRotateLocally,
				ID_optConstraintX,
				ID_optConstraintY,
				ID_optConstraintZ,
				ID_optConstraintXZ,
				ID_optConstraintYZ,
				ID_optConstraintXY,
				ID_btnSelectionList,
				ID_chkSelectionLock,
				ID_btnWingForm,
				ID_btnZoomSelected,
				ID_btnZoomExtents,
				ID_chkShowDistances,
				ID_chkOrbitSelected
			};
			
			wxMenuBar* mbrFRED;
			wxMenu* mnuFile;
			wxMenu* mnuFileSaveFormat;
			wxMenu* mnuFileImport;
			wxMenu* mnuEdit;
			wxMenu* mnuView;
			wxMenu* mnuViewDisplayFiter;
			wxMenu* mnuViewViewpoint;
			wxMenu* mnuSpeed;
			wxMenu* mnuSpeedMovement;
			wxMenu* mnuSpeedRotation;
			wxMenu* mnuEditors;
			wxMenu* mnuGroups;
			wxMenu* mnuGroupsSetGroup;
			wxMenu* mnuMisc;
			wxMenu* mnuHelp;
			wxToolBar* tbrFRED;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			virtual void OnFileNew( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileOpen( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileSave( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileSaveAs( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileRevert( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileSaveFormatFs2Open( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileSaveFormatFs2Retail( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileImportFs1Mission( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileImportWeaponLoadouts( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileRunFreespace( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileRecentFiles( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFileExit( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditUndo( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditDelete( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditDeleteWing( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditDisableUndo( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewToolbar( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewStatusBar( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewDisplayFilterShowShips( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewDisplayFilterShowPlayerStarts( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewDisplayFilterShowWaypoints( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewDisplayFilterShowFriendly( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewDisplayFilterShowHostile( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewHideMarkedObjects( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowHiddenObjects( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowShipModels( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowOutlines( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowShipInfo( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowCoordinates( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowGridPositions( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowDistances( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowModelPaths( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowModelDockPoints( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowGrid( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowHorizon( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewDoubleFineGridlines( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewAntiAliasedGridlines( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShow3DCompass( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewShowBackground( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewViewpointCamera( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewViewpointCurrentShip( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewSaveCameraPos( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewRestoreCameraPos( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnViewLightingFromSuns( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSpeedMovement( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSpeedRotation( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsShips( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsWings( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsObjects( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsWaypointPaths( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsMissionObjectives( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsEvents( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsTeamLoadout( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsBackground( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsReinforcements( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsAsteroidField( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsMissionSpecs( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsBriefing( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsDebriefing( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsCommandBriefing( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsFictionViewer( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsShieldSystem( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsSetGlobalShipFlags( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsVoiceActingManager( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditorsCampaign( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnGroupsGroup( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnGroupsSetGroup( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscLevelObject( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscAlignObject( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscMarkWing( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscControlObject( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscNextObject( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscPreviousObject( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscAdjustGrid( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscNextSubsystem( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscPrevSubsystem( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscCancelSubsystem( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscMissionStatistics( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMiscErrorChecker( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnHelpHelpTopics( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnHelpShowSexpHelp( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnHelpAbout( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSelect( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSelectMove( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSelectRotate( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnRotateLocally( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnConstraintX( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnConstraintY( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnConstraintZ( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnConstraintXZ( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnConstraintYZ( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnConstraintXY( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSelectionList( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSelectionLock( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnWingForm( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnWingDisband( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnZoomSelected( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnZoomExtents( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnShowDistances( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOrbitSelected( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmFRED( wxWindow* parent, wxWindowID id = ID_frmFRED, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmFRED();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmShipsEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmShipsEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_frmShipsEditor = 1000
			};
			
			wxMenuBar* mbrShipsEditor;
			wxMenu* selectShip;
			wxStaticText* lblShipName;
			wxTextCtrl* txtShipName;
			wxStaticText* lblWing;
			wxTextCtrl* txtWing;
			wxStaticText* lblShipClass;
			wxChoice* cboShipClass;
			wxStaticText* lblHotkey;
			wxChoice* cboHotkey;
			wxStaticText* lblAIClass;
			wxChoice* cboAIClass;
			wxStaticText* lblPersona;
			wxChoice* cboPersona;
			wxStaticText* lblTeam;
			wxChoice* cboTeam;
			wxStaticText* lblKillScore;
			wxTextCtrl* txtKillscore;
			wxStaticText* lblCargoCargo;
			wxComboBox* cboCargo;
			wxStaticText* lblAssistPercentage;
			wxTextCtrl* txtAssistPercentage;
			wxStaticText* lblAltName;
			wxComboBox* cboAltName;
			wxCheckBox* chkPlayerShip;
			wxStaticText* lblCallsign;
			wxComboBox* cboCallsign;
			wxButton* btnMakePlayerShip;
			wxButton* btnTextureReplacement;
			wxButton* btnAltShipClass;
			wxButton* btnPrevWing;
			wxButton* btnNextWing;
			wxButton* btnDelete;
			wxButton* btnReset;
			wxButton* btnWeapons;
			wxButton* btnPlayerOrders;
			wxButton* btnSpecialExplosion;
			wxButton* btnSpecialHits;
			wxButton* btnMiscOptions;
			wxButton* btnInitialStatus;
			wxButton* btnInitialOrders;
			wxButton* btnTBLInfo;
			wxButton* btnHideCues;
			wxStaticText* lblArrivalLocation;
			wxChoice* cboArrivalLocation;
			wxStaticText* lblArrivalTarget;
			wxChoice* cboArrivalTarget;
			wxStaticText* lblArrivalDistance;
			wxTextCtrl* txtArrivalDistance;
			wxStaticText* lblArrivalDelay;
			wxSpinCtrl* spnArrivalDelay;
			wxStaticText* lblArrivalDelaySeconds;
			wxButton* btnRestrictArrivalPaths;
			wxStaticText* lblArrivalCue;
			wxTreeCtrl* tctArrivalCues;
			wxCheckBox* chkNoArrivalWarp;
			wxStaticText* lblDepatureLocation;
			wxChoice* cboDepartureLocation;
			wxStaticText* lblDepartureTarget;
			wxChoice* cboDepartureTarget;
			wxStaticText* lblDepartureDelay;
			wxSpinCtrl* spnArrivalDelay1;
			wxStaticText* m_staticText1711;
			wxButton* btnRestrictDeparturePaths;
			wxStaticText* lblDepartureCue;
			wxTreeCtrl* tctDepartureCues;
			wxCheckBox* chkNoDepartureWarp;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmShipsEditor( wxWindow* parent, wxWindowID id = ID_frmShipsEditor, const wxString& title = wxT("Edit Ship"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_NO_TASKBAR|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
			
			~frmShipsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmWingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmWingEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_frmWingEditor = 1000,
				ID_btnInitialOrders,
				ID_btnHideCues
			};
			
			wxPanel* pnlProperties;
			wxStaticText* lblWingName;
			wxTextCtrl* txtWingName;
			wxStaticText* lblLeader;
			wxChoice* cboWingLeader;
			wxStaticText* lblWaveNumber;
			wxSpinCtrl* spnWaveNumber;
			wxStaticText* lblWaveThreshold;
			wxSpinCtrl* spnWaveThreshold;
			wxStaticText* lblHotkey;
			wxChoice* cboHotkey;
			wxButton* btnSquadLogo;
			wxTextCtrl* txtSquadLogo;
			wxCheckBox* chkReinforcement;
			wxCheckBox* chkIgnoreForGoals;
			wxCheckBox* chkNoArrivalMusic;
			wxCheckBox* chkNoArrivalMessage;
			wxCheckBox* chkNoDynamicGoals;
			wxButton* btnPrev;
			wxButton* btnNext;
			wxButton* btnDeleteWing;
			wxButton* btnDisbandWing;
			wxButton* btnInitialOrders;
			wxToggleButton* btnHideCues;
			wxPanel* pnlCues;
			wxStaticText* lblMinWaveDelay;
			wxSpinCtrl* spnMinWaveDelay;
			wxStaticText* lblMaxWaveDelay;
			wxSpinCtrl* spnMaxWaveDelay;
			wxStaticText* lblArrivalLocation;
			wxChoice* cboArrivalLocation;
			wxStaticText* lblArrivalTarget;
			wxChoice* cboArrivalTarget;
			wxStaticText* lblArrivalDistance;
			wxTextCtrl* txtArrivalDistance;
			wxStaticText* lblArrivalDelay;
			wxSpinCtrl* spnArrivalDelay;
			wxStaticText* lblArrivalDelaySeconds;
			wxButton* btnRestrictArrivalPaths;
			wxStaticText* lblArrivalCue;
			wxTreeCtrl* tctArrivalCues;
			wxCheckBox* chkNoArrivalWarp;
			wxStaticText* lblDepatureLocation;
			wxChoice* cboDepartureLocation;
			wxStaticText* lblDepartureTarget;
			wxChoice* cboDepartureTarget;
			wxStaticText* lblDepartureDelay;
			wxSpinCtrl* spnArrivalDelay1;
			wxStaticText* m_staticText1711;
			wxButton* btnRestrictDeparturePaths;
			wxStaticText* lblDepartureCue;
			wxTreeCtrl* tctDepartureCues;
			wxCheckBox* chkNoDepartureWarp;
			wxMenuBar* mbrWingEditor;
			wxMenu* mnuSelectWing;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmWingEditor( wxWindow* parent, wxWindowID id = ID_frmWingEditor, const wxString& title = wxT("Wing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
			
			~frmWingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgObjectEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgObjectEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			void _wxFB_OnFace( wxCommandEvent& event ){ OnFace( event ); }
			void _wxFB_OnOrientationOpt( wxCommandEvent& event ){ OnOrientationOpt( event ); }
			void _wxFB_OnCancel( wxCommandEvent& event ){ OnCancel( event ); }
			void _wxFB_OnOK( wxCommandEvent& event ){ OnOK( event ); }
			
		
		protected:
			enum
			{
				ID_dlgObjectEditor = 1000
			};
			
			wxStaticText* m_staticText217;
			wxSpinCtrl* spnPositionX;
			wxStaticText* m_staticText218;
			wxSpinCtrl* spnPositionY;
			wxStaticText* m_staticText220;
			wxSpinCtrl* spnPositionZ;
			wxCheckBox* chkPointTo;
			wxPanel* pnlOrientation;
			wxRadioButton* optObject;
			wxChoice* cbObject;
			wxRadioButton* optLocation;
			wxStaticText* lblLocationX;
			wxSpinCtrl* spnLocationX;
			wxStaticText* lblLocationY;
			wxSpinCtrl* spnLocationY;
			wxStaticText* lblLocationZ;
			wxSpinCtrl* spnLocationZ;
			wxRadioButton* optAngle;
			wxStaticText* lblPitch;
			wxSpinCtrl* spnPitch;
			wxStaticText* lblBank;
			wxSpinCtrl* spnBank;
			wxStaticText* lblHeading;
			wxSpinCtrl* spnHeading;
			wxStdDialogButtonSizer* sdbObjectEditor;
			wxButton* sdbObjectEditorOK;
			wxButton* sdbObjectEditorCancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			virtual void OnFace( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOrientationOpt( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgObjectEditor( wxWindow* parent, wxWindowID id = ID_dlgObjectEditor, const wxString& title = wxT("Object Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgObjectEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmWaypointEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmWaypointEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_frmWaypointEditor = 1000
			};
			
			wxMenuBar* menuWaypoint;
			wxMenu* menuPaths;
			wxStaticText* lblWaypointName;
			wxTextCtrl* txtWaypointName;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmWaypointEditor( wxWindow* parent, wxWindowID id = ID_frmWaypointEditor, const wxString& title = wxT("Waypoint Path/Jump Node"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 300,80 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmWaypointEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgMissionObjectivesEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgMissionObjectivesEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgMissionsObjectivesEditor = 1000
			};
			
			wxTreeCtrl* tctObjectives;
			wxStaticText* m_staticText117;
			wxChoice* m_choice27;
			wxStaticText* lblObjType;
			wxChoice* cboObjType;
			wxStaticText* lblObjName;
			wxTextCtrl* txtObjName;
			wxStaticText* lblObjText;
			wxTextCtrl* txtObjText;
			wxStaticText* lblObjScore;
			wxTextCtrl* txtObjScore;
			wxStaticText* lblObjTeam;
			wxChoice* cboObjTeam;
			wxCheckBox* cboCurrObjInvalid;
			wxCheckBox* cboCurrObjNoCompletionSound;
			wxButton* btnNewObjective;
			wxButton* btnConfirm;
			wxButton* btnCancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgMissionObjectivesEditor( wxWindow* parent, wxWindowID id = ID_dlgMissionsObjectivesEditor, const wxString& title = wxT("Mission Objectives"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgMissionObjectivesEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgEventsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgEventsEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgEventsEditor = 1000
			};
			
			wxPanel* pnlEvents;
			wxTreeCtrl* trbSexp;
			wxButton* btnNewEvent;
			wxButton* btnInsertEvent;
			wxButton* btnDeleteEvent;
			wxStaticText* lblRepeatCount;
			wxTextCtrl* txtRepeatCount;
			wxStaticText* lblTriggerCount;
			wxTextCtrl* txtTriggerCount;
			wxStaticText* lblIntervalTime;
			wxTextCtrl* txtIntervalTime;
			wxStaticText* lblScore;
			wxTextCtrl* txtScore;
			wxStaticText* lblTeam;
			wxChoice* cboTeam;
			wxCheckBox* chkChained;
			wxStaticText* lblChainDelay;
			wxTextCtrl* txtChainDelay;
			wxStaticText* lblDirectiveText;
			wxTextCtrl* txtDirectiveText;
			wxStaticText* lblDirectiveKeypress;
			wxTextCtrl* txtDirectiveKeypress;
			wxStaticText* lblStateLogging;
			wxCheckBox* chkTrue;
			wxCheckBox* chkTrueAlways;
			wxCheckBox* chkRepeatFirst;
			wxCheckBox* chkTriggerFirst;
			wxCheckBox* chkFalse;
			wxCheckBox* chkFalseAlways;
			wxCheckBox* chkRepeatLast;
			wxCheckBox* chkTriggerLast;
			wxPanel* pnlMessages;
			wxListBox* lstMessages;
			wxButton* btnNewMessage;
			wxButton* btnDeleteMessage;
			wxStaticText* lblMessageName;
			wxTextCtrl* txtMessageName;
			wxStaticText* lblMessageText;
			wxTextCtrl* txtMessageText;
			wxStaticText* lblMessageANI;
			wxComboBox* cboMessageANI;
			wxButton* btnANIBrowse;
			wxStaticText* lblMessageAudio;
			wxComboBox* cboMessageAudio;
			wxButton* btnAudioBrowse;
			wxBitmapButton* btnPlayAudio;
			wxStaticText* lblPersona;
			wxComboBox* m_comboBox9;
			wxButton* btnUpdateStuff;
			wxStaticText* lblMessageTeam;
			wxChoice* cboTeamMessage;
			wxStaticText* m_staticText139;
			wxStdDialogButtonSizer* m_sdbSizer6;
			wxButton* m_sdbSizer6OK;
			wxButton* m_sdbSizer6Cancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgEventsEditor( wxWindow* parent, wxWindowID id = ID_dlgEventsEditor, const wxString& title = wxT("Events Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMINIMIZE_BOX ); 
			~dlgEventsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmTeamLoadoutEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmTeamLoadoutEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			void _wxFB_OnCancel( wxCommandEvent& event ){ OnCancel( event ); }
			void _wxFB_OnOK( wxCommandEvent& event ){ OnOK( event ); }
			
		
		protected:
			enum
			{
				ID_frmTeamLoadoutEditor = 1000,
				ID_mnuTeam1,
				ID_mnuTeam2,
				ID_lbxStartShipsVariable,
				ID_cboSetShipAmountFromVariable,
				ID_StartWeaponsVariable
			};
			
			wxMenuBar* mnbDialogMenu;
			wxMenu* mnuSelectTeam;
			wxMenu* m_menu16;
			wxStaticText* lblAvailableStartShips;
			wxStaticText* lblShipsFromVariable;
			wxListBox* lbxStartShipsVariable;
			wxStaticText* lblShipsFromTbl;
			wxCheckListBox* cklShipsFromTbl;
			wxStaticText* m_staticText141;
			wxSpinCtrl* spnExtraShipsAvailable;
			wxStaticText* lblSetShipAmountFromVariable;
			wxChoice* cboSetShipAmountFromVariable;
			wxStaticText* lblAmountOfShipsInWings;
			wxTextCtrl* txtAmountOfShipsInWings;
			wxStaticLine* m_staticline2;
			wxStaticText* lblAvailableStartWeapons;
			wxStaticText* lblWeaponsFromVariable;
			wxListBox* lbxStartWeaponsVariable;
			wxStaticText* lblWeaponsFromTbl;
			wxCheckListBox* cklWeaponsFromTbl;
			wxStaticText* m_staticText1411;
			wxSpinCtrl* spnExtraWeaponsAvailable;
			wxStaticText* lblSetWeaponAmountFromVariable;
			wxChoice* cboSetWeaponAmountFromVariable;
			wxStaticText* lblAmountOfWeaponsInWings;
			wxTextCtrl* txtAmountOfWeaponsInWings;
			wxStdDialogButtonSizer* sdbDialogButtons;
			wxButton* sdbDialogButtonsOK;
			wxButton* sdbDialogButtonsCancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmTeamLoadoutEditor( wxWindow* parent, wxWindowID id = ID_frmTeamLoadoutEditor, const wxString& title = wxT("Team Loadout Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,500 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmTeamLoadoutEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgBackgroundEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgBackgroundEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_BackgroundEditor = 1000
			};
			
			wxChoice* cboBackgroundPreset;
			wxButton* btnImportBackground;
			wxListCtrl* lclBGBitmaps;
			wxButton* btnBitmapAdd;
			wxButton* btnBitmapDelete;
			wxStaticText* lblBitmap;
			wxChoice* cboBitmapSelect;
			wxStaticText* lblBitmapPitch;
			wxStaticText* lblBitmapBank;
			wxStaticText* lblBitmapHeading;
			wxSpinCtrl* spnBitmapPitch;
			wxSpinCtrl* spnBitmapBank;
			wxSpinCtrl* spnBitmapHeading;
			wxStaticText* lblBitmapScale;
			wxSpinCtrl* spnBitmapScaleX;
			wxSpinCtrl* spnBitmapScaleY;
			wxStaticText* lblBitmapDivisions;
			wxSpinCtrl* spnBitmapDivisionsX;
			wxSpinCtrl* spnBitmapDivisionsY;
			wxCheckBox* chkFullNebula;
			wxCheckBox* chkToggleShipTrails;
			wxStaticText* lblNebulaRange;
			wxTextCtrl* txtNebulaRange;
			wxStaticText* lblNebulaPattern;
			wxChoice* cboNebulaPattern;
			wxStaticText* lblLightningStorm;
			wxChoice* cboLightningStorm;
			wxStaticText* lblNebulaFogNear;
			wxSpinCtrl* m_spinCtrl49;
			wxStaticText* lblNebulaFogMultiplier;
			wxSpinCtrl* m_spinCtrl50;
			wxStaticText* lblNebulaPoofs;
			wxCheckListBox* clbNebulaPoofs;
			wxStaticText* m_staticText160;
			wxChoice* cboNebulaPattern1;
			wxStaticText* m_staticText161;
			wxChoice* cboNebulaColour;
			wxStaticText* lblBitmapPitch2;
			wxStaticText* lblBitmapBank2;
			wxStaticText* lblBitmapHeading2;
			wxSpinCtrl* spnBitmapPitch2;
			wxSpinCtrl* spnBitmapBank2;
			wxSpinCtrl* spnBitmapHeading2;
			wxButton* btnBGSwap;
			wxChoice* cboBackgroundSwap;
			wxListCtrl* lclBGSunbitmaps;
			wxButton* btnAddSunBitmap;
			wxButton* btnDeleteSunBitmap;
			wxStaticText* lblSun;
			wxChoice* cboSunSelect;
			wxStaticText* lblBitmapPitch1;
			wxStaticText* lblBitmapBank1;
			wxStaticText* lblBitmapHeading1;
			wxSpinCtrl* spnBitmapPitch1;
			wxSpinCtrl* spnBitmapBank1;
			wxSpinCtrl* spnBitmapHeading1;
			wxStaticText* m_staticText179;
			wxSpinCtrl* spnSunScale;
			wxStaticText* lblAmbientRed;
			wxSlider* sldAmbientRed;
			wxSpinCtrl* spnAmbientRed;
			wxStaticText* lblAmbientGreen;
			wxSlider* sldAmbientGreen;
			wxSpinCtrl* spnAmbientGreen;
			wxStaticText* lblAmbientBlue;
			wxSlider* sldAmbientBlue;
			wxSpinCtrl* spnAmbientBlue;
			wxButton* btnSkyboxSelect;
			wxTextCtrl* txtSkybox;
			wxButton* btnSkyboxMap;
			wxTextCtrl* m_textCtrl73;
			wxStaticText* lblBitmapPitch21;
			wxStaticText* lblBitmapBank21;
			wxStaticText* lblBitmapHeading21;
			wxSpinCtrl* spnBitmapPitch21;
			wxSpinCtrl* spnBitmapBank21;
			wxSpinCtrl* spnBitmapHeading21;
			wxCheckBox* chkSBNoCull;
			wxCheckBox* chkSBNoGlowmaps;
			wxCheckBox* chkSBNoLighting;
			wxCheckBox* chkSBNoZBuffer;
			wxCheckBox* chkSBForceClamp;
			wxCheckBox* chkSBTransparent;
			wxStaticText* m_staticText186;
			wxSlider* sldNumStars;
			wxSpinCtrl* m_spinCtrl43;
			wxCheckBox* m_checkBox48;
			wxButton* btnEnvMap;
			wxTextCtrl* txtEnvMap;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgBackgroundEditor( wxWindow* parent, wxWindowID id = ID_BackgroundEditor, const wxString& title = wxT("Background Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgBackgroundEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgReinforcementsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgReinforcementsEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_ReinforcementsEditor = 1000
			};
			
			wxStaticText* lblReinforcements;
			wxListBox* lstReinforcements;
			wxButton* btnAdd;
			wxButton* btnDelete;
			wxStaticLine* m_staticline3;
			wxButton* btnOk;
			wxButton* btnCancel;
			wxStaticText* lblUses;
			wxSpinCtrl* spnUses;
			wxStaticText* lblDelayAfterArrival;
			wxSpinCtrl* spnDelayAfterArrival;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgReinforcementsEditor( wxWindow* parent, wxWindowID id = ID_ReinforcementsEditor, const wxString& title = wxT("Reinforcements Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgReinforcementsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgReinforcementsPicker
	///////////////////////////////////////////////////////////////////////////////
	class dlgReinforcementsPicker : public wxDialog 
	{
		private:
		
		protected:
			enum
			{
				ID_ReinforcementsPicker = 1000
			};
			
			wxListCtrl* lstReincforcements;
			wxButton* btnOK;
			wxButton* btnCancel;
		
		public:
			
			dlgReinforcementsPicker( wxWindow* parent, wxWindowID id = ID_ReinforcementsPicker, const wxString& title = wxT("Select Reinforcement Unit"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgReinforcementsPicker();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgAsteroidFieldEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgAsteroidFieldEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgAsteroidFieldEditor = 1000
			};
			
			wxCheckBox* chkAsteroidsEnabled;
			wxPanel* pFieldProperties;
			wxRadioButton* optFieldActive;
			wxRadioButton* optFieldPassive;
			wxRadioButton* optFieldtypeAsteroid;
			wxRadioButton* optFieldTypeShip;
			wxCheckBox* chkBrownRocks;
			wxChoice* cboObjType1;
			wxCheckBox* chkBlueRocks;
			wxChoice* cboObjType2;
			wxCheckBox* chkOrangeRocks;
			wxChoice* cboObjType3;
			wxStaticLine* m_staticline1;
			wxStaticText* lblNumberObjects;
			wxSpinCtrl* spnObjects;
			wxStaticText* m_staticText68;
			wxTextCtrl* numAvgSpeed;
			wxPanel* pBoundingBoxes;
			wxStaticText* lblOuterMinimum;
			wxStaticText* lblOuterMaximum;
			wxStaticText* lblOuterX;
			wxTextCtrl* txtOuterMinimumX;
			wxTextCtrl* txtOuterMaximumX;
			wxStaticText* lblOuterY;
			wxTextCtrl* txtOuterMinimumY;
			wxTextCtrl* txtOuterMaximumY;
			wxStaticText* lblOuterZ;
			wxTextCtrl* txtOuterMinimumZ;
			wxTextCtrl* txtOuterMaximumZ;
			wxCheckBox* chkInnerBoxEnable;
			wxStaticText* lblInnerMinimum;
			wxStaticText* lblInnerMaximum;
			wxStaticText* lblInnerX;
			wxTextCtrl* numInnerBoxMinX;
			wxTextCtrl* numInnerBoxMaxX;
			wxStaticText* lblInnerY;
			wxTextCtrl* numInnerBoxMinY;
			wxTextCtrl* numInnerBoxMaxY;
			wxStaticText* lblInnerZ;
			wxTextCtrl* numInnerBoxMinZ;
			wxTextCtrl* numInnerBoxMaxZ;
			wxStdDialogButtonSizer* grpButtons;
			wxButton* grpButtonsOK;
			wxButton* grpButtonsCancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgAsteroidFieldEditor( wxWindow* parent, wxWindowID id = ID_dlgAsteroidFieldEditor, const wxString& title = wxT("Asteroid Field Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER ); 
			~dlgAsteroidFieldEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgMissionSpecsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgMissionSpecsEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgMissionSpecsEditor = 1000
			};
			
			wxStaticText* lblTitle;
			wxTextCtrl* txtTitle;
			wxStaticText* lblDesigner;
			wxTextCtrl* txtDesigner;
			wxStaticText* lblCreated;
			wxStaticText* txtCreated;
			wxStaticText* lblModified;
			wxStaticText* txtModified;
			wxRadioButton* optSinglePlayer;
			wxRadioButton* optMultiPlayer;
			wxRadioButton* optTraining;
			wxPanel* pnlMultiplayer;
			wxRadioButton* optCooperative;
			wxRadioButton* optTeamVsTeam;
			wxRadioButton* optDogfight;
			wxStaticText* m_staticText75;
			wxSpinCtrl* spnMaxRespawns;
			wxStaticText* m_staticText76;
			wxSpinCtrl* spnMaxRespawnDelay;
			wxStaticText* m_staticText74;
			wxTextCtrl* txtSquadronName;
			wxButton* btnSquadronLogo;
			wxTextCtrl* txtSquadronLogo;
			wxCheckBox* chkDisallowSupportShips;
			wxCheckBox* chkSupportShipsRepairHull;
			wxPanel* pnlRepairHull;
			wxStaticText* lblRepairCeiling;
			wxStaticText* lblHullRepairCeiling;
			wxSpinCtrl* spnHullRepairCeiling;
			wxStaticText* lblHullPercent;
			wxStaticText* lblSubsystemRepairCeiling;
			wxSpinCtrl* spnSubsystemRepairCeiling;
			wxStaticText* lblSubstemPercent;
			wxCheckBox* chkToggleNebula;
			wxCheckBox* chkMinimumTrailSpeed;
			wxSpinCtrl* spnMinimumTrailSpeed;
			wxStaticText* m_staticText71;
			wxComboBox* cboMessageSender;
			wxStaticText* m_staticText72;
			wxChoice* cboPersona;
			wxStaticText* m_staticText73;
			wxChoice* cboMusic;
			wxStaticText* m_staticText741;
			wxComboBox* cboMusicPackPresent;
			wxButton* btnSoundEnvironment;
			wxCheckListBox* m_checkList4;
			wxStaticText* m_staticText771;
			wxChoice* cboAIProfile;
			wxButton* btnLoadingScreen640x480;
			wxTextCtrl* txtLoadingScreen640x480;
			wxButton* btnLoadingScreen1024x768;
			wxTextCtrl* txtLoadingScreen1024x768;
			wxButton* btnCustomWingNames;
			wxStaticText* m_staticText159;
			wxSpinCtrl* m_spinCtrl18;
			wxStaticText* lblMissionDescription;
			wxTextCtrl* txtMissionDescription;
			wxStaticText* lblDesignerNotes;
			wxTextCtrl* txtDesignerNotes;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgMissionSpecsEditor( wxWindow* parent, wxWindowID id = ID_dlgMissionSpecsEditor, const wxString& title = wxT("Mission Specs"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,590 ), long style = wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU ); 
			~dlgMissionSpecsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgSoundEnvironment
	///////////////////////////////////////////////////////////////////////////////
	class dlgSoundEnvironment : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnOK( wxCommandEvent& event ){ OnOK( event ); }
			void _wxFB_OnCancel( wxCommandEvent& event ){ OnCancel( event ); }
			
		
		protected:
			enum
			{
				ID_dlgSoundEnvironment = 1000
			};
			
			wxStaticText* lblEnvironment;
			wxComboBox* cboEnvironment;
			wxStaticText* lblVolume;
			wxSpinCtrl* spnVolume;
			wxStaticText* lblDamping;
			wxSpinCtrl* spnDamping;
			wxStaticText* lblDecayTime;
			wxSpinCtrl* spnDecayTime;
			wxStaticText* lblDecaySeconds;
			wxButton* btnOK;
			wxButton* btnCancel;
			wxBitmapButton* m_bpButton7;
			wxFilePickerCtrl* m_filePicker2;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgSoundEnvironment( wxWindow* parent, wxWindowID id = ID_dlgSoundEnvironment, const wxString& title = wxT("Sound Environment"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgSoundEnvironment();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmBriefingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmBriefingEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_frmBriefingEditor = 1000
			};
			
			wxMenuBar* m_menubar8;
			wxMenu* mnuSelectTeam;
			wxMenu* mnuOptions;
			wxPanel* m_panel13;
			wxStaticText* lblStage;
			wxStaticText* lblCameraTransisitonTime;
			wxSpinCtrl* m_spinCtrl53;
			wxStaticText* m_staticText192;
			wxCheckBox* chkCutToNextStage;
			wxCheckBox* chkCutToPreviousStage;
			wxStaticText* lblText;
			wxTextCtrl* m_textCtrl75;
			wxButton* btnPreviousStage;
			wxButton* btnNextStage;
			wxButton* btnAddStage;
			wxButton* btnInsertStage;
			wxButton* btnDeleteStage;
			wxButton* btnSaveView;
			wxButton* btnGoToView;
			wxButton* btnCopyView;
			wxButton* btnPasteView;
			wxStaticText* lblVoiceFile;
			wxFilePickerCtrl* m_filePicker1;
			wxBitmapButton* btnPlayVoice;
			wxStaticText* lblDefaultMusic;
			wxChoice* m_choice42;
			wxBitmapButton* btnPlayMusic;
			wxStaticText* m_staticText196;
			wxChoice* m_choice43;
			wxBitmapButton* btnPlayMusicFromPack;
			wxStaticText* lblUsageFormula;
			wxTreeCtrl* m_treeCtrl9;
			wxCheckBox* chkDrawLines;
			wxStaticText* lblIconLabel;
			wxTextCtrl* txtIconLabel;
			wxStaticText* lblIconImage;
			wxComboBox* cboIconImage;
			wxStaticText* lblShipType;
			wxComboBox* cboShipType;
			wxStaticText* lblIconTeam;
			wxComboBox* m_comboBox13;
			wxStaticText* lblIconId;
			wxTextCtrl* txtIconID;
			wxCheckBox* chkHighlightIcon;
			wxCheckBox* chkChangeLocally;
			wxCheckBox* chkFlipIconLR;
			wxButton* btnMakeIcon;
			wxButton* btnDeleteIcon;
			wxButton* btnPropagate;
			wxStaticText* lblIconText;
			wxTextCtrl* txtIconText;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmBriefingEditor( wxWindow* parent, wxWindowID id = ID_frmBriefingEditor, const wxString& title = wxT("Briefing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmBriefingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmDebriefingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmDebriefingEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_frmDebriefingEditor = 1000
			};
			
			wxPanel* pnlMain;
			wxStaticText* txtStages;
			wxButton* btnPrev;
			wxButton* btnNext;
			wxButton* btnAdd;
			wxButton* btnInsert;
			wxButton* btnDelete;
			wxStaticText* lblUsageFormula;
			wxTreeCtrl* treeUsageFormula;
			wxStaticText* lblVoiceWaveFile;
			wxTextCtrl* txtVoiceWaveFile;
			wxButton* btnBrowse;
			wxBitmapButton* btnPlayVoice;
			wxStaticText* lblStageText;
			wxTextCtrl* txtStageText;
			wxStaticText* lblRecommendationText;
			wxTextCtrl* txtRecommendationText;
			wxPanel* pnlMusic;
			wxStaticText* lblMusicSuccess;
			wxChoice* cbMusicSuccess;
			wxBitmapButton* btnPlaySuccess;
			wxStaticText* m_staticText210;
			wxChoice* m_choice45;
			wxBitmapButton* btnPlayNuetral;
			wxStaticText* m_staticText211;
			wxChoice* m_choice46;
			wxBitmapButton* btnPlayFailure;
			wxMenuBar* mnbDebriefingEditor;
			wxMenu* mnuSelectTeam;
			wxMenu* mnuOptions;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmDebriefingEditor( wxWindow* parent, wxWindowID id = ID_frmDebriefingEditor, const wxString& title = wxT("Debriefing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmDebriefingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmCommandBriefingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmCommandBriefingEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_frmCommandBriefingEditor = 1000,
				ID_btnOK,
				ID_btnCancel
			};
			
			wxMenuBar* m_menubar6;
			wxMenu* mnuSelectTeam;
			wxMenu* mnuOptions;
			wxPanel* m_panel5;
			wxStaticText* txtNumCBStages;
			wxButton* btnPrev;
			wxButton* btnNext;
			wxButton* btnAdd;
			wxButton* btnInsert;
			wxButton* btnDelete;
			wxButton* btnOK;
			wxButton* btnCancel;
			wxStaticText* wxID_STATIC1;
			wxTextCtrl* txtStageText;
			wxStaticText* lblAniFile;
			wxFilePickerCtrl* fpAniFile;
			wxStaticText* lblVoiceWaveFile;
			wxFilePickerCtrl* fpVoiceWave;
			wxBitmapButton* btnVoicePlay;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmCommandBriefingEditor( wxWindow* parent, wxWindowID id = ID_frmCommandBriefingEditor, const wxString& title = wxT("Command Briefing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmCommandBriefingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgFictionViewer
	///////////////////////////////////////////////////////////////////////////////
	class dlgFictionViewer : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgFictionViewerEditor = 1000,
				ID_btnCancel
			};
			
			wxStaticText* lblStoryFile;
			wxFilePickerCtrl* fpStoryFile;
			wxStaticText* lblFontFile;
			wxFilePickerCtrl* fpFontFile;
			wxStaticText* lblMusic;
			wxChoice* cbMusic;
			wxBitmapButton* btnPlayMusic;
			wxButton* btnOK;
			wxButton* btnCancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgFictionViewer( wxWindow* parent, wxWindowID id = ID_dlgFictionViewerEditor, const wxString& title = wxT("Fiction Viewer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgFictionViewer();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgShieldSystemEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgShieldSystemEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgShieldSystemEditor = 1000
			};
			
			wxChoice* cboShipType;
			wxRadioButton* optShipTypeHasShieldSystem;
			wxRadioButton* optShipTypeNoShieldSystem;
			wxChoice* cboShipTeam;
			wxRadioButton* optShipTeamHasShieldSystem;
			wxRadioButton* optShipTeamNoShieldSystem;
			wxStdDialogButtonSizer* dlgShieldConfirm;
			wxButton* dlgShieldConfirmOK;
			wxButton* dlgShieldConfirmCancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgShieldSystemEditor( wxWindow* parent, wxWindowID id = ID_dlgShieldSystemEditor, const wxString& title = wxT("Shield System Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,153 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgShieldSystemEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgSetGlobalShipFlagsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgSetGlobalShipFlagsEditor : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgSetGlobalShipFlagsEditor = 1000
			};
			
			wxButton* btnGlobalNoShields;
			wxButton* btnGlobalNoSubspaceDrive;
			wxButton* btnGlobalPrimitiveSensors;
			wxButton* btnGlobalAffectedByGravity;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgSetGlobalShipFlagsEditor( wxWindow* parent, wxWindowID id = ID_dlgSetGlobalShipFlagsEditor, const wxString& title = wxT("Set Global Ship Flags"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 206,162 ), long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxSYSTEM_MENU ); 
			~dlgSetGlobalShipFlagsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgVoiceActingManager
	///////////////////////////////////////////////////////////////////////////////
	class dlgVoiceActingManager : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgVoiceActingManager = 1000
			};
			
			wxStaticText* lblCampaign;
			wxTextCtrl* txtAbbrevCampaign;
			wxStaticText* lblMission;
			wxTextCtrl* txtAbbrevMission;
			wxStaticText* lblCmdBriefingStage;
			wxTextCtrl* txtAbbrevCB;
			wxStaticText* lblBriefingStage;
			wxTextCtrl* txtAbbrevBriefing;
			wxStaticText* lblDebriefingStage;
			wxTextCtrl* txtAbbrevDebrief;
			wxStaticText* lblMessage;
			wxTextCtrl* txtAbbrevMessage;
			wxStaticText* lblAudioFileExtension;
			wxChoice* cboVAFileExt;
			wxStaticText* m_staticText125;
			wxTextCtrl* txtExampleFileName;
			wxCheckBox* chkVANoReplaceExistingFiles;
			wxButton* btnGenerateFileNames;
			wxTextCtrl* txtScriptEntryFormat;
			wxStaticText* lblScriptHelp;
			wxRadioButton* optEverything;
			wxRadioButton* optJustCommandBriefings;
			wxRadioButton* optJustBriefings;
			wxRadioButton* optJustDebriefings;
			wxRadioButton* optJustMessages;
			wxCheckBox* chkGroupMessageList;
			wxButton* btnGenerateScript;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgVoiceActingManager( wxWindow* parent, wxWindowID id = ID_dlgVoiceActingManager, const wxString& title = wxT("Voice Acting Manager"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxSYSTEM_MENU ); 
			~dlgVoiceActingManager();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmCampaignEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmCampaignEditor : public wxFrame 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_frmCampaignEditor = 1000,
				ID_lstAvailableMissions,
				ID_NEW,
				ID_OPEN,
				ID_SAVE,
				ID_SAVE_AS,
				ID_EXIT,
				ID_ERROR_CHECKER,
				ID_SHIPS,
				ID_WEAPONS
			};
			
			wxSplitterWindow* m_splitter1;
			wxScrolledWindow* pnlCampaign;
			wxStaticText* lblAvailableMissions;
			wxListCtrl* lstAvailableMissions;
			wxStaticText* lblCampaignName;
			wxTextCtrl* txtCampaignName;
			wxStaticText* lblCampaignType;
			wxChoice* cbCampaignType;
			wxStaticText* lblCampaignDescription;
			wxTextCtrl* txtCampaignDescription;
			wxCheckBox* chkUsesCustomTechDatabase;
			wxStaticText* lblBriefingCutscene;
			wxFilePickerCtrl* fpBriefingCutscene;
			wxStaticText* lblMainhallIndex;
			wxSpinCtrl* spnMainHallIndex;
			wxStaticText* lblDebriefingPersonaIndex;
			wxSpinCtrl* spnDebriefingPersonaIndex;
			wxStaticText* lblBranches;
			wxTreeCtrl* treeBranches;
			wxButton* btnMoveUp;
			wxButton* btnMoveDown;
			wxButton* btnToggleLoop;
			wxButton* btnRealignTree;
			wxButton* btnLoadMission;
			wxButton* btnClose;
			wxStaticText* lblDesignerNotes;
			wxTextCtrl* txtDesignerNotes;
			wxStaticText* lblMissionLoopDiscription;
			wxTextCtrl* txtMissionLoopDescription;
			wxStaticText* lblLoopBriefAni;
			wxFilePickerCtrl* fpLoopBriefAni;
			wxStaticText* lblBriefVoice;
			wxFilePickerCtrl* fpLoopBriefVoice;
			wxPanel* pnlCampaignGraph;
			wxMenuBar* m_menubar2;
			wxMenu* mnuFile;
			wxMenu* other;
			wxMenu* initialStatus;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			frmCampaignEditor( wxWindow* parent, wxWindowID id = ID_frmCampaignEditor, const wxString& title = wxT("Campaign Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 862,705 ), long style = wxDEFAULT_FRAME_STYLE|wxFRAME_FLOAT_ON_PARENT|wxTAB_TRAVERSAL );
			
			~frmCampaignEditor();
			
			void m_splitter1OnIdle( wxIdleEvent& )
			{
				m_splitter1->SetSashPosition( 0 );
				m_splitter1->Disconnect( wxEVT_IDLE, wxIdleEventHandler( frmCampaignEditor::m_splitter1OnIdle ), NULL, this );
			}
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgMissionStats
	///////////////////////////////////////////////////////////////////////////////
	class dlgMissionStats : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgMissionStats = 1000
			};
			
			wxTextCtrl* txtMissionStats;
			wxButton* btnDumpToFile;
			wxButton* btnCancel;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgMissionStats( wxWindow* parent, wxWindowID id = ID_dlgMissionStats, const wxString& title = wxT("Mission Statistics"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,300 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgMissionStats();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgAboutBox
	///////////////////////////////////////////////////////////////////////////////
	class dlgAboutBox : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgAboutBox = 1000
			};
			
			wxStaticBitmap* bmpLogo;
			wxStaticText* lblAppTitle;
			wxStaticText* lblCopywrite;
			wxStaticText* lblDevelopers;
			wxStaticText* lblQuote;
			wxButton* btnOK;
			wxButton* btnReportBug;
			wxButton* btnVisitForums;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgAboutBox( wxWindow* parent, wxWindowID id = ID_dlgAboutBox, const wxString& title = wxT("About FRED2"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxSYSTEM_MENU ); 
			~dlgAboutBox();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgSexpHelp
	///////////////////////////////////////////////////////////////////////////////
	class dlgSexpHelp : public wxDialog 
	{
		DECLARE_EVENT_TABLE()
		private:
			
			// Private event handlers
			void _wxFB_OnClose( wxCloseEvent& event ){ OnClose( event ); }
			
		
		protected:
			enum
			{
				ID_dlgSexpHelp = 1000
			};
			
			wxPanel* pnlSexpHelp;
			wxStaticText* lblArgInfo;
			wxTextCtrl* txtArgInfo;
			wxStaticText* lblSexpInfo;
			wxTextCtrl* txtSexpInfo;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			
		
		public:
			
			dlgSexpHelp( wxWindow* parent, wxWindowID id = ID_dlgSexpHelp, const wxString& title = wxT("Sexp Help"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 406,347 ), long style = wxCAPTION|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
			~dlgSexpHelp();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class pnlSexpHelp
	///////////////////////////////////////////////////////////////////////////////
	class pnlSexpHelp : public wxPanel 
	{
		private:
		
		protected:
			enum
			{
				ID_pnlSexpHelp = 1000
			};
			
			wxStaticText* lblArgInfo;
			wxTextCtrl* txtArgInfo;
			wxStaticText* lblSexpInfo;
			wxTextCtrl* txtSexpInfo;
		
		public:
			
			pnlSexpHelp( wxWindow* parent, wxWindowID id = ID_pnlSexpHelp, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL ); 
			~pnlSexpHelp();
		
	};
	
} // namespace fredBase

#endif //__WXFRED_BASE_H__
