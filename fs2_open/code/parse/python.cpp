#ifdef USE_PYTHON
#include "parse/python.h"
#include "parse/parselo.h"
#include "localization\localize.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "gamesequence/gamesequence.h"
#include "ship/ship.h"
#include <vector>

//*************************GLOBALS*************************
PyObject *Python_global_dict;
int Python_initted = 0;

static int Total_lines_parsed = 0;
static int Total_blocks_parsed = 0;

//*************************Python helpers*************************

#ifdef USE_PYTHON
std::vector<PyBytecode> Python_chunks;

//Note that PyBytecodelib should be a linked list or btree(?) in the future
class PyBytecodelib
{
private:
	std::vector<PyBytecode> Python_chunks;
public:
	int Add(PyBytecode &in);
	PyBytecode *Get(int idx);
}

//Returns the index in the lib, or -1 if invalid
int PyBytecodelib::Add(PyBytecode &in)
{
	if(in.Get() != NULL)
	{
		Python_chunks.push_back(in);
		return Python_chunks.size()-1;
	}
	else
	{
		return -1;
	}
}

//Returns pointer to the item, or NULL if invalid index
PyBytecode *PyBytecodelib::Get(int idx)
{
	Assert(idx > -1);
	if(idx < Python_chunks.size())
	{
		return &Python_chunks[idx];
	}

	return NULL;
}

//TODO: Make this a linked list
PyBytecodelib PyBytecodeLib;
#endif

//*************************Types*************************
//Returns true and a value to dest if possible
//Returns false and sets dest to 0 on failure/unsupported type
bool PyResult::GetLong(long *dest)
{
	if(rslt==NULL)
	{
		*dest = 0;
		return false;
	}
	else if(PyString_Check(rslt))
	{
		char buf[32];
		GetString(buf, sizeof(buf));
		*dest = atoi(buf);
		return true;
	}
	else
	{
		*dest = PyInt_AsLong(rslt);
		return true;
	}
}

//Returns true and a value to dest if possible
//Returns false and sets dest to 0 on failure/unsupported type
bool PyResult::GetDouble(double *dest)
{
	if(rslt==NULL)
	{
		*dest = 0;
		return false;
	}
	else if(PyString_Check(rslt))
	{
		char buf[32];
		GetString(buf, sizeof(buf));
		*dest = atof(buf);
		return true;
	}
	else
	{
		*dest = PyFloat_AsDouble(rslt);
		return true;
	}
}


//Returns num_chars and a string to dest if possible
//Returns false and sets dests to 0 on failure/unsupported type
int PyResult::GetString(char *dest, int dest_size)
{
	if(rslt == NULL)
	{
		strcpy(dest, "");
		return 0;
	}
	else if(PyInt_Check(rslt))
	{
		sprintf(dest,"%d",PyInt_AsLong(rslt));
		return strlen(dest);
	}
	else if(PyFloat_Check(rslt))
	{
		sprintf(dest,"%f",PyFloat_AsDouble(rslt));
		return strlen(dest);
	}
	else
	{
		int len = PyString_Size(rslt);
		if(len > -1)
		{
			strncpy(dest, PyString_AsString(rslt), dest_size-1);
			if(len >= dest_size)
				dest[dest_size-1] = '\0';
			else
				dest[len] = '\0';

			return len;
		}
		else
		{
			strcpy(dest, "");
			return 0;
		}
	}
}

