/*
 * wmcgui.h
 * created by WMCoolmon
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */



#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "io/mouse.h"
#include "globalincs/pstypes.h"

#include <string>
#include <limits.h>

//*****************************Low-level abstraction*******************************
//Lame attempt to keep things from being exceedingly difficult when switching to ferrium
#ifndef FERRIUM
#define IMG_HANDLE					int
#define IMG_HANDLE_SET_INVALID(h)	h = -1
#define IMG_HANDLE_IS_INVALID(h)	(h==-1)
#define IMG_HANDLE_IS_VALID(h)		(h!=-1)
#define IMG_HANDLE_SET_FRAME(dh,h,f)(dh = h + f)
#define IMG_LOAD(f)					bm_load(f)
#define IMG_LOAD_ANIM(f,n,fps)		bm_load_animation(f,n,fps)
#define IMG_UNLOAD(a)				bm_unload(a)
#define IMG_SET(h)					gr_set_bitmap(h)
#define IMG_SET_FRAME(h, f)			gr_set_bitmap(h + f)
#define IMG_DRAW(x,y)				gr_bitmap(x,y,false)
#define IMG_INFO(ha,w,h)			bm_get_info(ha,w,h)
#endif

//*****************************LinkedList*******************************
struct LinkedList
{
	friend class GUISystem;
	friend class GUIObject;
	friend class Tree;
	friend struct TreeItem;
	friend class Window;
public:
	struct LinkedList *next, *prev;

	LinkedList(){ next = this;  prev = this; }
	virtual ~LinkedList(){ prev->next = next;  next->prev = prev; }
};

//*****************************ClassInfoEntry*******************************
//This is chiefly used in conjunction with GUISystem to store information
//about classes

//T - Top
//L - Left
//R - Right
//B - Bottom

//N - Normal
//M - Mouseovered
//C - Clicked
//S - Selected/Active (for windows)
//D - Disabled

//A - Active
//I - Inactive

//Types of CIEs
#define CIE_NONE			-1
#define CIE_IMAGE			0
#define CIE_IMAGE_NMCSD		1
#define CIE_IMAGE_BORDER	2
#define CIE_COORDS			3
#define CIE_TEXT			4

//NMCSD Handles
#define CIE_HANDLE_N	0
#define CIE_HANDLE_M	1
#define CIE_HANDLE_C	3
#define CIE_HANDLE_S	3
#define CIE_HANDLE_D	4

//Border handles
#define CIE_HANDLE_TL	0
#define CIE_HANDLE_TM	1
#define CIE_HANDLE_TR	2
#define CIE_HANDLE_ML	3
#define CIE_HANDLE_MR	4
#define CIE_HANDLE_BL	5
#define CIE_HANDLE_BM	6
#define CIE_HANDLE_BR	7

//Text
#define CIE_COLOR_R		0
#define CIE_COLOR_G		1
#define CIE_COLOR_B		2
#define CIE_COLOR_A		3

//Return vals
#define CIE_GC_NONE_SET	0
#define CIE_GC_X_SET	(1<<0)
#define CIE_GC_Y_SET	(1<<1)
#define CIE_GC_W_SET	(1<<2)
#define CIE_GC_H_SET	(1<<3)

//Global stuff
#define CIE_NUM_HANDLES	8	//We need 8 for border
union Handle
{
	ubyte Colors[4];
	IMG_HANDLE Image;
};

//Individual info
class ClassInfoEntry
{
	int CIEType;
	Handle Handles[CIE_NUM_HANDLES];
	int Coords[2];

public:
	//--CONSTRUCTORS
	ClassInfoEntry();
	~ClassInfoEntry();

	//--SET FUNCTIONS
	void Parse(char* tag, int in_type);

	//--GET FUNCTIONS
	int GetImageHandle(int ID=CIE_HANDLE_N){return Handles[ID].Image;}
	ubyte GetColorHandle(int ColorID, int ID=CIE_HANDLE_N){if(this==NULL){return 0;}else{return Handles[ID].Colors[ColorID];}}
	//Copies the coordinates to the given location if coordinates are set.
	int GetCoords(int *x, int *y);
};

