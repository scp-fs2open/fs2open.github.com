/*
 * wmcgui.cpp
 * created by WMCoolmon
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */



#include "lab/wmcgui.h"
#include "graphics/2d.h"
#include "hud/hudbrackets.h"
#include "parse/parselo.h"
#include "globalincs/linklist.h"
#include "io/key.h"
#include "freespace2/freespace.h"
#include "localization/localize.h"

//Gobals
GUISystem GUI_system;

//*****************************ClassInfoEntry*******************************
ClassInfoEntry::ClassInfoEntry()
{
	CIEType = CIE_NONE;

	for (uint i = 0; i < CIE_NUM_HANDLES; i++) {
		IMG_HANDLE_SET_INVALID(Handles[i].Image);
	}

	Coords[0] = Coords[1] = INT_MAX;
}
ClassInfoEntry::~ClassInfoEntry()
{
	//Unload image handles
	if (CIEType == CIE_IMAGE_NMCSD || CIEType == CIE_IMAGE || CIEType == CIE_IMAGE_BORDER) {
		int i = 0;
		int num = sizeof(Handle)/sizeof(int);
		for (; i < num; i++) {
			if (IMG_HANDLE_IS_VALID(Handles[i].Image)) {
				IMG_UNLOAD(Handles[i].Image);
			}
		}
	}
}

bool ObjectClassInfoEntry::Parse()
{
	//Do we actually have a tag?
	//This is hackish, but it should get the job done
	if (check_for_string("<") && !check_for_string("</")) {
		char buf[NAME_LENGTH];
		char buf2[NAME_LENGTH+3];	//for the end tag and name buffer

		Assert( sizeof(buf2) >= (sizeof(buf) + 3) );

		//Find the name of the thing we're parsing
		parse_advance(1);
		stuff_string(buf, F_NAME, sizeof(buf), ">");
		parse_advance(1);	//skip the end ">"

		if (optional_string("+Name:")) {
			stuff_string(buf2, F_NAME, sizeof(buf2));
			Name = buf2;
		}
		if (optional_string("+Coords:")) {
			stuff_int_list(Coords, 2, RAW_INTEGER_TYPE);
		}
		if (optional_string("+Size:")) {
			stuff_int_list(&Coords[2], 2, RAW_INTEGER_TYPE);
		}

		//This is where you add specific classes to parse.
		//Follow the current classes' example, this basically involves
		//a !stricmp(), a resize() (to the number of entries) and multiple Entries[ID].Parse()
		
		//=================================================
		if (!stricmp(buf, "Window")) {
			Object = GT_WINDOW;
			Entries.resize(WCI_NUM_ENTRIES);

			//Fill it out
			Entries[WCI_CAPTION].Parse("Caption", CIE_IMAGE_NMCSD);
			Entries[WCI_HIDE].Parse("Hider", CIE_IMAGE_NMCSD);
			Entries[WCI_CLOSE].Parse("Closer", CIE_IMAGE_NMCSD);
			Entries[WCI_BODY].Parse("Body", CIE_IMAGE);
			Entries[WCI_BORDER].Parse("Border", CIE_IMAGE_BORDER);
		} else if (!stricmp(buf, "Button")) {
			Object = GT_BUTTON;
			Entries.resize(BCI_NUM_ENTRIES);

			//Fill it out
			Entries[BCI_BUTTON].Parse("Button", CIE_IMAGE_NMCSD);
		}

		//=================================================

		//Make sure that the next token isn't an end tag, but
		//is a tag
		ObjectClassInfoEntry temp_ocie;
		while (temp_ocie.Parse()) {
			Subentries.push_back(temp_ocie);
			temp_ocie = ObjectClassInfoEntry();
		}

		//We MUST have the end tag
		strcpy_s(buf2, "</");
		strcat_s(buf2, buf);
		strcat_s(buf2, ">");

		required_string(buf2);
		return true;
	}

	return false;
}

bool ScreenClassInfoEntry::Parse()
{
	if (check_for_string("#") && !check_for_string("#End")) {
		char buf[NAME_LENGTH];

		//Find the name of the thing we're parsing
		parse_advance(1);
		stuff_string(buf, F_NAME, sizeof(buf));
		Name = buf;

		//Parse all objects for this screen
		ObjectClassInfoEntry temp_ocie;
		while (temp_ocie.Parse()) {
			Entries.push_back(temp_ocie);
			temp_ocie = ObjectClassInfoEntry();
		}

		return true;
	}

	return false;
}

void GUISystem::ParseClassInfo(char* filename)
{
	int rval;

	if (ClassInfoParsed) {
		Warning(LOCATION, "Class info is being parsed twice");
		DestroyClassInfo();
	}

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("WMCGUI: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		lcl_ext_close();
		return;
	}

	read_file_text(filename);
	reset_parse();
	ScreenClassInfo.Parse();

	bool flag;
	do {
		ScreenClassInfoEntry* sciep = new ScreenClassInfoEntry;
		flag = sciep->Parse();
		if (flag) {
			list_append(&ScreenClassInfo, sciep);
		}
	} while(flag);

	// close localization
	lcl_ext_close();

	ClassInfoParsed = true;
}

