#pragma once

#include <Python.h>

#include "Room.h"
Room currentRoom(nullptr);
std::string currentParentName = "";

struct module_state {
	PyObject* error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif


HCObject* HCPy_GetObject(const char* name) {
	for (auto& l : currentRoom.objects) {
		for (auto& obj : l) {
			if (obj.name == std::string(name)) {
				return &obj;
			}
		}
	}
	return nullptr;
}

static PyObject*
error_out(PyObject* m) {
	struct module_state* st = GETSTATE(m);
	PyErr_SetString(st->error, "something bad happened");
	return NULL;
}

static PyObject*
get_position(PyObject* m, PyObject* args) {
	PyObject* result = PyTuple_New(2);

	char* s;

	if (!PyArg_ParseTuple(args, "s", &s)) {
		std::cout << "Error parsing python arguments for hc.get_position()\n";
		return NULL;
	}
	HCObject* obj = HCPy_GetObject(s);
	bool found = obj; //implicit "!= nullptr"
	
	if (found) {
		PyTuple_SetItem(result, 0, PyFloat_FromDouble(obj->position.x));
		PyTuple_SetItem(result, 1, PyFloat_FromDouble(obj->position.y));
		return result;
	}
	else {
		PyErr_SetString(PyExc_NameError, ("Object '" + std::string(s) +"' does not exist.").c_str());
		return NULL;
	}
}

static PyObject*
set_position(PyObject* m, PyObject* args) {
	//TODO: Find object & set position (args = tuple)
	char *s;
	float x = 0.f;
	float y = 0.f;
	if (!PyArg_ParseTuple(args, "sff", &s, &x, &y)) {
		PyErr_BadArgument();
		return NULL;
	}

	HCObject* obj = HCPy_GetObject(s);
	if (obj) {
		obj->position.x = x;
		obj->position.y = y;
	}
	Py_RETURN_NONE;
}


static PyObject*
get_size(PyObject* m, PyObject* args) {
	PyObject* result = PyTuple_New(2);

	char* s;

	if (!PyArg_ParseTuple(args, "s", &s)) {
		std::cout << "Error parsing python arguments for hc.get_position()\n";
		return NULL;
	}
	HCObject* obj = HCPy_GetObject(s);
	bool found = obj; //implicit "!= nullptr"

	if (found) {
		PyTuple_SetItem(result, 0, PyFloat_FromDouble(obj->GetSize().x));
		PyTuple_SetItem(result, 1, PyFloat_FromDouble(obj->GetSize().y));
		return result;
	}
	else {
		PyErr_SetString(PyExc_NameError, ("Object '" + std::string(s) + "' does not exist.").c_str());
		return NULL;
	}
}

static PyObject*
set_size(PyObject * m, PyObject * args) {
	//TODO: Find object & set position (args = tuple)
	char* s;
	float x = 0.f;
	float y = 0.f;
	if (!PyArg_ParseTuple(args, "sff", &s, &x, &y)) {
		PyErr_BadArgument();
		return NULL;
	}

	HCObject* obj = HCPy_GetObject(s);
	if (obj) {
		obj->SetSize(sf::Vector2f(x, y));
	}
	Py_RETURN_NONE;
}





static PyMethodDef hc_methods[] = {
	{"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
	{"get_position", (PyCFunction)get_position, METH_VARARGS, NULL},
	{"set_position", (PyCFunction)set_position, METH_VARARGS, NULL},
	{"get_size", (PyCFunction)get_size, METH_VARARGS, NULL},
	{"set_size", (PyCFunction)set_size, METH_VARARGS, NULL},
	{NULL, NULL}
};





#if PY_MAJOR_VERSION >= 3

static int hc_traverse(PyObject* m, visitproc visit, void* arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int hc_clear(PyObject* m) {
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}


static struct PyModuleDef hc = {
		PyModuleDef_HEAD_INIT,
		"hc",
		NULL,
		sizeof(struct module_state),
		hc_methods,
		NULL,
		hc_traverse,
		hc_clear,
		NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_myextension(void)

#else
#define INITERROR return

void
initmyextension(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
	PyObject * module = PyModule_Create(&hc);
#else
	PyObject* module = Py_InitModule("hc", hc_methods);
#endif

	if (module == NULL)
		INITERROR;
	struct module_state* st = GETSTATE(module);

	st->error = PyErr_NewException("hc.Error", NULL, NULL);
	if (st->error == NULL) {
		Py_DECREF(module);
		INITERROR;
	}

#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}