//Entries for an object, ie a window or button
class ObjectClassInfoEntry
{
	friend class GUIScreen;
private:
	int Object;
	std::string Name;	//Do we want this to only apply to a specific object?
						//If so, set name
	int Coords[4];

	SCP_vector<ObjectClassInfoEntry> Subentries;
	SCP_vector<ClassInfoEntry> Entries;
public:
	ObjectClassInfoEntry(){Object=-1;Coords[0]=Coords[1]=Coords[2]=Coords[3]=INT_MAX;}
	bool Parse();
	int GetImageHandle(int id, int handle_num);
	int GetCoords(int id, int *x, int *y);

	int GetObjectCoords(int *x, int *y, int *w, int *h);
};

//Entries for a screen.
class ScreenClassInfoEntry : public LinkedList
{
	friend class GUIScreen;
private:
	std::string Name;
	SCP_vector<ObjectClassInfoEntry> Entries;
public:
	bool Parse();

	std::string GetName(){return Name;}
};

//*****************************GUIObject*******************************
//What type a GUIObject is, mostly for debugging
#define GT_NONE					0
#define GT_WINDOW				1
#define GT_BUTTON				2
#define GT_MENU					3
#define GT_TEXT					4
#define	GT_CHECKBOX				5
#define GT_IMAGEANIM			6
#define GT_HUDGAUGE				7
#define GT_NUM_TYPES			8	//Total number of types

//States of being for GUIObjects
#define	GST_NORMAL				0
#define GST_MOUSE_LEFT_BUTTON	(1<<0)
#define GST_MOUSE_RIGHT_BUTTON	(1<<1)
#define GST_MOUSE_MIDDLE_BUTTON	(1<<2)
#define GST_MOUSE_OVER			(1<<3)
#define GST_KEYBOARD_CTRL		(1<<4)
#define GST_KEYBOARD_ALT		(1<<5)
#define GST_KEYBOARD_SHIFT		(1<<6)
#define GST_KEYBOARD_KEYPRESS	(1<<7)

#define GST_MOUSE_PRESS		(GST_MOUSE_LEFT_BUTTON | GST_MOUSE_RIGHT_BUTTON | GST_MOUSE_MIDDLE_BUTTON)
#define GST_MOUSE_STATUS	(GST_MOUSE_LEFT_BUTTON | GST_MOUSE_RIGHT_BUTTON | GST_MOUSE_MIDDLE_BUTTON | GST_MOUSE_OVER)
#define GST_KEYBOARD_STATUS (GST_KEYBOARD_CTRL | GST_KEYBOARD_ALT | GST_KEYBOARD_SHIFT | GST_KEYBOARD_KEYPRESS)

//GUIObject styles
#define GS_NOAUTORESIZEX		(1<<0)
#define	GS_NOAUTORESIZEY		(1<<1)
#define GS_HIDDEN				(1<<2)
#define GS_INTERNALCHILD		(1<<3)

//DoFrame return values
#define OF_TRUE					-1
#define OF_FALSE				-2
//#define OF_DESTROYED			-3	//If a call to DoFrame results in the object destroying itself
									//(ie the close button was pressed)

class GUIObject : public LinkedList
{
	friend class Window;			//Hack, because I can't figure out how to let it access protected
	friend class Menu;				//This too
	friend class Text;				//And this
	friend class Tree;				//By, the way...THIS
	friend class Checkbox;
	friend class Button;
	friend class ImageAnim;
	friend class HUDGauge;
	friend class GUIScreen;
	friend class GUISystem;
private:
	class GUISystem		*OwnerSystem;			//What system this object is associated with
	class GUIScreen		*OwnerScreen;

	int Coords[4];					//Upper left corner (x, y) and lower right corner (x, y)
	int	ChildCoords[4];				//Coordinates where children may frolick