void ClassInfoEntry::Parse(char* tag, int in_type)
{
	char buf[MAX_FILENAME_LEN];
	strcpy_s(buf, "+");
	strcat_s(buf, tag);
	if (in_type != CIE_IMAGE_BORDER) {
		strcat_s(buf, ":");
	}

	if(optional_string(buf)) {
		CIEType = in_type;
		if (in_type == CIE_IMAGE || in_type == CIE_IMAGE_NMCSD) {
			int num_frames;
			stuff_string(buf, F_NAME, sizeof(buf));
			Handles[CIE_HANDLE_N].Image = IMG_LOAD_ANIM(buf, &num_frames, NULL);
			if (IMG_HANDLE_IS_VALID(Handles[CIE_HANDLE_N].Image) && num_frames) {
				if (num_frames > 1) {
					IMG_HANDLE_SET_FRAME(Handles[CIE_HANDLE_N].Image, Handles[CIE_HANDLE_M].Image, 1);
				}
				if (num_frames > 2) {
					IMG_HANDLE_SET_FRAME(Handles[CIE_HANDLE_N].Image, Handles[CIE_HANDLE_C].Image, 2);
				}
				if (num_frames > 3) {
					IMG_HANDLE_SET_FRAME(Handles[CIE_HANDLE_N].Image, Handles[CIE_HANDLE_S].Image, 3);
				}
				if (num_frames > 4) {
					IMG_HANDLE_SET_FRAME(Handles[CIE_HANDLE_N].Image, Handles[CIE_HANDLE_D].Image, 4);
				}
				IMG_HANDLE_SET_FRAME(Handles[CIE_HANDLE_N].Image, Handles[CIE_HANDLE_N].Image, 0);
			} else {
				Handles[CIE_HANDLE_N].Image = IMG_LOAD(buf);
			}
			if (in_type == CIE_IMAGE_NMCSD) {
				if (optional_string("+Mouseover:")) {
					stuff_string(buf, F_NAME, sizeof(buf));
					Handles[CIE_HANDLE_M].Image = IMG_LOAD(buf);
				}
				if (optional_string("+Clicked:")) {
					stuff_string(buf, F_NAME, sizeof(buf));
					Handles[CIE_HANDLE_C].Image = IMG_LOAD(buf);
				}
				if (optional_string("+Selected:")) {
					stuff_string(buf, F_NAME, sizeof(buf));
					Handles[CIE_HANDLE_S].Image = IMG_LOAD(buf);
				}
				if (optional_string("+Disabled:")) {
					stuff_string(buf, F_NAME, sizeof(buf));
					Handles[CIE_HANDLE_D].Image = IMG_LOAD(buf);
				}
			}
			if(optional_string("+Coords:")) {
				stuff_int_list(Coords, 2, RAW_INTEGER_TYPE);
			}
		} else if (in_type == CIE_IMAGE_BORDER) {
			if (optional_string("+Top Left:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_TL].Image = IMG_LOAD(buf);
			}
			if (optional_string("+Top Mid:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_TM].Image = IMG_LOAD(buf);
			}
			if (optional_string("+Top Right:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_TR].Image = IMG_LOAD(buf);
			}
			if (optional_string("+Mid Left:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_ML].Image = IMG_LOAD(buf);
			}
			if (optional_string("+Mid Right:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_MR].Image = IMG_LOAD(buf);
			}
			if (optional_string("+Bottom left:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_BL].Image = IMG_LOAD(buf);
			}
			if (optional_string("+Bottom Mid:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_BM].Image = IMG_LOAD(buf);
			}
			if (optional_string("+Bottom Right:")) {
				stuff_string(buf, F_NAME, sizeof(buf));
				Handles[CIE_HANDLE_BR].Image = IMG_LOAD(buf);
			}
		} else if (in_type == CIE_COORDS) {
			stuff_int_list(Coords, 2, RAW_INTEGER_TYPE);
		}
	}

#ifndef NDEBUG
	//Do a little extra checking in debug mode
	//This makes sure the skinnger knows if they did something bad
	if (CIEType == CIE_IMAGE || CIEType == CIE_IMAGE_NMCSD) {
		int w,h,cw,ch;
		IMG_INFO(Handles[0].Image, &w, &h);
		for (uint i = 1; i < CIE_NUM_HANDLES; i++) {
			if (IMG_HANDLE_IS_VALID(Handles[i].Image)) {
				IMG_INFO(Handles[i].Image, &cw, &ch);
				if (cw != w || ch != h) {
					Warning(LOCATION, "Grouped image size unequal; Handle number %d under $%s: has a different size than base image type", i, tag);
				}
			}
		}
	}
#endif
}

int ClassInfoEntry::GetCoords(int *x, int *y)
{
	int rval = CIE_GC_NONE_SET;
	if (Coords[0]!=INT_MAX && x!=NULL) {
		*x=Coords[0];
		rval |= CIE_GC_X_SET;
	}
	if (Coords[1]!=INT_MAX && x!=NULL) {
		*y=Coords[1];
		rval |= CIE_GC_Y_SET;
	}

	return rval;
}

int ObjectClassInfoEntry::GetImageHandle(int id, int handle_num)
{
	return Entries[id].GetImageHandle(handle_num);
}

int ObjectClassInfoEntry::GetCoords(int id, int *x, int *y)
{
	return Entries[id].GetCoords(x, y);
}

int ObjectClassInfoEntry::GetObjectCoords(int *x, int *y, int *w, int *h)
{
	int rval = CIE_GC_NONE_SET;
	if (Coords[0] != INT_MAX && x != NULL) {
		*x = Coords[0];
		rval |= CIE_GC_X_SET;
	}
	if (Coords[1] != INT_MAX && y != NULL) {
		*y = Coords[1];
		rval |= CIE_GC_Y_SET;
	}
	if (Coords[2] != INT_MAX && w != NULL) {
		*w = Coords[2];
		rval |= CIE_GC_W_SET;
	}
	if (Coords[3] != INT_MAX && h != NULL) {
		*h = Coords[3];
		rval |= CIE_GC_H_SET;
	}

	return rval;
}
//*****************************GUIScreen*******************************
GUIScreen::GUIScreen(SCP_string in_Name)
{
	//Set the name
	Name = in_Name;

	//Setup the parent system
	//This can actually be null; but you must attach a screen to a
	//system for it to be functional.
	//this may allow for saving of screens in the future. Incredibly useful for HUDs.
	OwnerSystem = NULL;

	//Get class info
	if (OwnerSystem != NULL) {
		OwnerSystem->PushScreen(this);
		ScreenClassInfo = OwnerSystem->GetScreenClassInfo(Name);
	} else {
		ScreenClassInfo = NULL;
	}
}

GUIScreen::~GUIScreen()
{
	GUIObject* cgp = (GUIObject*)GET_FIRST(&Guiobjects);
	GUIObject* cgp_next;
	for (; cgp != END_OF_LIST(&Guiobjects); cgp = cgp_next) {
		cgp_next = (GUIObject*)GET_NEXT(cgp);
		delete cgp;
	}
}

ObjectClassInfoEntry *GUIScreen::GetObjectClassInfo(GUIObject *cgp)
{
	size_t len;
	uint i;

	if (cgp->Parent != NULL && cgp->Parent->InfoEntry != NULL) {
		len = cgp->Parent->InfoEntry->Subentries.size();

		for (i = 0; i < len; i++) {
			if (cgp->Parent->InfoEntry->Subentries[i].Name == cgp->Name) {
				return &cgp->Parent->InfoEntry->Subentries[i];
			}
		}

		for (i = 0; i < len; i++) {
			if ( cgp->Parent->InfoEntry->Subentries[i].Name.size() == 0 
			&& cgp->Parent->InfoEntry->Subentries[i].Object == cgp->Type ) {
				return &cgp->Parent->InfoEntry->Subentries[i];
			}
		}
	} else if (cgp->Parent == NULL && ScreenClassInfo != NULL) {
		len = ScreenClassInfo->Entries.size();
		for (i = 0; i < len; i++) {
			if (ScreenClassInfo->Entries[i].Name == cgp->Name) {
				return &ScreenClassInfo->Entries[i];
			}
		}

		for (i = 0; i < len; i++) {
			if( ScreenClassInfo->Entries[i].Name.size() == 0 && 
				ScreenClassInfo->Entries[i].Object == cgp->Type ) {
				return &ScreenClassInfo->Entries[i];
			}
		}
	}

	//Generic templates - ie if you want every single OK button to look the same,
	//or every single window to look the same.
	if (OwnerSystem != NULL) {
		len = OwnerSystem->GetClassInfo()->Entries.size();

		for (i = 0; i < len; i++) {
			if (OwnerSystem->GetClassInfo()->Entries[i].Name == cgp->Name) {
				return &OwnerSystem->GetClassInfo()->Entries[i];
			}
		}

		for (i = 0; i < len; i++) {
			if( OwnerSystem->GetClassInfo()->Entries[i].Name.size() == 0 && 
				OwnerSystem->GetClassInfo()->Entries[i].Object == cgp->Type) {
				return &OwnerSystem->GetClassInfo()->Entries[i];
			}
		}
	}

	return NULL;
}

//Adds objects to a screen
//IMPORTANT: Options added with the same name as
//older objects will make this function return
//a pointer to the original object with that name
GUIObject* GUIScreen::Add(GUIObject* new_gauge)
{
	if(new_gauge == NULL) {
		return NULL;
	}

	Assert(this != NULL);

	//First - do we have anything with the same name.
	for (GUIObject *tgp = (GUIObject*) GET_FIRST(&Guiobjects); tgp != END_OF_LIST(&Guiobjects); tgp = (GUIObject*) GET_NEXT(tgp)) {
		if (tgp->Name == new_gauge->Name) {
			//Get rid of the new gauge
			//We don't want it; breaks skinning
			delete new_gauge;

			if (tgp->Type == new_gauge->Type) {
				//If the type of the existing object is the same
				//as the new one, we can safely pass this one off
				delete new_gauge;
				return tgp;
			} else {
				//This is icky; we might cast a pointer after this.
				//So return NULL with a warning
				Warning(LOCATION, "Attempt to create another object with name '%s'; new object type was %d, existing object type was %d. This may cause null pointer issues.", tgp->Name.c_str(), new_gauge->Type, tgp->Type);
				delete new_gauge;
				return NULL;
			}
		}
	}

	//Add to the end of the list
	list_append(&Guiobjects, new_gauge);
	new_gauge->OwnerSystem = OwnerSystem;
	new_gauge->OwnerScreen = this;

	//For skinning
	new_gauge->SetCIPointer();
	//Set position and stuff
	new_gauge->GetOIECoords(&new_gauge->Coords[0], &new_gauge->Coords[1], &new_gauge->Coords[2], &new_gauge->Coords[3]);
	//In case we need to resize
	new_gauge->OnRefreshSize();

	return new_gauge;
}

void GUIScreen::DeleteObject(GUIObject* dgp)
{
	DeletionCache.push_back(dgp);
}

int GUIScreen::OnFrame(float frametime, bool doevents)
{
	GUIObject* cgp;
	GUIObject* cgp_prev;
	bool SomethingPressed = false;

	if (NOT_EMPTY(&Guiobjects) && doevents) {
		//Note that children WILL be changing this variable as they go along,
		//as they use up statuses.
		int status = OwnerSystem->GetStatus();

		//Pass the status on
		cgp = (GUIObject*)GET_LAST(&Guiobjects);
		cgp->Status |= (status & GST_KEYBOARD_STATUS);
		for (; cgp != END_OF_LIST(&Guiobjects); cgp = cgp_prev) {
			//In case an object deletes itself
			cgp_prev = (GUIObject*)GET_PREV(cgp);
			cgp->LastStatus = cgp->Status;
			cgp->Status = 0;

			//If we are moving something, nothing else can get click events
			if (OwnerSystem->GetGraspedObject() == NULL) {
				if (OwnerSystem->GetMouseX() >= cgp->Coords[0]
					&& OwnerSystem->GetMouseX() <= cgp->Coords[2]
					&& OwnerSystem->GetMouseY() >= cgp->Coords[1]
					&& OwnerSystem->GetMouseY() <= cgp->Coords[3]) {
					cgp->Status |= (status & GST_MOUSE_STATUS);
					if (status & GST_MOUSE_PRESS) {
						SomethingPressed = true;
					}
				}
			}

			cgp->OnFrame(frametime, &status);
		}
	}

	for (size_t i = DeletionCache.size(); i > 0; i--) {
		DeletionCache.pop_back();
	}

	// save zbuffer so that we can reset it after drawing (FIXME: this could probably be done better)
	int saved_zbuf = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	//Draw now. This prevents problems from an object deleting itself or moving around in the list
	cgp = (GUIObject*)GET_FIRST(&Guiobjects);

	while (cgp != END_OF_LIST(&Guiobjects)) {
		if ( !doevents ) {
			cgp->Status = 0;
		}

		cgp->OnDraw(frametime);

		cgp = (GUIObject*)GET_NEXT(cgp);
	}

	// reset zbuffer to saved value
	gr_zbuffer_set(saved_zbuf);

	if(SomethingPressed) {
		return GSOF_SOMETHINGPRESSED;
	} else {
		return GSOF_NOTHINGPRESSED;
	}
}

//*****************************GUISystem*******************************
GUISystem::GUISystem()
{
	GraspedGuiobject=ActiveObject=NULL;
	Status=LastStatus=0;
	MouseX = MouseY = 0;
	ClassInfoParsed = false;
}

GUISystem::~GUISystem()
{
	GUIScreen* csp = (GUIScreen*)GET_FIRST(&Screens);
	GUIScreen* csp_next;
	for (; csp != END_OF_LIST(&Screens); csp = csp_next) {
		csp_next = (GUIScreen*)GET_NEXT(csp);
		delete csp;
	}
	DestroyClassInfo();
}

GUIScreen *GUISystem::PushScreen(GUIScreen *csp)
{
	Assert(csp != NULL);
	list_append(&Screens, csp);
	csp->OwnerSystem = this;
	csp->ScreenClassInfo = GetScreenClassInfo(csp->Name);
	return csp;
}

//Removes a screen from a GUISystem
//Handy if you want to save window movements and such;
//just call this and it'll take care of everything
void GUISystem::PullScreen(GUIScreen *in_screen)
{
	GUIScreen *csp_next;
	for (GUIScreen *csp = (GUIScreen*)GET_FIRST(&Screens); csp != END_OF_LIST(&Screens); csp = csp_next) {
		csp_next = (GUIScreen*)GET_NEXT(csp);
		if (csp == in_screen) {
			csp->prev->next = csp->next;
			csp->next->prev = csp->prev;
			csp->next = csp;
			csp->prev = csp;
			csp->OwnerSystem = NULL;
		}
	}
}

ScreenClassInfoEntry *GUISystem::GetScreenClassInfo(const SCP_string & screen_name)
{
	ScreenClassInfoEntry *sciep;
	for (sciep = (ScreenClassInfoEntry*)GET_FIRST(&ScreenClassInfo); sciep != END_OF_LIST(&ScreenClassInfo); sciep = (ScreenClassInfoEntry*)GET_NEXT(sciep)) {
		if (sciep->GetName() == screen_name) {
			return sciep;
		}
	}

	return NULL;
}

int GUISystem::OnFrame(float frametime, bool doevents, bool clearandflip)
{
	//Set the global status variables for this frame
	LastStatus = Status;
	Status = GST_MOUSE_OVER;

	bool something_pressed = false;
	int unused_queue;
	if (clearandflip) {
		gr_clear();
	}

	if (NOT_EMPTY(&Screens)) {
		if (doevents) {
			KeyPressed = game_check_key();

			//Add keyboard event stuff
			if (KeyPressed) {
				Status |= GST_KEYBOARD_KEYPRESS;

				if (KeyPressed & KEY_CTRLED) {
					Status |= GST_KEYBOARD_CTRL;
				}
				if (KeyPressed & KEY_ALTED) {
					Status |= GST_KEYBOARD_ALT;
				}
				if (KeyPressed & KEY_SHIFTED) {
					Status |= GST_KEYBOARD_SHIFT;
				}
			}

			//Add mouse event stuff
			if (mouse_down(MOUSE_LEFT_BUTTON)) {
				Status |= GST_MOUSE_LEFT_BUTTON;
			} else if (mouse_down(MOUSE_MIDDLE_BUTTON)) {
				Status |= GST_MOUSE_MIDDLE_BUTTON;
			} else if (mouse_down(MOUSE_RIGHT_BUTTON)) {
				Status |= GST_MOUSE_RIGHT_BUTTON;
			}

			//Now that we are done setting status, add all that stuff to the unused queue
			//Children will be changing this function as they do stuff with statuses
			unused_queue = Status;

			mouse_get_pos(&MouseX, &MouseY);

			//Handle any grasped object (we are in the process of moving this)
			if (GraspedGuiobject != NULL) {
				if (Status & GraspingButton) {
					something_pressed = true;
					GraspedGuiobject->SetPosition(MouseX - GraspedDiff[0], MouseY - GraspedDiff[1]);
					GraspedGuiobject->Status = (Status & GST_MOUSE_STATUS);
				} else {
					GraspedGuiobject = NULL;
				}
			}
		}

		GUIScreen *csp_prev;	//so screens can delete themselves
		for (GUIScreen* csp = (GUIScreen*)GET_LAST(&Screens); csp != END_OF_LIST(&Screens); csp = csp_prev) {
			csp_prev = (GUIScreen*)GET_PREV(csp);
			if (csp->OnFrame(frametime, doevents) == GSOF_SOMETHINGPRESSED) {
				something_pressed = true;
			}
		}
	}

	if (clearandflip) {
		gr_flip();
	}

	if (something_pressed) {
		return GSOF_SOMETHINGPRESSED;
	} else {
		return GSOF_NOTHINGPRESSED;
	}
}

void GUISystem::SetActiveObject(GUIObject *cgp)
{
	if (cgp != NULL) {
		ActiveObject = cgp;
		//Move everything to the end of the list
		//This way, it gets precedence
		while (cgp->Parent != NULL) {
			list_move_append(&cgp->Parent->Children, cgp);
			cgp = cgp->Parent;
		}

		//Eventually, we make this last guy the last object in Guiobjects
		list_move_append(&cgp->OwnerScreen->Guiobjects, cgp);
	}
}

void GUISystem::SetGraspedObject(GUIObject *cgp, int button)
{
	//Protect thyself!
	if (cgp == NULL) {
		return;
	}

	GraspedGuiobject = cgp;
	GraspingButton = button;
	GraspedDiff[0] = MouseX - cgp->Coords[0];
	GraspedDiff[1] = MouseY - cgp->Coords[1];
}

void GUISystem::DestroyClassInfo()
{
	ScreenClassInfoEntry *sciep, *next_sciep;
	for (sciep = (ScreenClassInfoEntry*)GET_FIRST(&ScreenClassInfo); sciep != END_OF_LIST(&ScreenClassInfo); sciep = next_sciep) {
		next_sciep = (ScreenClassInfoEntry*)GET_NEXT(sciep);
		delete sciep;
	}
	ClassInfoParsed = false;
}

//*****************************GUIObject*******************************
GUIObject::GUIObject(SCP_string in_Name, int x_coord, int y_coord, int x_width, int y_height, int in_style)
{
	//General stuff
	LastStatus = Status = 0;
	OwnerSystem = NULL;
	OwnerScreen = NULL;
	Parent = NULL;
	CloseFunction = NULL;

	list_init(this);
	list_init(&Children);

	//These would make no sense
	//x_coord and y_coord of < 0 mean to let the parent handle placement.
	if (x_width == 0 || y_height == 0) {
		return;
	}
	
	//No! Bad!
	if (in_Name.length() < 1) {
		return;
	}

	Name = in_Name;
	Coords[0] = x_coord;
	Coords[1] = y_coord;
	Coords[2] = x_coord + x_width;
	Coords[3] = y_coord + y_height;

	Style = in_style;

	if (x_width > 0) {
		Style |= GS_NOAUTORESIZEX;
	}
	if (y_height > 0) {
		Style |= GS_NOAUTORESIZEY;
	}

	Type = GT_NONE;
	InfoEntry = NULL;
}

GUIObject::~GUIObject()
{
	if (CloseFunction != NULL) {
		CloseFunction(this);
	}

	DeleteChildren();

	// Set these properly
	next->prev = prev;
	prev->next = next;
}

void GUIObject::DeleteChildren(GUIObject* exception)
{
	if ( EMPTY(&Children) ) {
		return;
	}

	GUIObject* cgp = (GUIObject*)GET_FIRST(&Children);
	GUIObject* cgp_temp;

	while (cgp != END_OF_LIST(&Children)) {
		cgp_temp = cgp;
		cgp = (GUIObject*)GET_NEXT(cgp);
		delete cgp_temp;
	}

	list_init(&Children);
}

GUIObject* GUIObject::AddChildInternal(GUIObject *cgp)
{
	if (cgp == NULL) {
		return NULL;
	}

	cgp->Style |= GS_INTERNALCHILD;

	//Add to end of child list
	list_append(&Children, cgp);

	cgp->Parent = this;
	cgp->OwnerSystem = OwnerSystem;
	cgp->OwnerScreen = OwnerScreen;

	//Update coordinates (Should be relative x/y and width/height) to absolute coordinates
	cgp->Coords[0] += Coords[0];
	cgp->Coords[1] += Coords[1];
	cgp->Coords[2] += Coords[0];
	cgp->Coords[3] += Coords[1];

	//For skinning
	cgp->SetCIPointer();
	//Check position
	cgp->GetOIECoords(&cgp->Coords[0], &cgp->Coords[1], &cgp->Coords[2], &cgp->Coords[3]);
	//In case we need to resize
	cgp->OnRefreshSize();
	
	return cgp;
}

GUIObject* GUIObject::AddChild(GUIObject* cgp)
{
	if (cgp == NULL) {
		return NULL;
	}
	
	//AddInternalChild must be used
	if (cgp->Style & GS_INTERNALCHILD) {
		return NULL;
	}

	//Add to end of child list
	list_append(&Children, cgp);

	cgp->Parent = this;
	cgp->OwnerSystem = OwnerSystem;
	cgp->OwnerScreen = OwnerScreen;

	// Update coordinates (Should be relative x/y and width/height) to absolute coordinates
	cgp->Coords[0] += ChildCoords[0];
	cgp->Coords[1] += ChildCoords[1];
	cgp->Coords[2] += ChildCoords[0];
	cgp->Coords[3] += ChildCoords[1];

	//For skinning
	cgp->SetCIPointer();
	//Check position
	cgp->GetOIECoords(&cgp->Coords[0], &cgp->Coords[1], &cgp->Coords[2], &cgp->Coords[3]);
	//In case we need to resize
	cgp->OnRefreshSize();

	return cgp;
}

void GUIObject::Delete()
{
	if (OwnerScreen != NULL) {
		OwnerScreen->DeleteObject(this);
	} else {
		delete this;
	}
}

void GUIObject::OnDraw(float frametime)
{
	DoDraw(frametime);
	if (!(Style & GS_HIDDEN)) {
		GUIObject *cgp_prev;
		for (GUIObject* cgp = (GUIObject*)GET_LAST(&Children); cgp != END_OF_LIST(&Children); cgp = cgp_prev) {
			cgp_prev = (GUIObject*)GET_PREV(cgp);
			cgp->OnDraw(frametime);
		}
	}
}

int GUIObject::OnFrame(float frametime, int *unused_queue)
{
	int rval = OF_TRUE;

	GUIObject *cgp_prev;	//Elements will move themselves to the end of the list if they become active
	GUIObject *cgp = (GUIObject*)GET_LAST(&Children);

	for ( ; (cgp != NULL) && (cgp != END_OF_LIST(&Children)); cgp = cgp_prev) {
		cgp_prev = (GUIObject*)GET_PREV(cgp);
		cgp->LastStatus = cgp->Status;
		cgp->Status = 0;

		if (Status & GST_MOUSE_OVER) {
			if (OwnerSystem->GetMouseX() >= cgp->Coords[0]
				&& OwnerSystem->GetMouseX() <= cgp->Coords[2]
				&& OwnerSystem->GetMouseY() >= cgp->Coords[1]
				&& OwnerSystem->GetMouseY() <= cgp->Coords[3]) {
				cgp->Status |= (Status & GST_MOUSE_STATUS);
			}
		}
		if (cgp == GET_LAST(&Children)) {
			cgp->Status |= (Status & GST_KEYBOARD_STATUS);
		}
		cgp->OnFrame(frametime, &Status);
	}

	if (Status) {
		//MOUSE OVER
		if (Status & GST_MOUSE_OVER) {
			rval = DoMouseOver(frametime);

			if (rval == OF_TRUE) {
				(*unused_queue) &= ~GST_MOUSE_OVER;
			}
		}

		//MOUSE DOWN
		if (Status & GST_MOUSE_PRESS) {
			rval = DoMouseDown(frametime);

			if (rval == OF_TRUE) {
				(*unused_queue) &= ~(GST_MOUSE_LEFT_BUTTON | GST_MOUSE_RIGHT_BUTTON | GST_MOUSE_MIDDLE_BUTTON);
			}
		}

		//MOUSE UP
		if ((LastStatus & GST_MOUSE_PRESS) && !(OwnerSystem->GetStatus() & GST_MOUSE_PRESS)) {
			rval = DoMouseUp(frametime);

			if (rval == OF_TRUE) {
				(*unused_queue) &= ~(GST_MOUSE_LEFT_BUTTON | GST_MOUSE_RIGHT_BUTTON | GST_MOUSE_MIDDLE_BUTTON);
			}
		}

		//KEY STATE
		if ((Status & GST_KEYBOARD_CTRL) || (Status & GST_KEYBOARD_ALT) || (Status & GST_KEYBOARD_SHIFT)) {
			rval = DoKeyState(frametime);

			if (rval == OF_TRUE) {
				(*unused_queue) &= ~(GST_KEYBOARD_CTRL | GST_KEYBOARD_ALT | GST_KEYBOARD_SHIFT);
			}
		}

		//KEYPRESS
		if (Status & GST_KEYBOARD_KEYPRESS) {
			rval = DoKeyPress(frametime);

			if (rval == OF_TRUE) {
				(*unused_queue) &= ~GST_KEYBOARD_KEYPRESS;
			}
		}
	}

	if (!(Status & GST_MOUSE_OVER) && (LastStatus & GST_MOUSE_OVER)) {
		rval = DoMouseOut(frametime);
	}

	return DoFrame(frametime);
}

void GUIObject::OnMove(int dx, int dy)
{
	Coords[0]+=dx;
	Coords[1]+=dy;
	Coords[2]+=dx;
	Coords[3]+=dy;
	ChildCoords[0]+=dx;
	ChildCoords[1]+=dy;
	ChildCoords[2]+=dx;
	ChildCoords[3]+=dy;

	for (GUIObject *cgp = (GUIObject*)GET_FIRST(&Children); cgp != END_OF_LIST(&Children); cgp = (GUIObject*)GET_NEXT(cgp)) {
		cgp->OnMove(dx, dy);
	}

	DoMove(dx, dy);
}

int GUIObject::GetCIECoords(int id, int *x, int *y)
{
	if (InfoEntry!=NULL) {
		int rv = InfoEntry->GetCoords(id, x, y);
		if (rv & CIE_GC_X_SET) {
			if (*x < 0) {
				*x += Coords[2];
			} else {
				*x += Coords[0];
			}
		}
		if (rv & CIE_GC_Y_SET) {
			if (*y < 0) {
				*y += Coords[3];
			} else {
				*y += Coords[1];
			}
		}

		return rv;
	}

	return CIE_GC_NONE_SET;
}

int GUIObject::GetOIECoords(int *x1, int *y1, int *x2, int *y2)
{
	if (InfoEntry != NULL) {
		int rv = InfoEntry->GetObjectCoords(x1, y1, x2, y2);
		if (Parent != NULL) {
			if (rv & CIE_GC_X_SET) {
				if (*x1 < 0) {
					*x1 += Parent->ChildCoords[2];
				} else {
					*x1 += Parent->ChildCoords[0];
				}
			}
			if (rv & CIE_GC_Y_SET) {
				if (*y1 < 0) {
					*y1 += Parent->ChildCoords[3];
				} else {
					*y1 += Parent->ChildCoords[1];
				}
			}
		} else {
			if (rv & CIE_GC_X_SET) {
				if (*x1 < 0) {
					*x1 += gr_screen.clip_right;
				} else {
					*x1 += gr_screen.clip_left;
				}
			}
			if (rv & CIE_GC_Y_SET) {
				if (*y1 < 0) {
					*y1 += gr_screen.clip_bottom;
				} else {
					*y1 += gr_screen.clip_top;
				}
			}
		}
		if (rv & CIE_GC_W_SET) {
			*x2 = *x1 + *x2;
			Style |= GS_NOAUTORESIZEX;
		}
		if (rv & CIE_GC_H_SET) {
			*y2 = *y1 + *y2;
			Style |= GS_NOAUTORESIZEY;
		}

		return rv;
	}

	return CIE_GC_NONE_SET;
}

//Call this after an object's type is set to enable the GUIObject CIE functions
void GUIObject::SetCIPointer()
{
	if (OwnerScreen) {
		InfoEntry=OwnerScreen->GetObjectClassInfo(this);
	}
}

void GUIObject::SetPosition(int x, int y)
{
	int dx, dy;
	dx = x - Coords[0];
	dy = y - Coords[1];

	if (dx == 0 && dy == 0) {
		return;
	}

	if (Parent != NULL) {
		//Don't let stuff get moved outside client window
		if (Coords[2] + dx > Parent->Coords[2]) {
			dx = Parent->Coords[2] - Coords[2];
		}
		if (Coords[3] + dy > gr_screen.clip_bottom) {
			dy = Parent->Coords[3] - Coords[3];
		}

		//Check these last, so we're sure we can move stuff around still
		if (Coords[0] + dx < gr_screen.clip_left) {
			dx = Parent->Coords[0] - Coords[0];
		}
		if (Coords[1] + dy < gr_screen.clip_top) {
			dy = Parent->Coords[1] - Coords[1];
		}
	}

	OnMove(dx, dy);
}

//*****************************Window*******************************
int Window::DoRefreshSize()
{
	float num;
	int w=0, h=0;
	//Top left, top right, bottom left, bottom right
	int CornerWidths[4];

	//Determine left border's width
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML), &BorderSizes[0], NULL);
	} else {
		BorderSizes[0] = W_BORDERHEIGHT;
	}

	//Determine right border's width
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR), &BorderSizes[2], NULL);
	} else {
		BorderSizes[2] = W_BORDERWIDTH;
	}

	//Determine top border height
	bool custom_top = true;
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL), NULL, &BorderSizes[1]);
	} else if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR), NULL, &BorderSizes[1]);
	} else if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM), NULL,  &BorderSizes[1]);
	} else {
		custom_top = false;
		BorderSizes[1] = W_BORDERHEIGHT;
	}

	//Determine bottom border height
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), NULL, &BorderSizes[3]);
	} else if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR), NULL, &BorderSizes[3]);
	} else if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM), NULL,  &BorderSizes[3]);
	} else {
		BorderSizes[3] = W_BORDERHEIGHT;
	}


	//Determine corner sizes
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), &CornerWidths[0], NULL);
	} else {
		CornerWidths[0] = BorderSizes[0];
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR))){ 
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR), &CornerWidths[1], NULL);
	} else {
		CornerWidths[1] = BorderSizes[2];
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), &CornerWidths[2], NULL);
	} else {
		CornerWidths[2] = BorderSizes[0];
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), &CornerWidths[3], NULL);
	} else {
		CornerWidths[3] = BorderSizes[2];
	}


	//Do child stuff
	ChildCoords[0] = Coords[0] + BorderSizes[0];
	if (custom_top || (Style & WS_NOTITLEBAR)) {
		ChildCoords[1] = Coords[1] + BorderSizes[1];	//Set earlier on
	} else {
		ChildCoords[1] = Coords[1] + gr_get_font_height() + 5 + 3*W_BORDERHEIGHT;
	}
	ChildCoords[2] = Coords[2] - BorderSizes[2];
	ChildCoords[3] = Coords[3] - BorderSizes[3];

	if (!(Style & GS_NOAUTORESIZEX)) {
		ChildCoords[2] = Coords[2] = 0;
	}
	if (!(Style & GS_NOAUTORESIZEY)) {
		ChildCoords[3] = Coords[3] = 0;
	}

	for (GUIObject *cgp = (GUIObject*)GET_FIRST(&Children); cgp != END_OF_LIST(&Children); cgp = (GUIObject*)GET_NEXT(cgp)) {
		//SetPosition children around so they're within the upper-left corner of dialog
		if (cgp->Coords[0] < ChildCoords[0]) {
			cgp->SetPosition(ChildCoords[0], cgp->Coords[1] + BorderSizes[0]);
		}
		if (cgp->Coords[1] < ChildCoords[1]) {
			cgp->SetPosition(cgp->Coords[0], Coords[1] + BorderSizes[1]);
		}

		//Resize window to fit children
		if (cgp->Coords[2] > ChildCoords[2] && !(Style & GS_NOAUTORESIZEX)) {
			ChildCoords[2] = cgp->Coords[2];
			Coords[2] = cgp->Coords[2] + BorderSizes[2];
		}
		if (cgp->Coords[3] > ChildCoords[3] && !(Style & GS_NOAUTORESIZEY)) {
			ChildCoords[3] = cgp->Coords[3];
			Coords[3] = cgp->Coords[3] + BorderSizes[3];
		}
	}

	//Find caption coordinates
	if (!(Style & WS_NOTITLEBAR)) {
		int close_w = 0,close_h = 0;
		int hide_w = 0, hide_h = 0;

		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_CLOSE))) {
			IMG_INFO(GetCIEImageHandle(WCI_CLOSE), &close_w, &close_h);
		} else {
			gr_get_string_size(&close_w, &close_h, "X");
		}

		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_HIDE))) {
			IMG_INFO(GetCIEImageHandle(WCI_HIDE), &hide_w, &hide_h);
		} else {
			gr_get_string_size(&hide_w, &hide_h, "-");
		}
		
		int caption_min_size;
		if (Caption.size() > 0) {
			gr_get_string_size(&w, &h, (char *)Caption.c_str());
			caption_min_size = w + close_w + hide_w + 5;
		} else {
			caption_min_size = close_w + hide_w;
		}

		if ((Coords[2] - Coords[0]) < (caption_min_size)) {
				Coords[2] = Coords[0] + caption_min_size + BorderSizes[0] + BorderSizes[2];
		}

		if (Caption.size() > 0) {
			if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_CAPTION))) {
				int cw, ch;
				CaptionCoords[0] = Coords[0] + BorderSizes[0];
				CaptionCoords[1] = Coords[1] + BorderSizes[1];
				GetCIECoords(WCI_CAPTION, &CaptionCoords[0], &CaptionCoords[1]);

				//do the image
				IMG_INFO(GetCIEImageHandle(WCI_CAPTION), &cw, &ch);
				num = (float) w / cw;
				CaptionRectList = bitmap_rect_list(Coords[0] + CornerWidths[2], Coords[1], w, ch, 0, 0, 1.0f, 1.0f);
				h = ch;
			} else {
				h += 5;
				CaptionCoords[0] = Coords[0] + (((Coords[2]-Coords[0]) - w - (close_w + hide_w)) / 2);
				CaptionCoords[1] = Coords[1] + BorderSizes[1];
			}
			CaptionCoords[2] = CaptionCoords[0] + w;
			CaptionCoords[3] = CaptionCoords[1] + h;
		}

		//Find close coordinates now
		CloseCoords[0] = Coords[2] - close_w;
		CloseCoords[1] = Coords[1] + BorderSizes[1];
		GetCIECoords(WCI_CLOSE, &CloseCoords[0], &CloseCoords[1]);
		CloseCoords[2] = CloseCoords[0] + close_w;
		CloseCoords[3] = CloseCoords[1] + close_h;

		//Find hide coordinates now
		HideCoords[0] = CloseCoords[0] - hide_w;
		HideCoords[1] = CloseCoords[1];
		GetCIECoords(WCI_HIDE, &HideCoords[0], &HideCoords[1]);
		HideCoords[2] = HideCoords[0] + hide_w;
		HideCoords[3] = HideCoords[1] + hide_h;
	}

	//Do bitmap stuff
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM), &w, &h);
		num = (float)((Coords[2]-CornerWidths[3])-(Coords[0]+CornerWidths[2])) / (float)w;
		BorderRectLists[CIE_HANDLE_TM] = bitmap_rect_list(Coords[0] + CornerWidths[2], Coords[1], fl2i(w*num),h,0,0,num,1.0f);
		//gr_bitmap_list(&BorderRectLists[CIE_HANDLE_BM], 1, false);
	}
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM), &w, &h);
		num = (float)((Coords[2]-CornerWidths[3])-(Coords[0]+CornerWidths[2])) / (float)w;
		BorderRectLists[CIE_HANDLE_BM] = bitmap_rect_list(Coords[0] + CornerWidths[2], Coords[3]-h, fl2i(w*num),h,0,0,num,1.0f);
		//gr_bitmap_list(&BorderRectLists[CIE_HANDLE_BM], 1, false);
	}
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML), &w, &h);
		num = (float)((Coords[3]-BorderSizes[3])-(Coords[1]+BorderSizes[1])) / (float)h;
		BorderRectLists[CIE_HANDLE_ML] = bitmap_rect_list(Coords[0], Coords[1] + BorderSizes[1], w,fl2i(h*num),0,0,1.0f,num);
	}
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR), &w, &h);
		num = (float)((Coords[3]-BorderSizes[3])-(Coords[1]+BorderSizes[1])) / (float)h;
		BorderRectLists[CIE_HANDLE_MR] = bitmap_rect_list(Coords[2]-w, Coords[1] + BorderSizes[1], w,fl2i(h*num),0,0,1.0f,num);
	}

	return OF_TRUE;
}

