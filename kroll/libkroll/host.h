/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_HOST_H_
#define _KR_HOST_H_

#ifdef OS_WIN32
#include "win32/event_window.h"
#endif

#include <boost/timer.hpp>
#include <boost/thread/mutex.hpp>

#include "utils/application.h"
#include "binding/global_object.h"

#include "module.h"
#include "module_provider.h"
#include "main_thread_job.h"

namespace kroll
{
	typedef std::vector<SharedPtr<Module> > ModuleList;

	/**
	 * Class that is implemented by the OS to handle OS-specific
	 * loading and unloading of Kroll.
	 */
	class KROLL_API Host
	 : public ModuleProvider
	{
		static Host* hostInstance;
		Host(int argc, const char** argv);
		virtual ~Host();
		DISALLOW_EVIL_CONSTRUCTORS(Host);

	public:
		/**
		 * Get the host singleton.
		 */
		static void InitializeHost(int argc, const char** argv);

		/**
		 * Get the host singleton.
		 */
		static void UnInitializeHost();

		/**
		 * Get the host singleton.
		 */
		static Host* GetInstance();

		// Platform specific method InitializeMainThread
		static void InitializeMainThread();

		SharedApplication GetApplication();
		bool DebugModeEnabled() const { return this->debug; }
		bool ProfilingEnabled() const { return this->profile; }
		double GetElapsedTime() const { return timeStarted.elapsed(); }
		KObjectRef GetGlobalObject() { return GlobalObject::GetInstance(); }

		/**
		 * Called to run the host
		 */
		int Run();

		/**
		 * Called to exit the host and terminate the process
		 */
		void Exit(int exitcode);

		/*
		 * Call with a method and arguments to invoke the method on the UI thread.
		 * @param method method to execute on the main thread
		 * @param args method arguments
		 * @param waitForCompletion block until method is finished (default: true)
		 * @return the method's return value§
		 */
		KValueRef RunOnMainThread(KMethodRef method, const ValueList& args,
			bool waitForCompletion);

		/**
		 * Add a module provider to the host
		 */
		void AddModuleProvider(ModuleProvider *provider);

		/**
		 * Remove a module provider
		 */
		void RemoveModuleProvider(ModuleProvider *provider);

		/**
		 * Call the Destroy() lifecycle event on this a module and
		 * remove it from our map of modules.
		 *
		 * @module The module to remove.
		 */
		void UnregisterModule(SharedPtr<Module> module);

		/**
		 * Get a module given the module path.
		 * @param path The path of the module to get
		 *
		 * @return A reference to the module.
		 */
		SharedPtr<Module> GetModuleByPath(std::string& path);

		/**
		 * Get a module give by the module name (such as tiui)
		 * @param name of the module
		 * @return A reference to the module
		 */
		SharedPtr<Module> GetModuleByName(std::string& name);

		/**
		 * @return whether or not a module with the path exists
		 * @param name the full path to the module
		*/
		bool HasModule(std::string name);

		/**
		 * Execute all jobs waiting to be run on the main thread.
		 */
		void RunMainThreadJobs();

		/**
		 * @param path The filesystem path of a module
		 * @return true if the file is a native module (.dll / .dylib / .so)
		 */
		virtual bool IsModule(const std::string& path) const;

		virtual Module* CreateModule(const std::string& path);

		/*
		 * Return true if this thread is the main thread.
		 */
		bool IsMainThread();

#ifdef OS_WIN32
		HWND AddMessageHandler(MessageHandler handler);
		HWND GetEventWindow();
#endif

	private:
		ModuleList loadedModules;
		boost::mutex moduleMutex;
		std::vector<ModuleProvider *> moduleProviders;
		std::vector<std::string> modulePaths;
		SharedApplication application;
		bool exiting;
		int exitCode;
		bool debug;
		bool waitForDebugger;
		bool autoScan;

		// Profiling Related variables & methods
		bool profile;
		std::string profilePath;
		std::ofstream* profileStream;

		void SetupProfiling();
		void StopProfiling();

		// Logging Related variables & methods
		bool fileLogging;
		std::string logFilePath;
		bool consoleLogging;
		Logger* logger;

		std::string getLogFilePath();
		void SetupLogging();

		// other
		boost::timer timeStarted;
		boost::mutex jobQueueMutex;
		std::vector<MainThreadJob*> mainThreadJobs;
		std::vector<std::string> invalidModuleFiles;

		ModuleProvider* FindModuleProvider(std::string& filename);
		void UnloadModuleProviders();

		void FindBasicModules(std::string& dir);
		void ScanInvalidModuleFiles();

		SharedPtr<Module> LoadModule(std::string& path, ModuleProvider* provider);
		void StartModules(std::vector<SharedPtr<Module> > modules);
		void LoadModules();
		void UnloadModules();

		void SetupApplication(int argc, const char* argv[]);

		void AddInvalidModuleFile(std::string path);
		void ParseCommandLineArguments();
		void Shutdown();

		// Platform-specific
		void Initialize(int argc, const char* argv[]);
		void WaitForDebugger();
		bool RunLoop();
		void SignalNewMainThreadJob();
		void ExitImpl(int exitcode);
	};

	KROLL_API KValueRef RunOnMainThread(KMethodRef method, const ValueList& args,
		bool waitForCompletion=true);
}

extern "C" KROLL_API int Execute(int argc, const char **argv);

#endif

