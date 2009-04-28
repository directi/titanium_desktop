/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "python_module.h"

namespace kroll
{
	KROLL_MODULE(PythonModule, STRING(MODULE_VERSION), STRING(MODULE_NAME));

	PythonModule* PythonModule::instance_ = NULL;

	void PythonModule::Initialize()
	{
		PythonModule::instance_ = this;

		Py_Initialize();
		PyEval_InitThreads();

		//TODO: maybe we need to setup path to script?
		//Py_SetProgramName(); 

		this->InitializeBinding();
		host->AddModuleProvider(this);
	}

	void PythonModule::Stop()
	{
		SharedKObject global = this->host->GetGlobalObject();
		global->Set("Python", Value::Undefined);
		this->binding->Set("evaluate", Value::Undefined);
		this->binding = NULL;
		PythonModule::instance_ = NULL;

		Py_Finalize();
	}

	void PythonModule::InitializeBinding()
	{
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new StaticBoundObject();
		global->Set("Python", Value::NewObject(this->binding));

		SharedKMethod evaluator = new PythonEvaluator();
		/**
		 * @tiapi(method=True,name=Python.evaluate,since=0.2) evaluate a python script
		 * @tiarg(for=Python.evaluate,name=code,type=string) python script code
		 * @tiarg(for=Python.evaluate,name=scope,type=object) global variable scope
		 * @tiresult(for=Python.evaluate,type=object) result
		 */
		this->binding->Set("evaluate", Value::NewMethod(evaluator));

		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* main_dict = PyModule_GetDict(main_module);
		PyObject* api = PythonUtils::KObjectToPyObject(Value::NewObject(global));
		PyDict_SetItemString(main_dict, PRODUCT_NAME, api);
		Py_DECREF(api);
	}


	const static std::string python_suffix = "module.py";

	bool PythonModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-python_suffix.length()) == python_suffix);
	}

	Module* PythonModule::CreateModule(std::string& path)
	{
		std::cout << "Create module: " << path << std::endl;
		FILE *file = fopen(path.c_str(), "r");

		PyRun_SimpleFile(file,path.c_str());
		std::cout << "PyRan simple file" << std::endl;

		std::cout << "return new PythonModuleInstance " << path << std::endl;
		return new PythonModuleInstance(host, path);
	}

}