int Window::DoMouseOver(float frametime)
{
	if (OwnerSystem->GetMouseX() >= HideCoords[0]
		&& OwnerSystem->GetMouseX() <= HideCoords[2]
		&& OwnerSystem->GetMouseY() >= HideCoords[1]
		&& OwnerSystem->GetMouseY() <= HideCoords[3]) {
		HideHighlight = true;
	} else {
		HideHighlight = false;
	}

	if (OwnerSystem->GetMouseX() >= CloseCoords[0]
		&& OwnerSystem->GetMouseX() <= CloseCoords[2]
		&& OwnerSystem->GetMouseY() >= CloseCoords[1]
		&& OwnerSystem->GetMouseY() <= CloseCoords[3]) {
		CloseHighlight = true;
	} else {
		CloseHighlight = false;
	}

	return OF_TRUE;
}

int Window::DoMouseDown(float frametime)
{
	OwnerSystem->SetActiveObject(this);

	if (Style & WS_NONMOVEABLE) {
		return OF_TRUE;
	}

	if ((OwnerSystem->GetGraspedObject() == NULL)
		&& (Status & GST_MOUSE_LEFT_BUTTON)
		&& OwnerSystem->GetMouseX() >= CaptionCoords[0]
		&& ((OwnerSystem->GetMouseX() < CaptionCoords[2]) || ((Style & WS_NOTITLEBAR) && OwnerSystem->GetMouseX() <= Coords[2]))
		&& OwnerSystem->GetMouseY() >= CaptionCoords[1]
		&& ((OwnerSystem->GetMouseY() <= CaptionCoords[3]) || ((Style & WS_NOTITLEBAR) && OwnerSystem->GetMouseY() <= Coords[2])))
		{
			OwnerSystem->SetGraspedObject(this, GST_MOUSE_LEFT_BUTTON);
		}

		//All further movement of grasped objects is handled by GUISystem

	return OF_TRUE;
}