//*************************Libraries*************************
/*
typedef struct {
	PyObject_HEAD
	int ship_index;
} py_ship;

//Handles any initial arguments (causes segmentation fault right now)
static int py_class_ship_init(py_vec *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {"x", "y", "z", NULL};
	
	if(!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &self->x, &self->y, &self->z))
		return -1;
	
	return 0;
}


//Member function that puts the data in a format of type "{x y z}"
static PyObject* pyvec_string(py_vec *self)
{
	static PyObject *format = NULL;
	PyObject *args, *result;
	
	if(format == NULL)
	{
		format = PyString_FromString("{%f %f %f}");
		if(format == NULL)
			return NULL;
	}

	Error(LOCATION, "Vector: %f %f %f", self->x, self->y, self->z);
	
	args = Py_BuildValue("fff", self->x, self->y, self->z);
	if(args == NULL)
		return NULL;
		
	result = PyString_Format(format, args);
	Py_DECREF(args);
		
	return result;
}

//Methods table
static PyMethodDef pyvec_methods[] = {
	{"string", (PyCFunction)pyvec_string, METH_NOARGS, "return the vector in string format"},
	{NULL}	//sentinel?
};

//Defines variables accessible
//"Geteric Attribute Management" has access stuff
static PyMemberDef pyvec_members[] = {
	{NULL}
};

static PyTypeObject pyvecType = {
	PyObject_HEAD_INIT(NULL)
	0,				//ob_size
	"noddy.vec",			//tp_name
	sizeof(py_ship),			//tp_basicsize
	0,				//tp_itemsize
	0,				//tp_dealloc
	0,				//tp_print
	0,				//tp_getattr
	0,				//tp_setattr
	0,				//tp_compare
	0,				//tp_repr
	0,				//tp_as_number
	0,				//tp_as_sequence
	0,				//tp_as_mapping
	0,				//tp_hash
	0,				//tp_call
	0,				//tp_str
	0,				//tp_getattro
	0,				//tp_setattro
	0,				//tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		//tp_flags
	"Ship object",			//tp_doc
	0,				//tp_traverse
	0,				//tp_clear
	0,				//tp_richcompare
	0,				//tp_weaklistoffset
	0,				//tp_iter
	0,				//tp_iternext
	pyvec_methods,			//tp_methods
	pyvec_members,			//tp_members
	0,				//tp_getset
	0,				//tp_base
	0,				//tp_dict
	0,				//tp_descr_get
	0,				//tp_descr_set
	0,				//tp_dictoffset
	(initproc)pyvec_init,		//tp_init
	0,				//tp_alloc
	py_class_ship_new,				//tp_new
};

PyMODINIT_FUNC initpyvec(void)
{
	if(PyType_Ready(&pyvecType) < 0)
		return;
	
	Py_INCREF(&pyvecType);
}
*/
extern int Gr_inited;
static PyObject* py_lib_gr_set_color(PyObject *self, PyObject *args)
{
	if(!Gr_inited)
		return Py_BuildValue("i",0);

	int rgba[4]={255,255,255,255};

	PyArg_ParseTuple(args, "iii|i", &rgba[0], &rgba[1], &rgba[2], &rgba[3]);

	//Check
	for(int i = 0; i < 4; i++)
	{
		if(rgba[i] > 255)
			rgba[i] = 255;
		if(rgba[i] < 0)
			rgba[i] = 0;
	}

	//Do
	color clr;
	gr_init_alphacolor(&clr, rgba[0], rgba[1], rgba[2], rgba[3]);
	gr_set_color_fast(&clr);

	return Py_BuildValue("i",1);
}

static PyObject* py_lib_gr_line(PyObject *self, PyObject *args)
{
	if(!Gr_inited)
		return Py_BuildValue("i",0);

	int x1=0,y1=0,x2=0,y2=0;

	PyArg_ParseTuple(args, "iiii", &x1, &y1, &x2, &y2);
	gr_line(x1,y1,x2,y2);

	return Py_BuildValue("i",1);
}

static PyObject* py_lib_gr_string(PyObject *self, PyObject *args)
{
	if(!Gr_inited)
		return Py_BuildValue("i",0);

	int x=0,y=0;
	char *s = NULL;

	PyArg_ParseTuple(args, "iis", &x, &y, &s);

	if(s != NULL)
		gr_string(x,y,s);
	else
		return Py_BuildValue("i",0);

	return Py_BuildValue("i",1);
}

static std::vector<int> PythImageList;

static void py_add_image(int idx)
{
	if(idx == -1) return;

	for(uint i = 0; i < PythImageList.size(); i++)
	{
		if(PythImageList[i] == idx)
			return;
	}

	PythImageList.push_back(idx);
}

void py_clear_images()
{
	for(uint i = PythImageList.size()-1; i >= 0; i--)
	{
		bm_unload(PythImageList[i]);
	}
}

static PyObject* py_lib_gr_image(PyObject *self, PyObject *args)
{
	if(!Gr_inited)
		return Py_BuildValue("i",0);

	int x=0,y=0;
	char *s = NULL;

	PyArg_ParseTuple(args, "iis", &x, &y, &s);

	if(s == NULL)
		return Py_BuildValue("i",0);

	int idx = bm_load(s);

	if(idx != -1)
	{
		py_add_image(idx);
		gr_set_bitmap(idx);
		gr_bitmap(x,y);
	}
	else
		return Py_BuildValue("i",0);

	return Py_BuildValue("i",1);
}

static PyMethodDef GrMethods[] = {
	{"setColor", py_lib_gr_set_color, METH_VARARGS, "Sets a color"},
    {"drawLine", py_lib_gr_line, METH_VARARGS, "Draws a line"},
	{"drawString", py_lib_gr_string, METH_VARARGS, "Draws a string"},
	{"drawImage", py_lib_gr_image, METH_VARARGS, "Draws an image"},
    {NULL, NULL, 0, NULL}
};
void py_lib_init_gr()
{
	Py_InitModule("gr", GrMethods);
	//PyModule_AddObject(Py_InitModule("gr", GrMethods), "vec", (PyObject *)&pyvecType);
}

