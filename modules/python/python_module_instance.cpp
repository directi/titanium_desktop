/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module_instance.h"

namespace kroll
{
	PythonModuleInstance::PythonModuleInstance(Host *host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"), path(path), dir(dir), name(name)
	{
		// add module location to python path
		this->AppendPath();

		// load module
		try
		{
			this->Load();
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("Python");
			logger->Error("Could not load %s because %s", dir.c_str(), (*ss).c_str());
		}
	}

	PythonModuleInstance::~PythonModuleInstance()
	{
	}

	void PythonModuleInstance::AppendPath()
	{
		std::string syspath = std::string(Py_GetPath());
		syspath += PATH_SEPARATOR + dir;
		PySys_SetPath((char*)syspath.c_str());
	}

	void PythonModuleInstance::Load()
	{
		std::string module_name = name + "module";
		this->module = PyImport_ImportModule((char*)module_name.c_str());
		if (this->module == NULL)
		{
			PyErr_Print();
			throw ValueException::FromString("Could not load module");
		}
	}

	void PythonModuleInstance::Initialize () 
	{
		// get Module class implementation from loaded module
		PyObject* module_dict = PyModule_GetDict(this->module);
		std::string class_name = name + "Module";
		class_name[0] = toupper(class_name[0]);
		PyObject* module_impl = PyDict_GetItemString(module_dict, class_name.c_str());

		// initialize module
		this->module_class = NULL;
		if (PyClass_Check(module_impl) && module_impl != NULL)
		{
			this->module_class = PyInstance_New(module_impl, NULL, NULL);
			if (this->module_class == NULL)
			{
				PyErr_Print();
				throw ValueException::FromString("Could not create instance of module class");
			}
		}
 	}

	void PythonModuleInstance::Stop () 
	{
		if (Py_IsInitialized())
		{
			// release module class
			Py_XDECREF(this->module_class);

			// release module
			Py_XDECREF(this->module);
		}
	}
}