int Window::DoMouseUp(float frametime)
{
	if (Style & WS_NONMOVEABLE) {
		return OF_TRUE;
	}

	if (CloseHighlight) {
		Delete();
		return OF_TRUE;
	}

	if (HideHighlight) {
		if (Style & GS_HIDDEN) {
			Coords[3] = UnhiddenHeight;
			Style &= ~GS_HIDDEN;
		} else {
			UnhiddenHeight = Coords[3];
			Coords[3] = Coords[1] + (CaptionCoords[3] - CaptionCoords[1]) + BorderSizes[1] + BorderSizes[3];
			Style |= GS_HIDDEN;
		}

		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM))) {
			int h;
			IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM), NULL, &h);
			BorderRectLists[CIE_HANDLE_BM].screen_rect.y = Coords[3] - h;
		}
	}
	return OF_TRUE;
}

int Window::DoMouseOut(float frametime)
{
	HideHighlight = false;
	CloseHighlight = false;
	return OF_TRUE;
}

void Window::DoMove(int dx, int dy)
{
	CloseCoords[0] += dx;
	CloseCoords[1] += dy;
	CloseCoords[2] += dx;
	CloseCoords[3] += dy;

	HideCoords[0] += dx;
	HideCoords[1] += dy;
	HideCoords[2] += dx;
	HideCoords[3] += dy;

	CaptionCoords[0] += dx;
	CaptionCoords[1] += dy;
	CaptionCoords[2] += dx;
	CaptionCoords[3] += dy;

	// change the hidden height too if needed
	if (Style & GS_HIDDEN) {
		UnhiddenHeight += dy;
	}

	//Handle moving the border around
	uint i;
	for (i = 0; i < 8; i++) {
		BorderRectLists[i].screen_rect.x += dx;
		BorderRectLists[i].screen_rect.y += dy;
	}

	//now the caption
	CaptionRectList.screen_rect.x += dx;
	CaptionRectList.screen_rect.y += dy;
}