	int Type;
	std::string Name;

	int LastStatus;
	int Status;
	int Style;

	class ObjectClassInfoEntry* InfoEntry;

	void (*CloseFunction)(GUIObject *caller);

	class GUIObject* Parent;
	LinkedList Children;

	int GetOIECoords(int *x1, int *y1, int *x2, int *y2);
	GUIObject *AddChildInternal(GUIObject* cgp);
protected:
	//ON FUNCTIONS
	//These handle the calling of do functions, mostly.
	void OnDraw(float frametime);
	int OnFrame(float frametime, int *unused_queue);
	void OnMove(int dx, int dy);
	void OnRefreshSize(){if(DoRefreshSize() != OF_FALSE && Parent!=NULL)Parent->OnRefreshSize();}
	void OnRefreshSkin(){SetCIPointer(); DoRefreshSkin();}

	//DO FUNCTIONS
	//Used by individual objects to define actions when that event happens
	virtual void DoDraw(float frametime){}
	virtual int DoFrame(float frametime){return OF_FALSE;}
	virtual int DoRefreshSize(){return OF_FALSE;}
	virtual void DoRefreshSkin(){}
	virtual void DoMove(int dx, int dy){}
	virtual int DoMouseOver(float frametime){return OF_FALSE;}
	virtual int DoMouseDown(float frametime){return OF_FALSE;}
	virtual int DoMouseUp(float frametime){return OF_FALSE;}	//In other words, a click
	virtual int DoMouseOut(float frametime){return OF_FALSE;}
	virtual int DoKeyState(float frametime){return OF_FALSE;}
	virtual int DoKeyPress(float frametime){return OF_FALSE;}

	//CALCULATESIZE
	//Sort of an on and do function; if you define your own, the following MUST be included at the end:
	//"if(Parent!=NULL)Parent->OnRefreshSize();"

	//PRIVATE ClassInfo FUNCTIONS
	void SetCIPointer();
	int GetCIEImageHandle(int id, int handleid=0){if(InfoEntry!=NULL){return InfoEntry->GetImageHandle(id, handleid);}else{return -1;}}
	int GetCIECoords(int id, int *x, int *y);
public:
	//CONSTRUCTION/DESTRUCTION
	//Derive your class's constructer from the GUIObject one
	GUIObject(std::string in_Name="", int x_coord = 0, int y_coord = 0, int x_width = -1, int y_height = -1, int in_style = 0);
	~GUIObject();
	void Delete();

	//CHILD FUNCTIONS
	//Used for managing children. :)
	GUIObject *AddChild(GUIObject* cgp);
	void DeleteChildren(GUIObject* exception = NULL);

	//SET FUNCTIONS
	void SetPosition(int x, int y);
	void SetCloseFunction(void (*in_closefunc)(GUIObject* caller)){CloseFunction = in_closefunc;}

	//GET FUNCTIONS
	int GetWidth(){return Coords[2]-Coords[0];}
	int GetHeight(){return Coords[3]-Coords[1];}
};

//*****************************GUIScreen*******************************
#define GSOF_NOTHINGPRESSED			-1
#define GSOF_SOMETHINGPRESSED		-2
class GUIScreen : public LinkedList
{
	friend class GUISystem;
private:
	std::string Name;

	GUISystem* OwnerSystem;

	bool Active;	//Is this screen active?

	ScreenClassInfoEntry* ScreenClassInfo;
	GUIObject Guiobjects;
	SCP_vector<GUIObject*> DeletionCache;
public:
	GUIScreen(std::string in_Name="");
	~GUIScreen();

	ObjectClassInfoEntry *GetObjectClassInfo(GUIObject *cgp);

	//Set funcs
	GUIObject *Add(GUIObject* new_gauge);
	void DeleteObject(GUIObject* dgp);

	//On funcs
	int OnFrame(float frametime, bool doevents);
};

