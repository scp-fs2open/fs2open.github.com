#ifdef USE_PYTHON
#ifndef _PYTHON_H
#include <Python.h>
#include <structmember.h>
#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif


//*************************Python types*************************
//WMC - used for bytecode
struct PyBytecode
{
	PyObject *btcd;
public:
	PyObject *Get(){return btcd;}
	void Clear(){Py_XDECREF(btcd);}

	PyBytecode& operator= (PyObject *in){if(btcd!=NULL){Clear();}btcd=in;Py_XINCREF(in);return *this;}

	PyBytecode(PyObject *in_btcd=NULL){btcd=in_btcd;if(btcd!=NULL){Py_XINCREF(btcd);Py_XINCREF(btcd);}}
	~PyBytecode(){Py_XDECREF(btcd);}
};
//WMC - used to denote session
struct PySession
{
	PyObject *ssn;
public:
	PyObject *Get(){return ssn;}
	void Clear(){Py_XDECREF(ssn);}

	PySession& operator= (PyObject *in){if(ssn!=NULL){Clear();}ssn=in;Py_XINCREF(in);return *this;}

	PySession(PyObject *in_ssn=NULL){ssn=in_ssn;if(ssn!=NULL)Py_XINCREF(ssn);}
	~PySession(){Py_XDECREF(ssn);}
};
//WMC - used to denote Python result
struct PyResult
{
	PyObject *rslt;
public:
	//Generic functions
	PyObject *Get(){return rslt;}
	void Clear(){Py_XDECREF(rslt);}

	//Various data get functions
	bool GetLong(long *dest);
	bool GetDouble(double *dest);
	int GetString(char *dest, int dest_size);

	PyResult& operator= (PyObject *in){if(rslt!=NULL){Clear();}rslt=in;Py_XINCREF(in);return *this;}

	PyResult(PyObject *in_rslt=NULL){rslt=in_rslt;if(rslt!=NULL)Py_XINCREF(rslt);}
	~PyResult(){Py_XDECREF(rslt);}
};

//*************************Function Decs*************************
void python_init();
void python_close();
void python_do_frame();
void py_clear_images();

//*************************Usual Function Decs*************************
PySession py_session_create();
void py_session_import_module(PySession* session, char* module_name);
void py_session_use_namespace(PySession* session, char* module_name);
PyBytecode python_parse();
PyResult py_session_evaluate(PySession* session, PyBytecode* code);

#endif //_PYTHON_H
#endif //USE_PYTHON