//********************SHP

static PyObject* py_lib_shp_get_class(PyObject *self, PyObject *args)
{
	char *ship_name = NULL;
	PyArg_ParseTuple(args, "s", &ship_name);
	if(ship_name == NULL) {
		return Py_BuildValue("i",0);
	}

	int ship_idx = ship_name_lookup(ship_name);
	if(ship_idx == -1) {
		return Py_BuildValue("i",-1);
	}

	return PyString_FromString(Ship_info[Ships[ship_idx].ship_info_index].name);
}

static PyObject* py_lib_shp_get_hull(PyObject *self, PyObject *args)
{
	char *ship_name = NULL;
	PyArg_ParseTuple(args, "s", &ship_name);
	if(ship_name == NULL) {
		return NULL;
	}

	int ship_idx = ship_name_lookup(ship_name);
	if(ship_idx == -1) {
		return NULL;
	}

	return Py_BuildValue("i",Objects[Ships[ship_idx].objnum].hull_strength);
}

static PyObject* py_lib_shp_get_shields(PyObject *self, PyObject *args)
{
	char *ship_name = NULL;
	PyArg_ParseTuple(args, "s", &ship_name);
	if(ship_name == NULL) {
		return NULL;
	}

	int ship_idx = ship_name_lookup(ship_name);
	if(ship_idx == -1) {
		return NULL;
	}

	return Py_BuildValue("i",Ships[ship_idx].shield_hits);
}

static PyObject* py_lib_shp_get_target(PyObject *self, PyObject *args)
{
	char *ship_name = NULL;
	PyArg_ParseTuple(args, "s", &ship_name);
	if(ship_name == NULL) {
		return NULL;
	}

	int ship_idx = ship_name_lookup(ship_name);
	if(ship_idx == -1) {
		return NULL;
	}

	if(Ships[ship_idx].ai_index == -1) {
		return NULL;
	}

	ship_idx = Ai_info[Ships[ship_idx].ai_index].target_objnum;
	if(Objects[ship_idx].type != OBJ_SHIP) {
		return NULL;
	}

	ship_idx = Objects[ship_idx].instance;

	return PyString_FromString(Ships[ship_idx].ship_name);
}

static PyObject* py_lib_shp_get_player_name(PyObject *self, PyObject *args)
{
	if(Player_ship == NULL) {
		return NULL;
	}

	return PyString_FromString(Player_ship->ship_name);
}

static PyMethodDef ShpMethods[] = {
	{"getClass", py_lib_shp_get_class, METH_VARARGS, "Gets a ship's class"},
	{"getTarget", py_lib_shp_get_target, METH_VARARGS, "Returns ship's target ship"},
	{"getPlayerName", py_lib_shp_get_player_name, METH_VARARGS, "Gets player ship name"},
	{"getShields", py_lib_shp_get_shields, METH_VARARGS, "Gets a ship's shield strength"},
	{"getHull", py_lib_shp_get_hull, METH_VARARGS, "Gets a ship's hull strength"},
	{NULL, NULL, 0, NULL},
};


//********************FS2
static PyObject* py_lib_fs2_get_state(PyObject *self, PyObject *args)
{
	int depth = 0;
	PyArg_ParseTuple(args, "|i", &depth);
	PyObject *res = PyString_FromString(GS_state_text[gameseq_get_state(depth)]);
	return res;
}

static PyObject* py_lib_fs2_get_viewer_mode(PyObject *self, PyObject *args)
{
	return Py_BuildValue("i",Viewer_mode);
}

static PyMethodDef Fs2Methods[] = {
	{"getState", py_lib_fs2_get_state, METH_VARARGS, "Gets current game state"},
	{"getViewerMode", py_lib_fs2_get_viewer_mode, METH_VARARGS, "Gets current viewer mode"},
	{NULL, NULL, 0, NULL}
};

void py_lib_init_fs2()
{
	Py_InitModule("fs2", Fs2Methods);
	Py_InitModule("shp", Fs2Methods);
}

