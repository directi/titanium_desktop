/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include <kroll/thread_manager.h>
#include "filesystem_binding.h"
#include "file.h"
#include "file_stream.h"
#include "NamedMutex.h"
#include "async_copy.h"
#include "filesystem_utils.h"
#include <boost/filesystem.hpp>

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#elif OS_WIN32
#include <windows.h>
#include <shlobj.h>
#include <process.h>
#elif OS_LINUX
#include <sys/types.h>
#include <pwd.h>
#endif

#include <fstream>

#include <Poco/TemporaryFile.h>
#include <Poco/Exception.h>

namespace ti
{
	void listRoots(std::vector<std::string > & roots)
	{
#ifdef OS_WIN32
		roots.clear();
		char buffer[128];
		DWORD n = GetLogicalDriveStringsA(sizeof(buffer) - 1, buffer);
		char* it = buffer;
		char* end = buffer + (n > sizeof(buffer) ? sizeof(buffer) : n);
		while (it < end)
		{
			std::string dev;
			while (it < end && *it) dev += *it++;
			roots.push_back(dev);
			++it;
		}
#else
		roots.push_back("/");
#endif
	}
	FilesystemBinding::FilesystemBinding(Host* _host)
		: StaticBoundObject("Filesystem"), host(_host)
	{
		/**
		 * @tiapi(method=True,name=Filesystem.createTempFile) Creates a temporary file
		 * @tiresult(for=Filesystem.createTempFile,type=Filesystem.File) a File object referencing the temporary file
		 */
		this->SetMethod("createTempFile",&FilesystemBinding::CreateTempFile);
		/**
		 * @tiapi(method=True,name=Filesystem.createTempDirectory) Creates a temporary directory
		 * @tiresult(for=Filesystem.createTempDirectory,type=Filesystem.File) a File object referencing the temporary directory
		 */
		this->SetMethod("createTempDirectory",&FilesystemBinding::CreateTempDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getFile) Returns a file path, optionally joining multiple arguments together in an OS specific way
		 * @tiarg(for=Filesystem.getFile,name=pathname) a string that is used to form a path
		 * @tiarg(for=Filesystem.getFile,optional=true,name=...) a variable length argument list of Strings that are concatinated with pathname to form a path
		 * @tiresult(for=Filesystem.getFile,type=Filesystem.File) a File object referencing the file
		 */
		this->SetMethod("getFile",&FilesystemBinding::GetFile);
		/**
		 * @tiapi(method=True,name=Filesystem.getFileStream) Returns a Filestream object
		 * @tiresult(for=Filesystem.getFileStream,type=Filesystem.Filestream) a Filestream object referencing the file
		 */
		this->SetMethod("getFileStream",&FilesystemBinding::GetFileStream);
		/**
		 * @tiapi(method=True,name=Filesystem.GetNamedMutex) Returns a NamedMutex object
		 * @tiresult(for=Filesystem.getNamedMutex,type=FileSystem.NamedMutex) a NamedMutex object
		 */
		this->SetMethod("getNamedMutex",&FilesystemBinding::GetNamedMutex);
		/**
		 * @tiapi(method=True,name=Filesystem.getProgramsDirectory) Returns the programs directory of the current system
		 * @tiresult(for=Filesystem.getProgramsDirectory,type=Filesystem.File) a File object referencing the system programs directory
		 */
		this->SetMethod("getProgramsDirectory",&FilesystemBinding::GetProgramsDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getApplicationDirectory) Returns the directory where the application resides
		 * @tiresult(for=Filesystem.getApplicationDirectory,type=Filesystem.File) a File object referencing the directory where the application resides
		 */
		this->SetMethod("getApplicationDirectory",&FilesystemBinding::GetApplicationDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getApplicationDataDirectory) Returns the data directory of the application
		 * @tiresult(for=Filesystem.getApplicationDataDirectory,type=Filesystem.File) a File object referencing the data directory of the application
		 */
		this->SetMethod("getApplicationDataDirectory",&FilesystemBinding::GetApplicationDataDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getRuntimeHomeDirectory) Returns the directory of where the Titanium runtime files are stored
		 * @tiresult(for=Filesystem.getRuntimeHomeDirectory,type=Filesystem.File) a File object referencing the directory where the Titanium runtime files are stored.
		 */
		this->SetMethod("getRuntimeHomeDirectory",&FilesystemBinding::GetRuntimeHomeDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getResourcesDirectory) Returns the resources directory of the application
		 * @tiresult(for=Filesystem.getResourcesDirectory,type=Filesystem.File) a File object referencing the resources directory of the application
		 */
		this->SetMethod("getResourcesDirectory",&FilesystemBinding::GetResourcesDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getDesktopDirectory) Returns the system desktop directory
		 * @tiresult(for=Filesystem.getDesktopDirectory,type=Filesystem.File) a File object referencing the system desktop directory
		 */
		this->SetMethod("getDesktopDirectory",&FilesystemBinding::GetDesktopDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getDocumentsDirectory) Returns the system documents directory
		 * @tiresult(for=Filesystem.getDocumentsDirectory,type=Filesystem.File) a File object referencing the system documents directory
		 */
		this->SetMethod("getDocumentsDirectory",&FilesystemBinding::GetDocumentsDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getUserDirectory) Returns the home directory of the current user
		 * @tiresult(for=Filesystem.getUserDirectory,type=Filesystem.File) a File object referencing the home directory of the current user
		 */
		this->SetMethod("getUserDirectory",&FilesystemBinding::GetUserDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.getLineEnding) Returns the line ending used by the operating system
		 * @tiresult(for=Filesystem.getLineEnding,type=String) the line ending used by the operating system
		 */
		this->SetMethod("getLineEnding",&FilesystemBinding::GetLineEnding);
		/**
		 * @tiapi(method=True,name=Filesystem.getSeparator) Returns the path separator used by the operating system
		 * @tiresult(for=Filesystem.getSeparator,type=String) the path separator used by the operating system
		 */
		this->SetMethod("getSeparator",&FilesystemBinding::GetSeparator);
		/**
		 * @tiapi(method=True,name=Filesystem.getRootDirectories) Returns the system root directory
		 * @tiresult(for=Filesystem.getRootDirectories,type=Array<Filesystem.File>) a File object referencing the system root directory
		 */
		this->SetMethod("getRootDirectories",&FilesystemBinding::GetRootDirectories);
		/**
		 * @tiapi(method=True,name=Filesystem.asyncCopy) Executes an async copy operation
		 * @tiarg(for=Filesystem.asyncCopy,name=paths,type=Array<String|Filesystem.File>|Filesystem.File)
		 * @tiarg Either a path or array of paths to copy from
		 * @tiarg(for=Filesystem.asyncCopy,name=destination,type=Filesystem.File|String) either a string or file object to copy to
		 * @tiarg(for=Filesystem.asyncCopy,name=callback,type=Function) callback to invoke on each copy completion operation
		 * @tiresult(for=Filesystem.asyncCopy,type=Filesystem.AsyncCopy) async copy object
		 */
		this->SetMethod("asyncCopy",&FilesystemBinding::ExecuteAsyncCopy);

		/**
		 * @tiapi(property=True,immutable=True,name=Filesystem.MODE_READ, since=0.3, type=Number) File read constant
		 */

		this->SetInt("MODE_READ", MODE_READ);
		this->SetInt("MODE_WRITE", MODE_WRITE);
		this->SetInt("MODE_APPEND", MODE_APPEND);
		this->SetInt("SEEK_START", std::ios::beg);
		this->SetInt("SEEK_CURRENT", std::ios::cur);
		this->SetInt("SEEK_END", std::ios::end);
	}

