#include "globalincs/alphacolors.h"
#include "io/mouse.h"
#include <string>
#include <vector>

extern color Color_dark_blue;

//*****************************LinkedList*******************************
struct LinkedList
{
	friend class GUISystem;
	friend class GUIObject;
	friend class Tree;
	friend struct TreeItem;
	friend class Window;
protected:
	struct LinkedList *next, *prev;

	LinkedList(){next=this;prev=this;}
	virtual ~LinkedList(){prev->next=next;next->prev=prev;}
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

//NMCAD Handles
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

//Global stuff
#define CIE_NUM_HANDLES	8
union Handle
{
	ubyte Colors[4];
	int Image;
};
class ClassInfoEntry
{
	int Type;

	Handle Handles[CIE_NUM_HANDLES];	//We need 8 for border
	int Coords[2];

public:
	//--CONSTRUCTORS
	ClassInfoEntry();
	~ClassInfoEntry();

	//--SET FUNCTIONS
	void Parse(char* tag, int in_type);

	//--GET FUNCTIONS
	int GetImageHandle(int ID=CIE_HANDLE_N){if(this==NULL){return-1;}else{return Handles[ID].Image;}}
	ubyte GetColorHandle(int ColorID, int ID=CIE_HANDLE_N){if(this==NULL){return 0;}else{return Handles[ID].Colors[ColorID];}}
	//Copies the coordinates to the given location if coordinates are set.
	int GetCoords(int *x, int *y);
};

//*****************************GUIObject*******************************
//What type a GUIObject is, mostly for debugging
#define GT_NONE					0
#define GT_WINDOW				1
#define GT_BUTTON				2
#define GT_MENU					3
#define GT_TEXT					4
#define	GT_CHECKBOX				5
#define GT_NUM_TYPES			6	//Total number of types

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

//DoFrame return values
#define OF_TRUE					-1
#define OF_FALSE				-2
#define OF_DESTROYED			-3	//If a call to DoFrame results in the object destroying itself
									//(ie the close button was pressed)

class GUIObject : public LinkedList
{
	friend class Window;			//Hack, because I can't figure out how to let it access protected
	friend class Menu;				//This too
	friend class Text;				//And this
	friend class Tree;				//By, the way...THIS
	friend class Checkbox;
	friend class Button;
	friend class GUISystem;
private:
	class GUISystem*	Globals;			//What system this object is associated with
	int Type;
	int Coords[4];					//Upper left corner (x, y) and lower right corner (x, y)
	int	ChildCoords[4];				//Coordinates where children may frolick
	int LastStatus;
	int Status;
	int Style;

	class ClassInfoEntry* InfoEntry;

	void (*CloseFunction)(GUIObject *caller);

	class GUIObject* Parent;
	LinkedList Children;

	void DeleteChildren(GUIObject* exception = NULL);
protected:
	//ON FUNCTIONS
	//These handle the calling of do functions, mostly.
	void OnDraw(float frametime);
	int OnFrame(float frametime, int *unused_queue);
	void OnMove(int dx, int dy);

	//DO FUNCTIONS
	//Used by individual objects to define actions when that event happens
	virtual void DoDraw(float frametime){}
	virtual int DoFrame(float frametime){return OF_FALSE;}
	virtual void DoMove(int dx, int dy){}
	virtual int DoMouseOver(float frametime){return OF_FALSE;}
 	virtual int DoMouseDown(float frametime){return OF_FALSE;} 
	virtual int DoMouseUp(float frametime){return OF_FALSE;}	//In other words, a click
	virtual int DoMouseOut(float frametime){return OF_FALSE;}
	virtual int DoKeyState(float frametime){return OF_FALSE;}
	virtual int DoKeyPress(float frametime){return OF_FALSE;}

	//CALCULATESIZE
	//Sort of an on and do function; if you define your own, the following MUST be included at the end:
	//"if(Parent!=NULL)Parent->CalculateSize();"
	virtual void CalculateSize(){if(Parent!=NULL)Parent->CalculateSize();}