void draw_open_rect(int x1, int y1, int x2, int y2, bool resize = false)
{
	gr_line(x1, y1, x2, y1, resize);
	gr_line(x2, y1, x2, y2, resize);
	gr_line(x2, y2, x1, y2, resize);
	gr_line(x1, y2, x1, y1, resize);
}

extern void gr_opengl_shade(int x, int y, int w, int h, bool resize);

void Window::DoDraw(float frametime)
{
	int w, h;

	// if the window has no title bar, and no content, then don't draw any of it
	if ( (Style & WS_NOTITLEBAR) && !HasChildren() ) {
		return;
	}

	// shade the background of the window so that it's just slightly transparent
	gr_set_shader(&WindowShade);
	gr_opengl_shade(Coords[0], Coords[1], Coords[2], Coords[3], false);

	gr_set_color_fast(&Color_text_normal);

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL))) {
		IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TL));
		IMG_DRAW(Coords[0], Coords[1]);
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR))) {
		IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR));
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TR), &w, NULL);
		IMG_DRAW(Coords[2] - w, Coords[1]);
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL))) {
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL), NULL, &h);
		IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BL));
		IMG_DRAW(Coords[0], Coords[3] - h);
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR))) {
		IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR));
		IMG_INFO(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BR), &w, &h);
		//IMG_DRAW(Coords[2] - w, Coords[3] - h, false);
		IMG_DRAW(Coords[2]-w, Coords[3] - h);
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM))) {
		IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_TM));
		gr_bitmap_list(&BorderRectLists[CIE_HANDLE_TM], 1, false);
	} else {
		gr_line(Coords[0] + BorderSizes[0], Coords[1], Coords[2] - BorderSizes[2], Coords[1], false);
	}

	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM))) {
		IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_BM));
		gr_bitmap_list(&BorderRectLists[CIE_HANDLE_BM], 1, false);
	} else {
		gr_line(Coords[0] + BorderSizes[0], Coords[3], Coords[2] - BorderSizes[2], Coords[3], false);
	}

	if (!(Style & GS_HIDDEN)) {
		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML))) {
			IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_ML));
			gr_bitmap_list(&BorderRectLists[CIE_HANDLE_ML], 1, false);
		} else {
			gr_line(Coords[0], Coords[1] + BorderSizes[1], Coords[0], Coords[3] - BorderSizes[3], false);
		}

		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR))) {
			IMG_SET(GetCIEImageHandle(WCI_BORDER, CIE_HANDLE_MR));
			gr_bitmap_list(&BorderRectLists[CIE_HANDLE_MR], 1, false);
		} else {
			gr_line(Coords[2], Coords[1] + BorderSizes[1], Coords[2], Coords[3] - BorderSizes[3], false);
		}
	}

	if (!(Style & WS_NOTITLEBAR)) {
		//Draw the caption background
		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_CAPTION))) {
			IMG_SET(GetCIEImageHandle(WCI_CAPTION));
			gr_bitmap_list(&CaptionRectList, 1, false);
		} else {
			draw_open_rect(Coords[0], Coords[1], Coords[2], CaptionCoords[3]);
		}

		//Close button
		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_CLOSE))) {
			if (CloseHighlight && IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_CLOSE, CIE_HANDLE_M))) {
				IMG_SET(GetCIEImageHandle(WCI_CLOSE, CIE_HANDLE_M));
			} else {
				IMG_SET(GetCIEImageHandle(WCI_CLOSE));
			}
			IMG_DRAW(CloseCoords[0], CloseCoords[1]);
		} else {
			if (CloseHighlight) {
				gr_set_color_fast(&Color_text_active);
			} else {
				gr_set_color_fast(&Color_text_normal);
			}
			gr_string(CloseCoords[0], CloseCoords[1], "X", false);
		}
		

		//Hide button
		if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_HIDE))) {
			if (HideHighlight && IMG_HANDLE_IS_VALID(GetCIEImageHandle(WCI_HIDE, CIE_HANDLE_M))) {
				IMG_SET(GetCIEImageHandle(WCI_HIDE, CIE_HANDLE_M));
			} else {
				IMG_SET(GetCIEImageHandle(WCI_HIDE));
			}
			IMG_DRAW(HideCoords[0], HideCoords[1]);
		} else {
			if (HideHighlight) {
				gr_set_color_fast(&Color_text_active);
			} else {
				gr_set_color_fast(&Color_text_normal);
			}
			gr_string(HideCoords[0], HideCoords[1], "-", false);
		}

		//Caption text
		gr_set_color_fast(&Color_text_normal);

		gr_string(CaptionCoords[0], CaptionCoords[1], (char *)Caption.c_str(), false);
	}
}

void Window::ClearContent()
{
	if ( !HasChildren() ) {
		return;
	}

	LinkedList *cgp = GET_FIRST(&Children);
	LinkedList *cgp_next;

	while ( cgp && (cgp != END_OF_LIST(&Children)) ) {
		cgp_next = GET_NEXT(cgp);
		delete cgp;
		cgp = cgp_next;
	}
}

Window::Window(SCP_string in_caption, int x_coord, int y_coord, int x_width, int y_width, int in_style)
:GUIObject(in_caption, x_coord,y_coord,x_width,y_width,in_style)
{
	Caption = in_caption;

	CloseHighlight = false;
	HideHighlight = false;

	//Set the type
	Type = GT_WINDOW;

	// create a shader for the window
	gr_create_shader(&WindowShade, 0, 0, 0, 200);
}

//*****************************Button*******************************

Button::Button(SCP_string in_caption, int x_coord, int y_coord, void (*in_function)(Button *caller), int x_width, int y_height, int in_style)
:GUIObject(in_caption, x_coord, y_coord, x_width, y_height, in_style)
{
	Caption = in_caption;

	function = in_function;

	IsDown = false;

	//Set the type
	Type = GT_BUTTON;
}

int Button::DoRefreshSize()
{
	int w, h;
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(BCI_BUTTON))) {
		IMG_INFO(GetCIEImageHandle(BCI_BUTTON), &w, &h);
		if (!(Style & GS_NOAUTORESIZEX)) {
			Coords[2] = Coords[0] + w;
		}
		if (!(Style & GS_NOAUTORESIZEY)) {
			Coords[3] = Coords[1] + h;
		}
	} else {
		gr_get_string_size(&w, &h, (char *)Caption.c_str());
		if (!(Style & GS_NOAUTORESIZEX)) {
			Coords[2] = Coords[0] + w;
		}
		if (!(Style & GS_NOAUTORESIZEY)) {
			Coords[3] = Coords[1] + h;
		}
	}

	return OF_TRUE;
}

