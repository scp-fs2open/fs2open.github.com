#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "parse/lua.h"

#include <stdio.h>

//**********Scripting languages that are possible
#define SC_LUA			(1<<0)

//*************************Scripting structs*************************
#define SCRIPT_END_LIST		NULL

struct image_desc
{
	char fname[MAX_FILENAME_LEN];
	int handle;
};

//**********Main Conditional Hook stuff

#define MAX_HOOK_CONDITIONS	8

//Conditionals
#define CHC_NONE			-1
#define CHC_MISSION			0
#define CHC_SHIP			1
#define CHC_SHIPCLASS		2
#define CHC_SHIPTYPE		3
#define CHC_STATE			4
#define CHC_CAMPAIGN		5
#define CHC_WEAPONCLASS		6
#define CHC_OBJECTTYPE		7
#define CHC_KEYPRESS		8
#define CHC_ACTION			9
#define CHC_VERSION			10
#define CHC_APPLICATION		11

//Actions
#define CHA_NONE			-1
#define CHA_WARPOUT			0
#define CHA_WARPIN			1
#define CHA_DEATH			2
#define CHA_ONFRAME			3
#define CHA_COLLIDESHIP		4
#define CHA_COLLIDEWEAPON	5
#define CHA_COLLIDEDEBRIS	6
#define CHA_COLLIDEASTEROID	7
#define CHA_HUDDRAW			8
#define CHA_OBJECTRENDER	9
#define CHA_SPLASHSCREEN	10
#define CHA_GAMEINIT		11
#define CHA_MISSIONSTART	12
#define CHA_MISSIONEND		13
#define CHA_MOUSEMOVED		14
#define CHA_MOUSEPRESSED	15
#define CHA_MOUSERELEASED	16
#define CHA_KEYPRESSED		17
#define CHA_KEYRELEASED		18
#define CHA_ONSTATESTART	19
#define CHA_ONSTATEEND		20
#define CHA_ONWEAPONDELETE	21
#define CHA_ONWPEQUIPPED	22
#define CHA_ONWPFIRED		23
#define CHA_ONWPSELECTED	24
#define CHA_ONWPDESELECTED	25
#define CHA_GAMEPLAYSTART	26
#define CHA_ONTURRETFIRED	27
#define CHA_PRIMARYFIRE		28
#define CHA_SECONDARYFIRE	29
#define CHA_ONSHIPARRIVE	30
#define CHA_COLLIDEBEAM		31
#define CHA_ONACTION		32
#define CHA_ONACTIONSTOPPED	33
#define CHA_MSGRECEIVED		34
#define CHA_HUDMSGRECEIVED	35

// management stuff
void scripting_state_init();
void scripting_state_close();
void scripting_state_do_frame(float frametime);

class script_condition
{
public:
	int condition_type;
	union
	{
		char name[CONDITION_LENGTH];
	} data;

	script_condition()
		: condition_type(CHC_NONE)
	{
		memset(data.name, 0, sizeof(data.name));
	}
};

class script_action
{
public:
	int action_type;
	script_hook hook;

	script_action()
		: action_type(CHA_NONE)
	{
		script_hook_init(&hook);
	}
};

class ConditionedHook
{
private:
	SCP_vector<script_action> Actions;
	script_condition Conditions[MAX_HOOK_CONDITIONS];
public:
	bool AddCondition(script_condition *sc);
	bool AddAction(script_action *sa);

	bool ConditionsValid(int action, class object *objp=NULL, int more_data = 0);
	bool IsOverride(class script_state *sys, int action);
	bool Run(class script_state *sys, int action, char format='\0', void *data=NULL);
};

//**********Main script_state function
class script_state
{
private:
	char StateName[32];

	int Langs;
	struct lua_State *LuaState;
	const struct script_lua_lib_list *LuaLibs;

	//Utility variables
	SCP_vector<image_desc> ScriptImages;
	SCP_vector<ConditionedHook> ConditionalHooks;

private:

	void ParseChunkSub(int *out_lang, int *out_index, char* debug_str=NULL);
	int RunBytecodeSub(int in_lang, int in_idx, char format='\0', void *data=NULL);

	void SetLuaSession(struct lua_State *L);

	void OutputLuaMeta(FILE *fp);
	
	//Lua private helper functions
	bool OpenHookVarTable();
	bool CloseHookVarTable();

	//Internal Lua helper functions
	void EndLuaFrame();

	//Destroy everything
	void Clear();

public:
	//***Init/Deinit
	script_state(char *name);
	script_state& operator=(script_state &in);
	~script_state();

	//***Internal scripting stuff
	int LoadBm(char *name);
	void UnloadImages();

	lua_State *GetLuaSession(){return LuaState;}

	//***Init functions for langs
	int CreateLuaState();

	//***Get data
	int OutputMeta(char *filename);

	//***Moves data
	//void MoveData(script_state &in);

	//***Variable handling functions
	bool GetGlobal(char *name, char format='\0', void *data=NULL);
	void RemGlobal(char *name);

	void SetHookVar(char *name, char format, void *data=NULL);
	void SetHookObject(char *name, object *objp);
	void SetHookObjects(int num, ...);
	bool GetHookVar(char *name, char format='\0', void *data=NULL);
	void RemHookVar(char *name);
	void RemHookVars(unsigned int num, ...);

	//***Hook creation functions
	bool EvalString(const char *string, const char *format=NULL, void *rtn=NULL, const char *debug_str=NULL);
	void ParseChunk(script_hook *dest, char* debug_str=NULL);
	bool ParseCondition(const char *filename="<Unknown>");

	//***Hook running functions
	int RunBytecode(script_hook &hd, char format='\0', void *data=NULL);
	bool IsOverride(script_hook &hd);
	int RunCondition(int condition, char format='\0', void *data=NULL, class object *objp = NULL, int more_data = 0);
	bool IsConditionOverride(int action, object *objp=NULL);

	//*****Other functions
	void EndFrame();
};


//**********Script registration functions
void script_init();

//**********Script globals
extern class script_state Script_system;
extern bool Output_scripting_meta;

//**********Script hook stuff (scripting.tbl)
extern script_hook Script_globalhook;
extern script_hook Script_simulationhook;
extern script_hook Script_hudhook;
extern script_hook Script_splashhook;
extern script_hook Script_gameinithook;

//*************************Conditional scripting*************************

#endif //_SCRIPTING_H