	//PRIVATE CIE FUNCTIONS
	int GetCIEImageHandle(int id, int handleid=0){if(InfoEntry!=NULL){return InfoEntry[id].GetImageHandle(handleid);}else{return -1;}}
	int GetCIECoords(int id, int *x, int *y);
	void SetCIEHandle();
public:
	//CONSTRUCTION/DESTRUCTION
	//Derive your class's constructer from the GUIObject one
	GUIObject(int x_coord = 0, int y_coord = 0, int x_width = -1, int y_height = -1, int in_style = 0);
	~GUIObject();

	//CHILD FUNCTIONS
	//Used for managing children. :)
	GUIObject *AddChild(GUIObject* cgp);

	//SET FUNCTIONS
	void SetPosition(int x, int y);
	void SetCloseFunction(void (*in_closefunc)(GUIObject* caller)){CloseFunction = in_closefunc;}

	//GET FUNCTIONS
	int GetWidth(){return Coords[2]-Coords[0];}
	int GetHeight(){return Coords[3]-Coords[1];}
};

//*****************************GUISystem*******************************
#define GSDF_NOTHINGPRESSED			-1
#define GSDF_SOMETHINGPRESSED		-2
class GUISystem
{
	GUIObject Guiobjects;
	GUIObject* ActiveObject;

	//Moving stuff
	GUIObject* GraspedGuiobject;
	int GraspingButton;	//Button flag for button used to grasp the object
	int GraspedDiff[2];	//Diff between initial mouse position and object corner

	//Class info (ie skinning)
	class ClassInfoEntry *ClassInfo[GT_NUM_TYPES];

	//Mouse/status
	int MouseX;
	int MouseY;
	int KeyPressed;
	int Status, LastStatus;

	void ParseClassInfo(char* section);
	void DestroyClassInfo();
public:
	GUISystem(char *section);
	~GUISystem();

	//-----
	ClassInfoEntry *GetClassInfo(int in_class){return ClassInfo[in_class];}
	//-----

	//Set stuff
	GUIObject *Add(GUIObject* cgp);
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

#define WS_NOTITLEBAR		(1<<31)
#define WS_NONMOVEABLE		(1<<30)

#define WCI_CAPTION				0
#define WCI_CAPTION_TEXT		1
#define WCI_BORDER				2
#define WCI_BODY				3
#define WCI_HIDE				4
#define WCI_CLOSE				5
#define WCI_NUM_ENTRIES			6

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
	//Top left, top right, bottom left, bottom right
	int CornerWidths[4];
	//Left width, top height, right width, bottom height
	int BorderSizes[4];
	std::vector<bitmap_rect_list> BorderRectLists[8];

	//Functions
	void CalculateSize();
protected:
	int DoMouseOver(float frametime);
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
	int DoMouseOut(float frametime);
	void DoMove(int dx, int dy);
	void DoDraw(float frametime);
public:
	Window(std::string in_caption, int x_coord, int y_coord, int x_width = -1, int y_height = -1, int in_style = 0);
	void SetCaption(std::string in_caption){Caption = in_caption;}
};

//*****************************Button*******************************
//#define DEFAULT_BUTTON_WIDTH	50
#define B_BORDERWIDTH			1
#define B_BORDERHEIGHT			1
#define DEFAULT_BUTTON_HEIGHT	15

#define BS_STICKY			(1<<31)	//Button stays pressed

class Button : public GUIObject
{
	std::string Caption;
	void (*function)(Button *caller);

	bool IsDown;	//Does it look pressed?

	void CalculateSize();
protected:
	void DoDraw(float frametime);
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
	int DoMouseOut(float frametime);
public:
	Button(std::string in_caption, int x_coord, int y_coord, void (*in_function)(Button *caller) = NULL, int x_width = -1, int y_height = -1, int in_style = 0);

