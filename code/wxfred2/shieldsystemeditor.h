/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/ShieldSystemEditor.h $
 * $Revision: 1.1 $
 * $Date: 2005-05-12 14:00:14 $
 * $Author: Goober5000 $
 *
 * Shield system editor
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.0  2005/05/12 07:30:00  Goober5000
 * Addition to CVS repository
 *
 */

#ifndef _SHIELD_SYSTEM_EDITOR_H
#define _SHIELD_SYSTEM_EDITOR_H

class dlgShieldSystemEditor : public wxDialog
{
	public:
		// constructor/destructors
		dlgShieldSystemEditor(wxWindow *parent);
		~dlgShieldSystemEditor();

		// event handlers
		// event functions go here

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