void Button::DoDraw(float frametime)
{
	if (IMG_HANDLE_IS_VALID(GetCIEImageHandle(BCI_BUTTON))) {
		IMG_SET(GetCIEImageHandle(BCI_BUTTON));

		if ((Status & GST_MOUSE_OVER) && IMG_HANDLE_IS_VALID(GetCIEImageHandle(BCI_BUTTON, CIE_HANDLE_M))) {
			IMG_SET(GetCIEImageHandle(BCI_BUTTON, CIE_HANDLE_M));
		} else if ((Status & GST_MOUSE_LEFT_BUTTON) && IMG_HANDLE_IS_VALID(GetCIEImageHandle(BCI_BUTTON, CIE_HANDLE_C))) {
			IMG_SET(GetCIEImageHandle(BCI_BUTTON, CIE_HANDLE_C));
		} else if((Style & BS_STICKY) && IsDown && IMG_HANDLE_IS_VALID(GetCIEImageHandle(BCI_BUTTON, CIE_HANDLE_S))) {
			IMG_SET(GetCIEImageHandle(BCI_BUTTON, CIE_HANDLE_S));
		}
		IMG_DRAW(Coords[0], Coords[1]);
	} else {
		if (Status == 0) {
			gr_set_color_fast(&Color_text_normal);
		} else if (Status & GST_MOUSE_OVER) {
			gr_set_color_fast(&Color_text_active);
		} else {
			gr_set_color_fast(&Color_text_selected);
		}
		draw_open_rect(Coords[0], Coords[1], Coords[2], Coords[3], false);

		int half_x, half_y;
		gr_get_string_size(&half_x, &half_y, (char *)Caption.c_str());
		half_x = Coords[0] +(((Coords[2]-Coords[0]) - half_x) / 2);
		half_y = Coords[1] +(((Coords[3]-Coords[1]) - half_y) / 2);
		gr_string(half_x, half_y, (char *)Caption.c_str(), false);
	}
}

int Button::DoMouseDown(float frametime)
{
	OwnerSystem->SetActiveObject(this);

	if (!(Style & BS_STICKY)) {
		IsDown = true;
	}
	return OF_TRUE;
}

int Button::DoMouseUp(float frametime)
{
	if (function != NULL) {
		function(this);
	}

	if (!(Style & BS_STICKY)) {
		IsDown = false;
	} else {
		IsDown = !IsDown;
	}

	return OF_TRUE;
}

int Button::DoMouseOut(float frametime)
{
	if (!(Style & BS_STICKY)) {
		IsDown = false;
	}

	return OF_TRUE;
}

//*****************************Menu*******************************
TreeItem::TreeItem()
:LinkedList()
{
	Function = NULL;
	Data = 0;
	ShowThis = true;
	ShowChildren = false;
	DeleteData = true;
	Parent = NULL;
	memset(Coords, 0, sizeof(Coords));
}

void TreeItem::ClearAllItems()
{
	LinkedList* cgp = GET_FIRST(&Children);
	LinkedList* cgp_next;
	for (; cgp != END_OF_LIST(&Children); cgp = cgp_next) {
		cgp_next = GET_NEXT(cgp);
		delete cgp;
	}
}

TreeItem::~TreeItem()
{
	ClearAllItems();
}

void Tree::CalcItemsSize(TreeItem *items, int DrawData[4])
{
	//DrawData
	//0 - indent
	//1 - largest width so far
	//2 - total height so far
	//3 - Are we done?
	int w, h;

	int temp_largest = DrawData[1];
	for (TreeItem* tip = (TreeItem*)GET_FIRST(items); tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip)) {
		if (!DrawData[3]) {
			gr_get_string_size(&w, &h, (char*)tip->Name.c_str());
			if ((w + DrawData[0]) > temp_largest) {
					temp_largest = w + DrawData[0];
			}

			//This should really be moved to where we handle the scrolling code
			//I'm not doing that right now. :)
			//Or at least this should be called from that
			if ( ((DrawData[2] + h) > Coords[3] && (Style & GS_NOAUTORESIZEY)) || (temp_largest > Coords[2] && (Style & GS_NOAUTORESIZEX)) ) {
				DrawData[3] = 1;
				tip->ShowThis = false;
			} else {
				tip->ShowThis = true;
			}

			//Do we care?
			if (tip->ShowThis) {
				if ((w + DrawData[0]) > DrawData[1]) {
					DrawData[1] = w + DrawData[0];
				}

				tip->Coords[0] = Coords[0] + DrawData[0];
				tip->Coords[1] = Coords[1] + DrawData[2];
				tip->Coords[2] = Coords[0] + DrawData[0] + w;
				tip->Coords[3] = Coords[1] + DrawData[2] + h;

				DrawData[2] += h;

				if (NOT_EMPTY(&tip->Children) && tip->ShowChildren) {
					//Indent
					DrawData[0] += TI_INDENT_PER_LEVEL;
					CalcItemsSize((TreeItem*)&tip->Children, DrawData);
					DrawData[0] -= TI_INDENT_PER_LEVEL;
				}
			}
		}
		if (DrawData[3] || !tip->ShowThis) {
			tip->ShowThis = false;
			tip->ShowChildren = false;
			tip->Coords[0] = tip->Coords[1] = tip->Coords[2] = tip->Coords[3] = -1;
		}
	}
}

int Tree::DoRefreshSize()
{
	//DrawData
	//0 - indent
	//1 - largest width so far
	//2 - total height so far
	//3 - Are we done? 0=no,1=yes
	int DrawData[4] = {TI_INITIAL_INDENT,0,TI_INITIAL_INDENT_VERTICAL,0};
	CalcItemsSize(&Items, DrawData);

	//CalcItemsSize takes into account limited height/width
	if (!(Style & GS_NOAUTORESIZEX)) {
		Coords[2] = Coords[0] + DrawData[1] + TI_BORDER_WIDTH;
	}
	if (!(Style & GS_NOAUTORESIZEY)) {
		Coords[3] = Coords[1] + DrawData[2] + TI_BORDER_HEIGHT;
		if (Parent != NULL) {
			if(DrawData[2] >= Parent->ChildCoords[3]) {
			}
		}
	}

	return OF_TRUE;
}

Tree::Tree(SCP_string in_name, int x_coord, int y_coord, void* in_associateditem, int x_width, int y_height, int in_style)
:GUIObject(in_name, x_coord, y_coord, x_width, y_height, in_style)
{
	AssociatedItem = in_associateditem;

	SelectedItem = NULL;
	HighlightedItem = NULL;

	//Set the type
	Type = GT_MENU;
}

void Tree::DrawItems(TreeItem *items)
{
	for (TreeItem* tip = (TreeItem*)GET_FIRST(items); tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip)) {
		if (tip->ShowThis) {
			if (SelectedItem == tip) {
				gr_set_color_fast(&Color_text_selected);
			} else if (HighlightedItem == tip) {
				gr_set_color_fast(&Color_text_active);
			} else {
				gr_set_color_fast(&Color_text_normal);
			}

			gr_string(tip->Coords[0], tip->Coords[1], (char*)tip->Name.c_str(), false);

			if (NOT_EMPTY(&tip->Children) && tip->ShowChildren) {
				DrawItems((TreeItem*)&tip->Children);
			}
		}
	}
}

void Tree::DoDraw(float frametime)
{

	if (EMPTY(&Items)) {
		return;
	}

	DrawItems(&Items);

	gr_set_color_fast(&Color_text_normal);
	draw_open_rect(Coords[0], Coords[1], Coords[2], Coords[3]);
}

TreeItem* Tree::HitTest(TreeItem *items)
{
	TreeItem *hti = NULL;	//Highlighted item

	for (TreeItem *tip = (TreeItem*)GET_FIRST(items);tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip)) {
		if (OwnerSystem->GetMouseX() >= tip->Coords[0]
			&& OwnerSystem->GetMouseX() <= tip->Coords[2]
			&& OwnerSystem->GetMouseY() >= tip->Coords[1]
			&& OwnerSystem->GetMouseY() <= tip->Coords[3]) {

			hti = tip;
		}

		if (hti != NULL) {
			return hti;
		} else if (NOT_EMPTY(&tip->Children) && tip->ShowChildren) {
			hti = HitTest((TreeItem*)&tip->Children);
		}

		if (hti != NULL) {
			return hti;
		}
	}
	return hti;
}

int Tree::DoMouseOver(float frametime)
{
	HighlightedItem = HitTest(&Items);
	return OF_TRUE;
}

int Tree::DoMouseDown(float frametime)
{
	OwnerSystem->SetActiveObject(this);
	return OF_TRUE;
}

int Tree::DoMouseUp(float frametime)
{
	OwnerSystem->SetActiveObject(this);

	if (HighlightedItem != NULL) {
		SelectedItem = HighlightedItem;
		SelectedItem->ShowChildren = !SelectedItem->ShowChildren;
		if (NOT_EMPTY(&SelectedItem->Children)) {
			OnRefreshSize();	//Unfortunately
		}
		if (SelectedItem->Function != NULL) {
			SelectedItem->Function(this);
		}
	}

	return OF_TRUE;
}

void Tree::MoveTreeItems(int dx, int dy, TreeItem *items)
{
	for (TreeItem *tip = (TreeItem*)GET_FIRST(items);tip != END_OF_LIST(items); tip = (TreeItem*)GET_NEXT(tip)) {
		if (tip->ShowThis) {
			tip->Coords[0] += dx;
			tip->Coords[1] += dy;
			tip->Coords[2] += dx;
			tip->Coords[3] += dy;

			if (NOT_EMPTY(&tip->Children) && tip->ShowChildren) {
				MoveTreeItems(dx, dy, (TreeItem*)&tip->Children);
			}
		}
	}
}

void Tree::DoMove(int dx, int dy)
{
	MoveTreeItems(dx, dy, &Items);
}

TreeItem* Tree::AddItem(TreeItem *parent, SCP_string in_name, int in_data, bool in_delete_data, void (*in_function)(Tree* caller))
{
	TreeItem *ni = new TreeItem;

	ni->Parent = parent;
	ni->Name = in_name;
	ni->Data = in_data;
	ni->DeleteData = in_delete_data;
	ni->Function = in_function;

	if (parent != NULL) {
		list_append(&parent->Children, ni);
	} else {
		list_append(&Items, ni);
	}

	OnRefreshSize();

	return ni;
}

