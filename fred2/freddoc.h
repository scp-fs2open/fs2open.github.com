#ifndef _FREDDOC_H
#define _FREDDOC_H
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */
#include "MissionSave.h"

#define MISSION_BACKUP_NAME "Backup"

#define US_WORLD_CHANGED    0x01
#define US_VIEW_CHANGED     0x02

/**
 * @class CFREDDoc
 *
 * @brief The document object for FRED mission files
 *
 * @details Handles the saving, loading of mission files, as well as a simple Undo stack
 */
class CFREDDoc : public CDocument
{
public:
	/**
	 * @brief Checks we can do another Undo()
	 *
	 * @returns 0 If we can not, or
	 * @returns 1 If we can
	 */
	int check_undo();

	/**
	 * @brief Pops off an Undo item from the stack and loads it as the active mission.
	 *
	 * @returns The file index of the loaded Undo item
	 */
	bool autoload();

	/**
	 * @brief Read in a new mission file from disk
	 *
	 * @param[in] pathname The full filepath name of the file to open
	 * @param[in] flags
	 *
	 * @returns true on success, false on failure
	 */
	bool load_mission(char *pathname, int flags = 0);

	/**
	 * @brief Pushes an Undo item onto the stack
	 *
	 * @param[in] desc Optional. Description of the undo item
	 *
	 * @returns  0 If successful,
	 * @returns  0 If autosave is disabled, or
	 * @returns -1 If could not save to the stack
	 */
	int autosave(char *desc);

	//{{AFX_VIRTUAL(CFREDDoc)
	/**
	 * @brief Handler for New File operations
	 *
	 * @returns TRUE  if successful, or
	 * @returns FALSE otherwise
	 */
	virtual BOOL OnNewDocument();

	/**
	 * @brief Handler for serialization
	 *
	 * @details Does nothing now.  Handled by OnOpenDocument and OnSaveDocument.  This is because we want to avoid
	 * using the CArchive for file I/O  -JH
	 *
	 * @TODO Remove this. Need to check if anything tries to use it..
	 */
	virtual void Serialize(CArchive& ar);

	/**
	 * @brief Handler for Edit-> Clear All
	 *
	 * @note Doesn't seem to exist. Calls DeleteContents() anyway.
	 */
	virtual void OnEditClearAll();

	/**
	 * @brief Deletes the contents of the mission, starting with a clean slate
	 */
	virtual void DeleteContents();

	/**
	 * @brief Handler for opening a document
	 *
	 * @param[in] lpszPathName The full pathname of the file to open
	 *
	 * @returns TRUE  if successful, or
	 * @returns FALSE otherwise
	 */
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	/**
	* @brief Handler for saving a document
	*
	* @param[in] lpszPathName The full pathname of the file to save
	*
	* @returns TRUE  if successful, or
	* @returns FALSE otherwise
	*/
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

	/**
	 * @brief Standard deconstructor
	 */
	virtual ~CFREDDoc();

	/**
	 * @breif Sets the modified flag of the document
	 *
	 * @param[in] flags Unused. Setting to any other value does not set the modified flag of the document
	 *
	 * @TODO Check if we can remove the flags parameter.
	 */
	static void UpdateStatus(int flags = US_WORLD_CHANGED);

#ifdef _DEBUG
	/**
	 * @brief Asserts if the document is valid
	 *
	 * @details Currently just calls the CDocument's AssertValid()
	 */
	virtual void AssertValid() const;

	/**
	 * @brief Dumps debug info(?) to the given dump context
	 *
	 * @param dc Dump context?
	 *
	 * @details Currently just calls the CDocument's Dump()
	 */
	virtual void Dump(CDumpContext &dc) const;
#endif

	char mission_pathname[256];             //!< Full pathname to the opened mission

	CString undo_desc[BACKUP_DEPTH + 1];    //!< String array of the undo descriptions

protected:
	/**
	 * @brief Default constructor. Called from serialization only
	 */
	CFREDDoc();
	DECLARE_DYNCREATE(CFREDDoc)

	//{{AFX_MSG(CFREDDoc)
	/**
	 * @brief Unused.
	 *
	 * @TODO Remove.
	 */
	afx_msg void OnEditUndo();

	/**
	 * @brief Initialize (and also clear out) the mission.
	 */
	afx_msg void editor_init_mission();

	/**
	 * @brief Handler for File->Import FreeSpace Mission
	 * @author Goober5000
	 */
	afx_msg void OnFileImportFSM();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	/**
	 * @brief Recreate child dialogs.
	 * @author Goober5000
	 *
	 * @details Recreates the child dialogs that would be affected by a mission load. (Briefing dialog)
	 */
	void recreate_dialogs();
};

extern int Local_modified;      //!< Flag. Nonzero if the FREDDoc object is modified
extern int Undo_available;      //!< Flag. Nonzero if we can do an Undo call
extern int Undo_count;          //!< Number of Undo's we can do

/**
 * @brief Global pointer to the FREDDoc object
 *
 * @details :V: "Used by MK to, among other things, I hope, update the modified flag from outside the FREDDoc class."
 */
extern CFREDDoc *FREDDoc_ptr;

/**
 * @brief Sets the modified flag of the FREDDoc object
 */
void set_modified(BOOL arg = TRUE);

/////////////////////////////////////////////////////////////////////////////

#endif