//*****************************GUISystem*******************************
class GUISystem
{
	friend class GUIScreen;	//I didn't want to do this, but it's the easiest way
							//to keep it from being confusing about how to remove GUIScreens
private:
	GUIScreen Screens;
	GUIObject* ActiveObject;

	//Moving stuff
	GUIObject* GraspedGuiobject;
	int GraspingButton;	//Button flag for button used to grasp the object
	int GraspedDiff[2];	//Diff between initial mouse position and object corner

	//Linked list of screen class info
	bool ClassInfoParsed;
	ScreenClassInfoEntry ScreenClassInfo;

	//Mouse/status
	int MouseX;
	int MouseY;
	int KeyPressed;
	int Status, LastStatus;

	void DestroyClassInfo();
public:
	GUISystem();
	~GUISystem();

	//-----
	GUIScreen* PushScreen(GUIScreen *csp);
	void PullScreen(GUIScreen *in_screen);
	ScreenClassInfoEntry *GetClassInfo(){return &ScreenClassInfo;}
	ScreenClassInfoEntry *GetScreenClassInfo(const std::string & screen_name);
	//-----

	//Set stuff
	void ParseClassInfo(char* section);
	void SetActiveObject(GUIObject *cgp);
	void SetGraspedObject(GUIObject *cgp, int button);

	//Get stuff
	int GetMouseX(){return MouseX;}
	int GetMouseY(){return MouseY;}
	int GetStatus(){return Status;}
	//int *GetLimits(){return Guiobjects.ChildCoords;}
	GUIObject* GetActiveObject(){return ActiveObject;}
	GUIObject* GetGraspedObject(){return GraspedGuiobject;}
	int GetKeyPressed(){return KeyPressed;}

	int OnFrame(float frametime, bool doevents, bool clearandflip);
};

//*****************************Window*******************************
#define W_BORDERWIDTH			1
#define W_BORDERHEIGHT			1

#define WS_NOTITLEBAR		(1<<31)		// doesn't have a title bar (ie, no title or min/close buttons)
#define WS_NONMOVEABLE		(1<<30)		// can't be moved around

#define WCI_CAPTION				0
#define WCI_CAPTION_TEXT		1
#define WCI_BORDER				2
#define WCI_BODY				3
#define WCI_HIDE				4
#define WCI_CLOSE				5
#define WCI_COORDS				6
#define WCI_NUM_ENTRIES			7

class Window : public GUIObject
{
	std::string Caption;

	//Close
	bool CloseHighlight;
	int CloseCoords[4];

	//Hide
	bool HideHighlight;
	int HideCoords[4];

	//Caption text
	int CaptionCoords[4];

	//Old height
	int UnhiddenHeight;

	//Skinning stuff
	//Left width, top height, right width, bottom height
	int BorderSizes[4];
	bitmap_rect_list BorderRectLists[8];
	bitmap_rect_list CaptionRectList;

	shader WindowShade;

protected:
	void DoDraw(float frametime);
	void DoMove(int dx, int dy);
	int DoRefreshSize();
	int DoMouseOver(float frametime);
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
	int DoMouseOut(float frametime);
	bool HasChildren(){return NOT_EMPTY(&Children);}

public:
	Window(std::string in_caption, int x_coord, int y_coord, int x_width = -1, int y_height = -1, int in_style = 0);
	void SetCaption(std::string in_caption){Caption = in_caption;}
	void ClearContent();
};

//*****************************Button*******************************
//#define DEFAULT_BUTTON_WIDTH	50
#define B_BORDERWIDTH			1
#define B_BORDERHEIGHT			1
#define DEFAULT_BUTTON_HEIGHT	15

#define BS_STICKY			(1<<31)	//Button stays pressed

#define BCI_COORDS			0
#define BCI_BUTTON			1
#define BCI_NUM_ENTRIES		2

class Button : public GUIObject
{
	std::string Caption;
	void (*function)(Button *caller);