//*************************GLOBAL FUNCS*************************
std::vector<PyBytecode> PythonFuncs;
PySession Temp_session;
static char* Test_file = "python.tbl";
int python_test()
{
	int rval;
	lcl_ext_open();
	if ((rval = setjmp(parse_abort)) != 0)
	{
		nprintf(("Warning", "Unable to parse %s!  Code = %i.\n", Test_file, rval));
		lcl_ext_close();
		return 0;
	}
	else
	{	
		read_file_text(Test_file);
		reset_parse();
	}

	read_file_text(Test_file);

	Temp_session = py_session_create();
	//py_session_import_module(&my_session, "gr");
	//py_session_use_namespace(&my_session, "gr");

	while(optional_string("$Python:"))
	{
		PyBytecode me = python_parse();
		if(me.Get() == NULL) {
			Error(LOCATION, "Parse error encountered in Python script");
		} else {
			PythonFuncs.push_back(me);
		}
	}

	lcl_ext_close();
	return 1;
}
void python_do_frame()
{
	for(uint i = 0; i < PythonFuncs.size(); i++)
	{
		py_session_evaluate(&Temp_session, &PythonFuncs[i]);
	}

	if(Gr_inited) {
//		gr_flip();
	}
}
void python_init()
{
	if(!Python_initted)
	{
		//Initialize Python
		Py_Initialize();

		Python_global_dict = PyDict_New ();
		//Get builtins.
		//Why? I don't know -WMC
		PyDict_SetItemString (Python_global_dict, "__builtins__", PyEval_GetBuiltins ());

		mprintf(("PYTHON: Core initialized"));

		//*************************LIBRARY INITIALIZATION*************************
		mprintf(("PYTHON: Initializing libraries..."));
		py_lib_init_gr();
		py_lib_init_fs2();
		//initpyvec();
		//PyDict_Update
		//TODO: Add all library initializations here.
		//************************************************************************

		mprintf(("PYTHON: Fully initialized"));
		Python_initted=1;
	}
	else
	{
		Warning(LOCATION, "Attempt to initialize Python twice was detected.");
	}

	python_test();
}

void python_close()
{
	Py_Finalize();
	Py_XDECREF(Python_global_dict);
	py_clear_images();

	mprintf(("PYTHON: Shutdown complete"));
	Python_initted=0;
}

//*************************SESSION FUNCTIONS*************************

//py_session_create
//Returns the local dictionary for this session.
//Use for an individual subsystem (eg HUD, armor.tbl)
//Any namespace changes and new functions or classes
//made by the user will be persistent in a session.
//IMPORTANT: Use PY_XDECREF to unload this when
//you're done with a session
PySession py_session_create()
{
	//Get rid of local dictionary
	return PySession(PyDict_New ());
}

void py_session_import_module(PySession* session, char* module_name)
{
	Assert(session != NULL && session->Get() != NULL);

	//PyDict_Update(session->Get(),PyImport_Import(PyString_FromString(module_name)));
	PyDict_Update(session->Get(), PyModule_GetDict(PyImport_ImportModule(module_name)));
}

//Adds a given module to the local dictionary, so that functions
//and such can be called without use of the name.
//EG: "math.cos()" could be invoked with "cos()"
void py_session_use_namespace(PySession* session, char* module_name)
{
	Assert(session != NULL && session->Get() != NULL);

	//PyDict_Update(session, PyModule_GetDict(PyImport_Import(PyString_FromString(module_name))));
	PyDict_Update(session->Get(), PyModule_GetDict(PyImport_AddModule(module_name)));
	//PyDict_Update(loc, PyModule_GetDict(PyImport_Import(PyString_FromString("time"))));
}

//parse_python()
//Throw this anywhere you want to parse python.
//It'll handle both large blocks and single lines
//To define a block, use '{' and '}'
//Returns the parsed Python object.
//After it's been parsed, call python_evaluate() to run it.
PyBytecode python_parse()
{
	//Placeholder for this call
	char debug_str[32];

	//If "{" is present, we assume multi-line Python
	//If it is not present, single-line Python
	if(!check_for_string("{"))
	{
		//We have a line
		sprintf(debug_str, "parse_python(ln) %d", Total_lines_parsed++);

		//Stuff it
		char buf[PARSE_BUF_SIZE] = {0};
		stuff_string(buf, F_RAW, NULL, sizeof(buf)-1);

		//Compile & return it
		return PyBytecode(Py_CompileString(buf, debug_str, Py_eval_input));
	}
	else
	{
		//We have a block
		sprintf(debug_str, "parse_python(bk) %d", Total_blocks_parsed++);

		//Get the block
		char* raw_python = alloc_block("{","}");

		//Compile & return
		return PyBytecode(Py_CompileString(raw_python, debug_str, Py_file_input));
	}
}

//given a session, evaluates compiled Python code
PyResult py_session_evaluate(PySession* session, PyBytecode* code)
{
	Assert(code != NULL);
	

	if(code->Get() != NULL)
	{
		PyObject *pyr = PyEval_EvalCode((PyCodeObject*)code->Get(),Python_global_dict,session->Get());
		if(pyr != NULL) {
			return PyResult(pyr);
		}
	}

	return PyResult(NULL);
}
#endif