	void SetPressed(bool in_isdown){IsDown = in_isdown;}
};

//*****************************Menu*******************************

//In pixels
#define TI_BORDER_WIDTH					1
#define TI_BORDER_HEIGHT				1
#define TI_INITIAL_INDENT				2
#define TI_INITIAL_INDENT_VERTICAL		2
#define TI_INDENT_PER_LEVEL				10
#define	TI_SPACE_BETWEEN_VERTICAL		2

struct TreeItem : public LinkedList
{
	std::string Name;
	void (*Function)(Tree *caller);
	void *Data;

	bool DeleteData;	//Do we delete data for the user?
	bool ShowThis;
	bool ShowChildren;

	int Coords[4];	//For hit testing

	TreeItem *Parent;
	LinkedList Children;

	TreeItem();
	~TreeItem();
};

class Tree : public GUIObject
{
	TreeItem Items;
	void *AssociatedItem;

	TreeItem *SelectedItem;
	TreeItem *HighlightedItem;

	void CalculateSize();
	TreeItem* HitTest(TreeItem *items);

	void MoveTreeItems(int dx, int dy, TreeItem *items);
	void CalcItemsSize(TreeItem *items, int *DrawData);
	void DrawItems(TreeItem *items);
protected:
	void DoDraw(float frametime);
	int DoMouseOver(float frametime);
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
	void DoMove(int dx, int dy);
public:
	Tree(int x_coord, int y_coord, void* in_associateditem = NULL, int x_width = -1, int y_width = -1, int in_style = 0);

	//void LoadItemList(TreeItem *in_list, unsigned int count);
	TreeItem* AddItem(TreeItem *parent, std::string in_name, void *in_data = NULL, bool in_delete_data = true, void (*in_function)(Tree *caller) = NULL);

	TreeItem* GetSelectedItem(){return SelectedItem;}
};

//*****************************Text*******************************
#define MAX_TEXT_LINES		100
#define T_EDITTABLE			(1<<31)

//What type?
#define T_ST_NONE			0		//No saving
#define T_ST_INT			(1<<0)
#define T_ST_CHAR			(1<<1)
#define T_ST_FLOAT			(1<<2)
#define T_ST_UBYTE			(1<<3)

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
		int *iSavePointer;
		ubyte *ubSavePointer;
		float *flSavePointer;
		char *chSavePointer;
		char **chpSavePointer;
	};
	union{int SaveMax;float flSaveMax;uint uSaveMax;};
	union{int SaveMin;float flSaveMin;uint uSaveMin;};

	void CalculateSize();
protected:
	int DoMouseDown(float frametime);
	int DoKeyPress(float frametime);
	void DoDraw(float frametime);
public:
	Text(std::string in_content, int x_coord, int y_coord, int x_width = -1, int y_width = -1, int in_style = 0);

	//Set
	void SetText(std::string in_content);
	void SetText(int the_int);
	void SetText(float the_float);
	void SetSaveLoc(int *ptr, int save_method, int max_value=INT_MAX, int min_value=INT_MIN);
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

	int CheckCoords[4];
	bool IsChecked;	//Is it checked?
	int HighlightStatus;

	void CalculateSize();
protected:
	void DoMove(int dx, int dy);
	void DoDraw(float frametime);
	int DoMouseOver(float frametime);
	int DoMouseDown(float frametime);
	int DoMouseUp(float frametime);
	int DoMouseOut(float frametime);
public:
	Checkbox(std::string in_label, int x_coord, int y_coord, void (*in_function)(Checkbox *caller) = NULL, int x_width = -1, int y_height = DEFAULT_BUTTON_HEIGHT, int in_style = 0);

	bool GetChecked(){return IsChecked;}

	void SetLabel(std::string in_label){Label = in_label;}
	void SetChecked(bool in_ischecked){IsChecked = in_ischecked;}
	void SetFlag(int* in_flag_ptr, int in_flag){FlagPtr = in_flag_ptr;Flag = in_flag;if(FlagPtr != NULL && (*FlagPtr & Flag))IsChecked=true;}
	void SetFlag(uint* in_flag_ptr, int in_flag){SetFlag((int*)in_flag_ptr, in_flag);}
};