	bool IsDown;	//Does it look pressed?

protected:
	void DoDraw(float frametime);
	int DoRefreshSize();
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
	int DoMouseOut(float frametime);
public:
	Button(std::string in_caption, int x_coord, int y_coord, void (*in_function)(Button *caller) = NULL, int x_width = -1, int y_height = -1, int in_style = 0);

	void SetPressed(bool in_isdown){IsDown = in_isdown;}
};

//*****************************Tree*******************************

//In pixels
#define TI_BORDER_WIDTH					1
#define TI_BORDER_HEIGHT				1
#define TI_INITIAL_INDENT				2
#define TI_INITIAL_INDENT_VERTICAL		2
#define TI_INDENT_PER_LEVEL				10
#define	TI_SPACE_BETWEEN_VERTICAL		2

// forward declaration
class Tree;

struct TreeItem : public LinkedList
{
	friend class Tree;
private:
	std::string Name;
	void (*Function)(Tree *caller);
	int Data;

	bool DeleteData;	//Do we delete data for the user?
	bool ShowThis;
	bool ShowChildren;

	int Coords[4];	//For hit testing

	TreeItem *Parent;
	LinkedList Children;
public:
	//Get
	TreeItem * GetParentItem(){return Parent;}
	int GetData(){return Data;}
	bool HasChildren(){return NOT_EMPTY(&Children);}
	
	void ClearAllItems();
	
	TreeItem();
	~TreeItem();
};

class Tree : public GUIObject
{
	TreeItem Items;
	void *AssociatedItem;

	int StartLine;
	TreeItem *SelectedItem;
	TreeItem *HighlightedItem;

	TreeItem* HitTest(TreeItem *items);

	void MoveTreeItems(int dx, int dy, TreeItem *items);
	void CalcItemsSize(TreeItem *items, int *DrawData);
	void DrawItems(TreeItem *items);
protected:
	void DoDraw(float frametime);
	void DoMove(int dx, int dy);
	int DoRefreshSize();
	int DoMouseOver(float frametime);
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
public:
	Tree(std::string in_name, int x_coord, int y_coord, void* in_associateditem = NULL, int x_width = -1, int y_width = -1, int in_style = 0);

	//void LoadItemList(TreeItem *in_list, unsigned int count);
	TreeItem* AddItem(TreeItem *parent, std::string in_name, int in_data = 0, bool in_delete_data = true, void (*in_function)(Tree *caller) = NULL);
	void ClearItems();

	TreeItem* GetSelectedItem(){return SelectedItem;}
};

//*****************************Text*******************************
#define MAX_TEXT_LINES		100
#define T_EDITTABLE			(1<<31)

//What type?
#define T_ST_NONE			0		//No saving
#define T_ST_INT			(1<<0)
#define T_ST_SINT			(1<<1)
#define T_ST_CHAR			(1<<2)
#define T_ST_FLOAT			(1<<3)
#define T_ST_UBYTE			(1<<4)

//When do we save changes?
#define T_ST_ONENTER		(1<<21)
#define T_ST_CLOSE			(1<<22)
#define T_ST_REALTIME		(1<<23)

//If dynamically allocated, then how?
#define T_ST_NEW			(1<<30)	//Allocated using new
#define T_ST_MALLOC			(1<<31)	//Allocated using malloc

class Text : public GUIObject
{
	std::string Content;

	//Used to display stuff; change only from calculate func
	int NumLines;
	int LineLengths[MAX_TEXT_LINES];
	char *LineStartPoints[MAX_TEXT_LINES];

	//Used for editing
	int CursorPos;	//Line #, then position in line
	int SaveType;
	union
	{
		short *siSavePointer;
		int *iSavePointer;
		ubyte *ubSavePointer;
		float *flSavePointer;
		char *chSavePointer;
		char **chpSavePointer;
	};
	union{int SaveMax;float flSaveMax;uint uSaveMax;};
	union{int SaveMin;float flSaveMin;uint uSaveMin;};

protected:
	void DoDraw(float frametime);
	int DoRefreshSize();
	int DoMouseDown(float frametime);
	int DoKeyPress(float frametime);
public:
	Text(std::string in_name, std::string in_content, int x_coord, int y_coord, int x_width = -1, int y_width = -1, int in_style = 0);

