///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
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
		private:
		
		protected:
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
			wxToolBarToolBase* optSelect; 
			wxToolBarToolBase* optSelectMove; 
			wxToolBarToolBase* optSelectRotate; 
			wxToolBarToolBase* chkRotateLocally; 
			wxToolBarToolBase* optConstraintX; 
			wxToolBarToolBase* optConstraintY; 
			wxToolBarToolBase* optConstraintZ; 
			wxToolBarToolBase* optConstraintXZ; 
			wxToolBarToolBase* optConstraintYZ; 
			wxToolBarToolBase* optConstraintXY; 
			wxToolBarToolBase* btnSelectionList; 
			wxToolBarToolBase* chkSelectionLock; 
			wxToolBarToolBase* btnWingForm; 
			wxToolBarToolBase* btnWingDisband; 
			wxToolBarToolBase* btnZoomSelected; 
			wxToolBarToolBase* btnZoomExtents; 
			wxToolBarToolBase* chkShowDistances; 
			wxToolBarToolBase* chkOrbitSelected; 
		
		public:
			
			frmFRED( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmFRED();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmShipsEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmShipsEditor : public wxFrame 
	{
		private:
		
		protected:
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
		
		public:
			
			frmShipsEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Edit Ship"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_NO_TASKBAR|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
			
			~frmShipsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmWingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmWingEditor : public wxFrame 
	{
		private:
		
		protected:
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
		
		public:
			
			frmWingEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Wing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
			
			~frmWingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgObjectEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgObjectEditor : public wxDialog 
	{
		private:
		
		protected:
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
			wxStdDialogButtonSizer* m_sdbSizer6;
			wxButton* m_sdbSizer6OK;
			wxButton* m_sdbSizer6Cancel;
		
		public:
			
			dlgObjectEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Object Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgObjectEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmWaypointEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmWaypointEditor : public wxFrame 
	{
		private:
		
		protected:
			wxMenuBar* menuWaypoint;
			wxMenu* menuPaths;
			wxStaticText* lblWaypointName;
			wxTextCtrl* txtWaypointName;
		
		public:
			
			frmWaypointEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Waypoint Path/Jump Node"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 300,80 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmWaypointEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgMissionObjectivesEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgMissionObjectivesEditor : public wxDialog 
	{
		private:
		
		protected:
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
		
		public:
			
			dlgMissionObjectivesEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Mission Objectives"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgMissionObjectivesEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgEventsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgEventsEditor : public wxDialog 
	{
		private:
		
		protected:
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
			wxStdDialogButtonSizer* m_sdbSizer7;
			wxButton* m_sdbSizer7OK;
			wxButton* m_sdbSizer7Cancel;
		
		public:
			
			dlgEventsEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Events Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxMINIMIZE_BOX ); 
			~dlgEventsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmTeamLoadoutEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmTeamLoadoutEditor : public wxFrame 
	{
		private:
		
		protected:
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
			wxStdDialogButtonSizer* m_sdbSizer8;
			wxButton* m_sdbSizer8OK;
			wxButton* m_sdbSizer8Cancel;
		
		public:
			
			frmTeamLoadoutEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Team Loadout Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,500 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmTeamLoadoutEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgBackgroundEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgBackgroundEditor : public wxDialog 
	{
		private:
		
		protected:
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
		
		public:
			
			dlgBackgroundEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Background Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgBackgroundEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgReinforcementsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgReinforcementsEditor : public wxDialog 
	{
		private:
		
		protected:
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
		
		public:
			
			dlgReinforcementsEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Reinforcements Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgReinforcementsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgReinforcementsPicker
	///////////////////////////////////////////////////////////////////////////////
	class dlgReinforcementsPicker : public wxDialog 
	{
		private:
		
		protected:
			wxListCtrl* lstReincforcements;
			wxButton* btnOK;
			wxButton* btnCancel;
		
		public:
			
			dlgReinforcementsPicker( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Select Reinforcement Unit"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgReinforcementsPicker();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgAsteroidFieldEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgAsteroidFieldEditor : public wxDialog 
	{
		private:
		
		protected:
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
			wxStdDialogButtonSizer* m_sdbSizer9;
			wxButton* m_sdbSizer9OK;
			wxButton* m_sdbSizer9Cancel;
		
		public:
			
			dlgAsteroidFieldEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Asteroid Field Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER ); 
			~dlgAsteroidFieldEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgMissionSpecsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgMissionSpecsEditor : public wxDialog 
	{
		private:
		
		protected:
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
		
		public:
			
			dlgMissionSpecsEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Mission Specs"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,590 ), long style = wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU ); 
			~dlgMissionSpecsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgSoundEnvironment
	///////////////////////////////////////////////////////////////////////////////
	class dlgSoundEnvironment : public wxDialog 
	{
		private:
		
		protected:
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
		
		public:
			
			dlgSoundEnvironment( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Sound Environment"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgSoundEnvironment();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmBriefingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmBriefingEditor : public wxFrame 
	{
		private:
		
		protected:
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
		
		public:
			
			frmBriefingEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Briefing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmBriefingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmDebriefingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmDebriefingEditor : public wxFrame 
	{
		private:
		
		protected:
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
		
		public:
			
			frmDebriefingEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Debriefing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmDebriefingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmCommandBriefingEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmCommandBriefingEditor : public wxFrame 
	{
		private:
		
		protected:
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
		
		public:
			
			frmCommandBriefingEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Command Briefing Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~frmCommandBriefingEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgFictionViewer
	///////////////////////////////////////////////////////////////////////////////
	class dlgFictionViewer : public wxDialog 
	{
		private:
		
		protected:
			wxStaticText* lblStoryFile;
			wxFilePickerCtrl* fpStoryFile;
			wxStaticText* lblFontFile;
			wxFilePickerCtrl* fpFontFile;
			wxStaticText* lblMusic;
			wxChoice* cbMusic;
			wxBitmapButton* btnPlayMusic;
			wxButton* btnOK;
			wxButton* btnCancel;
		
		public:
			
			dlgFictionViewer( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Fiction Viewer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgFictionViewer();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgShieldSystemEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgShieldSystemEditor : public wxDialog 
	{
		private:
		
		protected:
			wxChoice* cboShipType;
			wxRadioButton* optShipTypeHasShieldSystem;
			wxRadioButton* optShipTypeNoShieldSystem;
			wxChoice* cboShipTeam;
			wxRadioButton* optShipTeamHasShieldSystem;
			wxRadioButton* optShipTeamNoShieldSystem;
			wxStdDialogButtonSizer* m_sdbSizer10;
			wxButton* m_sdbSizer10OK;
			wxButton* m_sdbSizer10Cancel;
		
		public:
			
			dlgShieldSystemEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Shield System Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,153 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgShieldSystemEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgSetGlobalShipFlagsEditor
	///////////////////////////////////////////////////////////////////////////////
	class dlgSetGlobalShipFlagsEditor : public wxDialog 
	{
		private:
		
		protected:
			wxButton* btnGlobalNoShields;
			wxButton* btnGlobalNoSubspaceDrive;
			wxButton* btnGlobalPrimitiveSensors;
			wxButton* btnGlobalAffectedByGravity;
		
		public:
			
			dlgSetGlobalShipFlagsEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Set Global Ship Flags"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 206,162 ), long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxSYSTEM_MENU ); 
			~dlgSetGlobalShipFlagsEditor();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgVoiceActingManager
	///////////////////////////////////////////////////////////////////////////////
	class dlgVoiceActingManager : public wxDialog 
	{
		private:
		
		protected:
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
		
		public:
			
			dlgVoiceActingManager( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Voice Acting Manager"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxSYSTEM_MENU ); 
			~dlgVoiceActingManager();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class frmCampaignEditor
	///////////////////////////////////////////////////////////////////////////////
	class frmCampaignEditor : public wxFrame 
	{
		private:
		
		protected:
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
		
		public:
			
			frmCampaignEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Campaign Editor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 862,705 ), long style = wxDEFAULT_FRAME_STYLE|wxFRAME_FLOAT_ON_PARENT|wxTAB_TRAVERSAL );
			
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
		private:
		
		protected:
			wxTextCtrl* txtMissionStats;
			wxButton* btnDumpToFile;
			wxButton* btnCancel;
		
		public:
			
			dlgMissionStats( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Mission Statistics"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,300 ), long style = wxDEFAULT_DIALOG_STYLE ); 
			~dlgMissionStats();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgAboutBox
	///////////////////////////////////////////////////////////////////////////////
	class dlgAboutBox : public wxDialog 
	{
		private:
		
		protected:
			wxStaticBitmap* bmpLogo;
			wxStaticText* lblAppTitle;
			wxStaticText* lblCopywrite;
			wxStaticText* lblDevelopers;
			wxStaticText* lblQuote;
			wxButton* btnOK;
			wxButton* btnReportBug;
			wxButton* btnVisitForums;
		
		public:
			
			dlgAboutBox( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("About FRED2"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxSYSTEM_MENU ); 
			~dlgAboutBox();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class dlgSexpHelp
	///////////////////////////////////////////////////////////////////////////////
	class dlgSexpHelp : public wxDialog 
	{
		private:
		
		protected:
			wxPanel* pnlSexpHelp;
			wxStaticText* lblArgInfo;
			wxTextCtrl* txtArgInfo;
			wxStaticText* lblSexpInfo;
			wxTextCtrl* txtSexpInfo;
		
		public:
			
			dlgSexpHelp( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Sexp Help"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 406,347 ), long style = wxCAPTION|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
			~dlgSexpHelp();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class pnlSexpHelp
	///////////////////////////////////////////////////////////////////////////////
	class pnlSexpHelp : public wxPanel 
	{
		private:
		
		protected:
			wxStaticText* lblArgInfo;
			wxTextCtrl* txtArgInfo;
			wxStaticText* lblSexpInfo;
			wxTextCtrl* txtSexpInfo;
		
		public:
			
			pnlSexpHelp( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL ); 
			~pnlSexpHelp();
		
	};
	
} // namespace fredBase

#endif //__WXFRED_BASE_H__
