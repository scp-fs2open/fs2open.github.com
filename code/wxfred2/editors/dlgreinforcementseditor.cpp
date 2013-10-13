/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */
#include "editors/dlgreinforcementseditor.h"
#include "editors/dlgreinforcementspicker.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// Public Members:
dlgReinforcementsEditor::dlgReinforcementsEditor( wxWindow* parent, wxWindowID id )
	: fredBase::dlgReinforcementsEditor(parent, id)
{
	dlgReinforcementsPicker_p = new dlgReinforcementsPicker(this);
}

// Protected Members:
	// Handlers for dlgReinforcementsEditor
void dlgReinforcementsEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed(this);
	Destroy();
}

void dlgReinforcementsEditor::OnAdd( wxCommandEvent& event )
{
	dlgReinforcementsPicker_p->Center();
	dlgReinforcementsPicker_p->ShowModal();
}

void dlgReinforcementsEditor::OnRemove( wxCommandEvent& event )
{
}

void dlgReinforcementsEditor::OnOK( wxCommandEvent& event )
{
	Close();
}

void dlgReinforcementsEditor::OnCancel( wxCommandEvent& event )
{
	Close();
}