	//Set
	void SetText(std::string in_content);
	void SetText(int the_int);
	void SetText(float the_float);
	void SetSaveLoc(int *ptr, int save_method, int max_value=INT_MAX, int min_value=INT_MIN);
	void SetSaveLoc(short int *sint_ptr, int save_method, short int max_value=SHRT_MAX, short int min_value=SHRT_MIN);
	void SetSaveLoc(float *ptr, int save_method, float max_value=INT_MAX, float min_value=INT_MIN);
	void SetSaveLoc(char *ptr, int save_method, uint max_len=UINT_MAX, uint min_len = 0);
	void SetSaveLoc(ubyte *ptr, int save_method, int max_value=UCHAR_MAX, int min_value=0);
	void SetSaveStringAlloc(char **ptr, int save_method, int mem_flags, uint max_len=UINT_MAX, uint min_len = 0);
	void AddLine(std::string in_line);

	//Get?
	bool Save();
	void Load();
};

//*****************************Checkbox*******************************
#define CB_TEXTCHECKDIST	2

class Checkbox : public GUIObject
{
	std::string Label;
	void (*function)(Checkbox *caller);

	//For toggling flags with this thing
	int* FlagPtr;
	int Flag;

	bool *BoolFlagPtr;

	int CheckCoords[4];
	bool IsChecked;	//Is it checked?
	int HighlightStatus;

protected:
	void DoDraw(float frametime);
	void DoMove(int dx, int dy);
	int DoRefreshSize();
	int DoMouseOver(float frametime);
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
	int DoMouseOut(float frametime);

public:
	Checkbox(std::string in_label, int x_coord, int y_coord, void (*in_function)(Checkbox *caller) = NULL, int x_width = -1, int y_height = DEFAULT_BUTTON_HEIGHT, int in_style = 0);

	bool GetChecked() {
		return IsChecked;
	}

	void SetLabel(std::string in_label) {
		Label = in_label;
	}

	void SetChecked(bool in_ischecked) {
		IsChecked = in_ischecked;
	}

	void SetFlag(int* in_flag_ptr, int in_flag) {
		FlagPtr = in_flag_ptr;
		Flag = in_flag;

		if ( (FlagPtr != NULL) && (*FlagPtr & Flag) ) {
			IsChecked = true;
		}
	}

	void SetFlag(uint* in_flag_ptr, int in_flag) {
		SetFlag((int*)in_flag_ptr, in_flag);
	}

	void SetBool(bool *in_bool_ptr) {
		BoolFlagPtr = in_bool_ptr;
	}
};

//*****************************ImageAnim*******************************
#define PT_STOPPED			0
#define PT_PLAYING			1
#define PT_PLAYING_REVERSE	2
#define PT_STOPPED_REVERSE	3

#define IF_NONE				0
#define IF_BOUNCE			1
#define IF_REPEAT			2
#define IF_REVERSED			3

class ImageAnim : public GUIObject
{
	std::string ImageAnimName;

	IMG_HANDLE ImageHandle;
	int TotalFrames;
	int FPS;
	bool IsSet; //Something of a hack, this is called so that
				//SetImage overrides skin image

	float Progress;
	float ElapsedTime;
	float TotalTime;

	int PlayType;
	int ImageFlags;
protected:
	void DoDraw(float frametime);
	int DoRefreshSize();
public:
	ImageAnim(std::string in_name, std::string in_imagename, int x_coord, int y_coord, int x_width = -1, int y_width = -1, int in_style = 0);

	void SetImage(std::string in_imagename);
	void Play(bool in_isreversed);
	void Pause();
	void Stop();
};

//*****************************GLOBALS*******************************
extern GUISystem GUI_system;