	FilesystemBinding::~FilesystemBinding()
	{
	}

	void FilesystemBinding::CreateTempFile(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::TemporaryFile tempFile;
			tempFile.keepUntilExit();
			tempFile.createFile();

			ti::File* jsFile = new ti::File(tempFile.path());
			result->SetObject(jsFile);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void FilesystemBinding::CreateTempDirectory(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::TemporaryFile tempDir;
			tempDir.keepUntilExit();
			tempDir.createDirectory();

			ti::File* jsFile = new ti::File(tempDir.path());
			result->SetObject(jsFile);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}


	void FilesystemBinding::GetFile(const ValueList& args, KValueRef result)
	{
		result->SetObject(new ti::File(FilesystemUtils::FilenameFromArguments(args)));
	}

	void FilesystemBinding::GetFileStream(const ValueList& args, KValueRef result)
	{
		result->SetObject(new ti::FileStream(FilesystemUtils::FilenameFromArguments(args)));
	}

	void FilesystemBinding::GetNamedMutex(const ValueList& args, KValueRef result)
	{
		std::string filename = args.at(0)->ToString();
		ti::NamedMutex * namedMutex = new ti::NamedMutex(filename);
		result->SetObject(namedMutex);
	}

	void FilesystemBinding::GetApplicationDirectory(const ValueList& args, KValueRef result)
	{
		result->SetObject(new ti::File(host->GetApplication()->getPath()));
	}

	void FilesystemBinding::GetApplicationDataDirectory(const ValueList& args, KValueRef result)
	{
		result->SetObject(new ti::File(
			Host::GetInstance()->GetApplication()->GetDataPath()));
	}

	void FilesystemBinding::GetRuntimeHomeDirectory(const ValueList& args, KValueRef result)
	{
		std::string dir = FileUtils::GetSystemRuntimeHomeDirectory();
		ti::File* file = new ti::File(dir);
		result->SetObject(file);
	}

	void FilesystemBinding::GetResourcesDirectory(const ValueList& args, KValueRef result)
	{
		ti::File* file = new ti::File(host->GetApplication()->GetResourcesPath());
		result->SetObject(file);
	}

	void FilesystemBinding::GetProgramsDirectory(const ValueList &args, KValueRef result)
	{
#ifdef OS_WIN32
		wchar_t path[MAX_PATH];
		if (!SHGetSpecialFolderPathW(NULL, path, CSIDL_PROGRAM_FILES, FALSE))
			throw ValueException::FromString("Could not get Program Files path.");
		std::string dir(::WideToUTF8(path));

#elif OS_OSX
		std::string dir([[NSSearchPathForDirectoriesInDomains(
			NSApplicationDirectory, NSLocalDomainMask, YES)
			objectAtIndex: 0] UTF8String]);

#elif OS_LINUX
		// TODO: this might need to be configurable
		std::string dir("/usr/local/bin");
#endif
		ti::File* file = new ti::File(dir);
		result->SetObject(file);
	}

	void FilesystemBinding::GetDesktopDirectory(const ValueList& args, KValueRef result)
	{
#ifdef OS_WIN32
		wchar_t path[MAX_PATH];
		if (!SHGetSpecialFolderPathW(NULL, path, CSIDL_DESKTOPDIRECTORY, FALSE))
			throw ValueException::FromString("Could not get Desktop path.");
		std::string dir(::WideToUTF8(path));

#elif OS_OSX
		std::string dir([[NSSearchPathForDirectoriesInDomains(
			NSDesktopDirectory, NSUserDomainMask, YES)
			objectAtIndex: 0] UTF8String]);

#elif OS_LINUX
		passwd *user = getpwuid(getuid());
		std::string homeDirectory = user->pw_dir;
		std::string dir(FileUtils::Join(homeDirectory.c_str(), "Desktop", NULL));
		if (!FileUtils::IsDirectory(dir))
		{
			dir = homeDirectory;
		}
#endif
		ti::File* file = new ti::File(dir);
		result->SetObject(file);
	}

	void FilesystemBinding::GetDocumentsDirectory(const ValueList& args, KValueRef result)
	{
#ifdef OS_WIN32
		wchar_t path[MAX_PATH];
		if (!SHGetSpecialFolderPathW(NULL, path, CSIDL_PERSONAL, FALSE))
			throw ValueException::FromString("Could not get Documents path.");
		std::string dir(::WideToUTF8(path));

#elif OS_OSX
		std::string dir([[NSSearchPathForDirectoriesInDomains(
			NSDocumentDirectory, NSUserDomainMask, YES)
			objectAtIndex: 0] UTF8String]);

#elif OS_LINUX
		passwd* user = getpwuid(getuid());
		std::string homeDirectory = user->pw_dir;
		std::string dir(FileUtils::Join(homeDirectory.c_str(), "Documents", NULL));
		if (!FileUtils::IsDirectory(dir))
		{
			dir = homeDirectory;
		}
#endif
		ti::File* file = new ti::File(dir);
		result->SetObject(file);
	}

	void FilesystemBinding::GetUserDirectory(const ValueList& args, KValueRef result)
	{
		std::string dir;
		dir = EnvironmentUtils::GetHomePath();
		result->SetObject(new ti::File(dir));
	}

	void FilesystemBinding::GetLineEnding(const ValueList& args, KValueRef result)
	{
		// TODO: fix with platform specific set line encoding.
		// not breaking anything as its anyway returning same
		result->SetString("\n");
	}

	void FilesystemBinding::GetSeparator(const ValueList& args, KValueRef result)
	{
		result->SetString(KR_PATH_SEP);
	}

	void FilesystemBinding::GetRootDirectories(const ValueList& args, KValueRef result)
	{
		try
		{
			std::vector<std::string> roots;
			listRoots(roots);

			KListRef rootList = new StaticBoundList();
			for(size_t i = 0; i < roots.size(); i++)
			{
				ti::File* file = new ti::File(roots.at(i));
				KValueRef value = Value::NewObject((KObjectRef) file);
				rootList->Append(value);
			}

			KListRef list = rootList;
			result->SetList(list);
		}
		catch (...)
		{
			throw ValueException::FromString("Unknown Exception");
		}
	}

	void FilesystemBinding::ExecuteAsyncCopy(const ValueList& args, KValueRef result)
	{
		if (args.size()!=3)
		{
			throw ValueException::FromString("invalid arguments - this method takes 3 arguments");
		}
		std::vector<std::string> files;
		if (args.at(0)->IsString())
		{
			files.push_back(args.at(0)->ToString());
		}
		else if (args.at(0)->IsList())
		{
			KListRef list = args.at(0)->ToList();
			for (unsigned int c = 0; c < list->Size(); c++)
			{
				files.push_back(FilesystemUtils::FilenameFromValue(list->At(c)));
			}
		}
		else
		{
			files.push_back(FilesystemUtils::FilenameFromValue(args.at(0)));
		}
		KValueRef v = args.at(1);
		std::string destination(FilesystemUtils::FilenameFromValue(v));
		KMethodRef method = args.at(2)->ToMethod();
		KObjectRef copier = new ti::AsyncCopy(files, destination, method);
		result->SetObject(copier);
	}
}