void Tree::ClearItems()
{
	Items.ClearAllItems();

	LinkedList* cgp = GET_FIRST(&Items);
	LinkedList* cgp_next;
	for (; cgp != END_OF_LIST(&Items); cgp = cgp_next) {
		cgp_next = GET_NEXT(cgp);
		delete cgp;
	}
}

//*****************************Text*******************************
int Text::DoRefreshSize()
{
	int width;
	if (Style & GS_NOAUTORESIZEX) {
		width = Coords[2] - Coords[0];
	} else if (Parent != NULL && (Parent->Style & GS_NOAUTORESIZEX)) {
		width = Parent->ChildCoords[2] - Coords[0];
	} else {
		width = gr_screen.clip_right - Coords[0];
	}

	NumLines = split_str((char*)Content.c_str(), width, LineLengths, LineStartPoints, MAX_TEXT_LINES);

	//Find the shortest width we need to show all the shortened strings
	int longest_width = 0;
	int width_test;
	for (int i = 0; i < NumLines; i++) {
		gr_get_string_size(&width_test, NULL, LineStartPoints[i], LineLengths[i]);
		if (width_test > longest_width) {
			longest_width = width_test;
		}
	}
	if (longest_width < width) {
		width = longest_width;
	}

	//Resize along the Y axis
	if (Style & GS_NOAUTORESIZEY) {
		int new_height = (Coords[3] - Coords[1]) / gr_get_font_height();

		if (new_height < NumLines) {
			NumLines = new_height;
		}
	} else {
		Coords[3] = Coords[1] + gr_get_font_height() * NumLines;
		if (Parent != NULL) {
			if (Coords[3] >= Parent->ChildCoords[3] && (Parent->Style & GS_NOAUTORESIZEY)) {
				Coords[3] = Coords[1] + (Parent->ChildCoords[3] - Coords[1]);
				NumLines = (Coords[3] - Coords[1]) / gr_get_font_height();
			}
		}
	}

	if (!(Style & GS_NOAUTORESIZEX)) {
		Coords[2] = Coords[0] + width;
	}

	if (Style & T_EDITTABLE) {
		ChildCoords[0] = Coords[0] + 1;
		ChildCoords[1] = Coords[1] + 1;
		ChildCoords[2] = Coords[2] - 1;
		ChildCoords[3] = Coords[3] - 1;
	} else {
		ChildCoords[0] = Coords[0];
		ChildCoords[1] = Coords[1];
		ChildCoords[2] = Coords[2];
		ChildCoords[3] = Coords[3];
	}

	//Tell the parent to recalculate its size too
	return OF_TRUE;
}

int Text::DoMouseDown(float frametime)
{
	//Make this the active object
	if (Style & T_EDITTABLE) {
		OwnerSystem->SetActiveObject(this);
		//For now, always set the cursor pos to the end
		CursorPos = Content.size();
		return OF_TRUE;
	} else {
		return OF_FALSE;
	}
}

void Text::DoDraw(float frametime)
{
	if (OwnerSystem->GetActiveObject() != this) {
		gr_set_color_fast(&Color_text_normal);
	} else {
		gr_set_color_fast(&Color_text_active);
	}

	if (Style & T_EDITTABLE) {
		draw_open_rect(Coords[0], Coords[1], Coords[2], Coords[3]);
	}

	//Don't draw anything. Besides, it crashes
	if (!Content.length()) {
		return;
	}

	int font_height = gr_get_font_height();

	for (int i = 0; i < NumLines; i++) {
		gr_string(ChildCoords[0], ChildCoords[1] + (i*font_height), (char*)Content.substr(LineStartPoints[i] - Content.c_str(), LineLengths[i]).c_str(), false);
	}
}

extern int keypad_to_ascii(int c);

int Text::DoKeyPress(float frametime)
{
	if (!(Style & T_EDITTABLE)) {
		return OF_FALSE;
	}

	switch(OwnerSystem->GetKeyPressed()) {
		case KEY_BACKSP:
			Content = Content.substr(0, Content.length() - 1);
		break;

		case KEY_ENTER:
			if (SaveType & T_ST_ONENTER) {
				if (Save()) {
					OwnerSystem->SetActiveObject(Parent);
				} else {
					Load();
				}
			}
		break;

		case KEY_ESC:
			Load();
			OwnerSystem->SetActiveObject(Parent);
		break;

		default: {
			//Figure out what key, exactly, we have
			int symbol = keypad_to_ascii(OwnerSystem->GetKeyPressed());
			if (symbol == -1) {
				symbol = key_to_ascii(OwnerSystem->GetKeyPressed());
			}
			//Is it ok with us?
			if (iscntrl(symbol)) {
				return OF_FALSE;	//Guess not
			} else if (SaveType & T_ST_INT) {
				if (!isdigit(symbol) && symbol != '-') {
					return OF_FALSE;
				}
			} else if (SaveType & T_ST_FLOAT) {
				if (!isdigit(symbol) && symbol != '.' && symbol != '-') {
					return OF_FALSE;
				}
			} else if (SaveType & T_ST_UBYTE) {
				if (!isdigit(symbol)) {
					return OF_FALSE;
				}
			} else if (!isalnum(symbol) && symbol != ' ') {
				return OF_FALSE;
			}

			//We can add the letter to the box. Cheers.
			Content += char(symbol);
			if (SaveType & T_ST_REALTIME) {
				Save();
			}
		}
	}
	OnRefreshSize();
	return OF_TRUE;
}

Text::Text(SCP_string in_name, SCP_string in_content, int x_coord, int y_coord, int x_width, int y_height, int in_style)
:GUIObject(in_name, x_coord, y_coord, x_width, y_height, in_style)
{
	Content = in_content;
	NumLines = 0;
	SaveType = T_ST_NONE;
	CursorPos = -1;	//Not editing
	Type = GT_TEXT;
}

void Text::SetText(SCP_string in_content)
{
	Content = in_content;
	OnRefreshSize();
}

void Text::SetText(int the_int)
{
	char buf[33];
	sprintf(buf, "%d", the_int);
	Content = buf;
	OnRefreshSize();
}

void Text::SetText(float the_float)
{
	char buf[32];
	sprintf(buf, "%f", the_float);
	Content = buf;
	OnRefreshSize();
}

void Text::AddLine(SCP_string in_line)
{
	Content += in_line;
	OnRefreshSize();
}

void Text::SetSaveLoc(int *int_ptr, int save_method, int max_value, int min_value)
{
	Assert(int_ptr != NULL);		//Naughty.
	Assert(min_value < max_value);	//Mmm-hmm.

	SaveType = T_ST_INT;
	if (save_method == T_ST_CLOSE) {
		SaveType |= T_ST_CLOSE;
	} else if (save_method == T_ST_REALTIME) {
		SaveType |= T_ST_REALTIME;
	} else if (save_method == T_ST_ONENTER) {
		SaveType |= T_ST_ONENTER;
	}

	iSavePointer = int_ptr;
	SaveMax = max_value;
	SaveMin = min_value;
}

void Text::SetSaveLoc(short int *sint_ptr, int save_method, short int max_value, short int min_value)
{
	Assert(sint_ptr != NULL);		//Naughty.
	Assert(min_value < max_value);	//Mmm-hmm.

	SaveType = T_ST_SINT;
	if (save_method == T_ST_CLOSE) {
		SaveType |= T_ST_CLOSE;
	} else if (save_method == T_ST_REALTIME) {
		SaveType |= T_ST_REALTIME;
	} else if (save_method == T_ST_ONENTER) {
		SaveType |= T_ST_ONENTER;
	}

	siSavePointer = sint_ptr;
	SaveMax = max_value;
	SaveMin = min_value;
}

void Text::SetSaveLoc(float *ptr, int save_method, float max_value, float min_value)
{
	Assert(ptr != NULL);		//Naughty.
	Assert(min_value < max_value);	//Causes problems with floats

	SaveType = T_ST_FLOAT;
	if (save_method == T_ST_CLOSE) {
		SaveType |= T_ST_CLOSE;
	} else if (save_method == T_ST_REALTIME) {
		SaveType |= T_ST_REALTIME;
	} else if (save_method == T_ST_ONENTER) {
		SaveType |= T_ST_ONENTER;
	}

	flSavePointer = ptr;
	flSaveMax = max_value;
	flSaveMin = min_value;
}

void Text::SetSaveLoc(char *ptr, int save_method, uint max_length, uint min_length)
{
	Assert(ptr != NULL);		//Naughty.

	SaveType = T_ST_CHAR;
	if (save_method == T_ST_CLOSE) {
		SaveType |= T_ST_CLOSE;
	} else if (save_method == T_ST_REALTIME) {
		SaveType |= T_ST_REALTIME;
	} else if (save_method == T_ST_ONENTER) {
		SaveType |= T_ST_ONENTER;
	}

	chSavePointer = ptr;
	uSaveMax = max_length;
	uSaveMin = min_length;
}

void Text::SetSaveStringAlloc(char **ptr, int save_method, int mem_flags, uint max_length, uint min_length)
{
	Assert(ptr != NULL);		//Naughty.

	if (mem_flags == T_ST_NEW) {
		SaveType = T_ST_NEW;
	} else if (mem_flags == T_ST_MALLOC) {
		SaveType = T_ST_MALLOC;
	} else {
		return;	//This MUST use some sort of mem allocation
	}

	SaveType |= T_ST_CHAR;
	if (save_method == T_ST_CLOSE) {
		SaveType |= T_ST_CLOSE;
	} else if (save_method == T_ST_REALTIME) {
		SaveType |= T_ST_REALTIME;
	} else if (save_method == T_ST_ONENTER) {
		SaveType |= T_ST_ONENTER;
	}

	chpSavePointer = ptr;
	uSaveMax = max_length;
	uSaveMin = min_length;
}

void Text::SetSaveLoc(ubyte *ptr, int save_method, int max_value, int min_value)
{
	Assert(ptr != NULL);		//Naughty.
	Assert(min_value < max_value);	//Mmm-hmm.

	SaveType = T_ST_UBYTE;
	if (save_method == T_ST_CLOSE) {
		SaveType |= T_ST_CLOSE;
	} else if (save_method == T_ST_REALTIME) {
		SaveType |= T_ST_REALTIME;
	} else if (save_method == T_ST_ONENTER) {
		SaveType |= T_ST_ONENTER;
	}

	ubSavePointer = ptr;
	SaveMax = max_value;
	SaveMin = min_value;
}

bool Text::Save()
{
	if (SaveType == T_ST_NONE) {
		return false;
	}

	if (SaveType & T_ST_INT) {
		int the_int =  atoi(Content.c_str());
		if (the_int <= SaveMax && the_int >= SaveMin) {
			*iSavePointer = the_int;
			return true;
		}
	} else if (SaveType & T_ST_SINT) {
		short the_sint =  (short)atoi(Content.c_str());
		if (the_sint <= SaveMax && the_sint >= SaveMin) {
			*siSavePointer = the_sint;
			return true;
		}
	} else if (SaveType & T_ST_FLOAT) {
		float the_float = (float)atof(Content.c_str());
		if (the_float <= flSaveMax && the_float >= flSaveMin) {
			*flSavePointer = the_float;
			return true;
		}
	} else if (SaveType & T_ST_CHAR) {
		size_t len = Content.length();
		if (SaveType & T_ST_NEW && len < uSaveMax) {
			if (*chpSavePointer != NULL) {
				delete[] *chpSavePointer;
			}

			*chpSavePointer = new char[len + 1];
			strcpy(*chpSavePointer, Content.c_str());
			return true;
		} else if (SaveType & T_ST_MALLOC && len < uSaveMax) {
			if (*chpSavePointer != NULL) {
				vm_free(*chpSavePointer);
			}

			*chpSavePointer = (char*)vm_malloc(sizeof(char) * (len + 1));
			strcpy(*chpSavePointer, Content.c_str());
			return true;
		} else if (len < uSaveMax) {
			strcpy(chSavePointer, Content.c_str());
			return true;
		}

	} else if (SaveType & T_ST_UBYTE) {
		int the_int = atoi(Content.c_str());
		if (the_int <= SaveMax && the_int >= SaveMin) {
			*ubSavePointer = (ubyte)the_int;
			return true;
		}
	}

	return false;
}

void Text::Load()
{
	if (SaveType == T_ST_NONE) {
		return;
	}

	if (SaveType & T_ST_INT) {
		SetText(*iSavePointer);
	} else if (SaveType & T_ST_FLOAT) {
		SetText(*flSavePointer);
	} else if ((SaveType & T_ST_CHAR) && (SaveType & (T_ST_MALLOC | T_ST_NEW))) {
		SetText(*chpSavePointer);
	} else if (SaveType & T_ST_CHAR) {
		SetText(chSavePointer);
	} else if (SaveType & T_ST_UBYTE) {
		SetText(*ubSavePointer);
	} else {
		Warning(LOCATION, "Unknown type (or no type) given in Text::Load() - nothing happened.");
	}
}

//*****************************Checkbox*******************************

Checkbox::Checkbox(SCP_string in_label, int x_coord, int y_coord, void (*in_function)(Checkbox *caller), int x_width, int y_height, int in_style)
:GUIObject(in_label, x_coord, y_coord, x_width, y_height, in_style)
{
	Label = in_label;

	function = in_function;

	IsChecked = false;
	HighlightStatus = 0;
	FlagPtr = NULL;
	BoolFlagPtr = NULL;

	//Set the type
	Type = GT_CHECKBOX;
}

int Checkbox::DoRefreshSize()
{
	int w, h,tw,th;
	gr_get_string_size(&w, &h, (char *)Label.c_str());
	tw = w;
	th = h;

	gr_get_string_size(&w, &h, "X");
	tw += w;
	th += h + CB_TEXTCHECKDIST; //Spacing b/t 'em

	CheckCoords[0] = Coords[0];
	CheckCoords[1] = Coords[1];
	CheckCoords[2] = Coords[0] + w;
	CheckCoords[3] = Coords[1] + h;

	if (!(Style & GS_NOAUTORESIZEX)) {
		Coords[2] = Coords[0] + tw;
	}

	if (!(Style & GS_NOAUTORESIZEY)) {
		Coords[3] = Coords[1] + th;
	}

	return OF_TRUE;
}

void Checkbox::DoMove(int dx, int dy)
{
	CheckCoords[0] += dx;
	CheckCoords[2] += dx;
	CheckCoords[1] += dy;
	CheckCoords[3] += dy;
}

void Checkbox::DoDraw(float frametime)
{
	if (HighlightStatus == 1) {
		gr_set_color_fast(&Color_text_active);
	} else if (HighlightStatus == 2) {
		gr_set_color_fast(&Color_text_selected);
	} else {
		gr_set_color_fast(&Color_text_normal);
	}

	draw_open_rect(CheckCoords[0], CheckCoords[1], CheckCoords[2], CheckCoords[3], false);

	if ( (IsChecked && ((FlagPtr == NULL) && (BoolFlagPtr == NULL)))
		|| ((FlagPtr != NULL) && ((*FlagPtr) & Flag))
		|| ((BoolFlagPtr != NULL) && (*BoolFlagPtr)) ) {
		gr_string(CheckCoords[0], CheckCoords[1], "X", false);
	}

	gr_set_color_fast(&Color_text_normal);
	gr_string(CheckCoords[2] + CB_TEXTCHECKDIST, CheckCoords[1], (char *)Label.c_str(), false);
}

int Checkbox::DoMouseOver(float frametime)
{
	if ( (OwnerSystem->GetMouseX() >= CheckCoords[0])
		&& (OwnerSystem->GetMouseX() <= CheckCoords[2])
		&& (OwnerSystem->GetMouseY() >= CheckCoords[1])
		&& (OwnerSystem->GetMouseY() <= CheckCoords[3]) ) {
		HighlightStatus = 1;
	}

	return OF_TRUE;
}

int Checkbox::DoMouseDown(float frametime)
{
	OwnerSystem->SetActiveObject(this);

	if ( (OwnerSystem->GetMouseX() >= CheckCoords[0])
		&& (OwnerSystem->GetMouseX() <= CheckCoords[2])
		&& (OwnerSystem->GetMouseY() >= CheckCoords[1])
		&& (OwnerSystem->GetMouseY() <= CheckCoords[3]) ) {
		HighlightStatus = 2;
	}

	return OF_TRUE;
}

int Checkbox::DoMouseUp(float frametime)
{
	if ( (OwnerSystem->GetMouseX() >= CheckCoords[0])
		&& (OwnerSystem->GetMouseX() <= CheckCoords[2])
		&& (OwnerSystem->GetMouseY() >= CheckCoords[1])
		&& (OwnerSystem->GetMouseY() <= CheckCoords[3]) ) {
		HighlightStatus = 1;

		if (function != NULL) {
			function(this);
		}

		if (FlagPtr != NULL) {
			if ( !(*FlagPtr & Flag) ) {
				*FlagPtr |= Flag;
				IsChecked = true;
			} else {
				*FlagPtr &= ~Flag;
				IsChecked = false;
			}
		} else if (BoolFlagPtr != NULL) {
			*BoolFlagPtr = !(*BoolFlagPtr);
			IsChecked = *BoolFlagPtr;
		} else {
			IsChecked = !IsChecked;
		}
	}

	return OF_TRUE;
}

int Checkbox::DoMouseOut(float frametime)
{
	HighlightStatus = 0;

	return OF_TRUE;
}

//*****************************ImageAnim*******************************
ImageAnim::ImageAnim(SCP_string in_name, SCP_string in_imagename, int x_coord, int y_coord, int x_width, int y_width, int in_style)
:GUIObject(in_name, x_coord, y_coord, x_width, y_width, in_style)
{
	//Load the image
	IsSet = false;
	ImageHandle = -1;
	ImageFlags = 0;
	TotalFrames = 0;
	FPS = 0;
	Stop();

	Type = GT_IMAGEANIM;
}

void ImageAnim::DoDraw(float frametime)
{
	//Do nothing if nothing to draw
	if (ImageHandle == -1) {
		return;
	}

	if (PlayType != PT_STOPPED) {
		if (PlayType == PT_PLAYING) {
			ElapsedTime += frametime;
		} else if (PlayType == PT_PLAYING_REVERSE) {
			ElapsedTime -= frametime;
		}

		Progress = ElapsedTime/TotalTime;
	}

	int CurrentFrame = fl2i(Progress * TotalFrames);
	if (Progress > 1.0f) {
		if (ImageFlags & IF_BOUNCE) {
			Progress = 1.0f;
			CurrentFrame = TotalFrames;
			PlayType = PT_PLAYING_REVERSE;
		} else if (ImageFlags & IF_REPEAT) {
			Progress = 0.0f;
			CurrentFrame = 0;
			PlayType = PT_PLAYING;
		} else {
			Progress = 1.0f;
			CurrentFrame = TotalFrames;
			PlayType = PT_STOPPED;
		}
	} else if (Progress < 1.0f) {
		if (ImageFlags & IF_BOUNCE) {
			Progress = 0.0f;
			CurrentFrame = 0;
			PlayType = PT_PLAYING;
		} else if (ImageFlags & IF_REPEAT && ImageFlags & (IF_REVERSED)) {
			Progress = 1.0f;
			CurrentFrame = TotalFrames;
			PlayType = PT_PLAYING_REVERSE;
		} else {
			Progress = 0.0f;
			CurrentFrame = 0;
			PlayType = PT_STOPPED;
		}
	}

	//IMG_SET_FRAME(ImageHandle, CurrentFrame);
	gr_set_bitmap( ImageHandle + CurrentFrame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1-((Progress - (float)(CurrentFrame / TotalFrames)) / (float)(TotalFrames/CurrentFrame)));
	IMG_DRAW(Coords[0], Coords[1]);
	gr_set_bitmap( ImageHandle + CurrentFrame + 1, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, ((Progress - (float)(CurrentFrame / TotalFrames)) / (float)(TotalFrames/CurrentFrame)));
	IMG_DRAW(Coords[0], Coords[1]);
}

int ImageAnim::DoRefreshSize()
{
	//TODO: Fill this in
	return OF_FALSE;
}

void ImageAnim::SetImage(SCP_string in_imagename)
{
	if (in_imagename.size()) {
		ImageHandle = IMG_LOAD_ANIM((char*)in_imagename.c_str(), &TotalFrames, &FPS);
		TotalTime = (float) (TotalFrames/FPS);
		if (IMG_HANDLE_IS_INVALID(ImageHandle)) {
			ImageHandle = IMG_LOAD((char*)in_imagename.c_str());
			if (IMG_HANDLE_IS_VALID(ImageHandle)) {
				TotalFrames = 1;
			} else {
				TotalFrames = 0;
			}
			FPS = 0;
			TotalTime = 0.0f;
		}
	}

	//This takes care of setting up the play-flags
	Stop();

	IsSet = true;
	OnRefreshSize();
}

void ImageAnim::Play(bool in_isreversed)
{
	if (ImageFlags & IF_REVERSED) {
		PlayType = PT_PLAYING;
	}
}

void ImageAnim::Pause()
{
	if (PlayType == PT_PLAYING) {
		PlayType = PT_STOPPED;
	} else {
		PlayType = PT_STOPPED_REVERSE;
	}
}

void ImageAnim::Stop()
{
	if (ImageFlags & IF_REVERSED) {
		PlayType = PT_STOPPED_REVERSE;
	} else {
		PlayType = PT_STOPPED;
	}

	Progress = 0.0f;
	ElapsedTime = 0.0f;
}
