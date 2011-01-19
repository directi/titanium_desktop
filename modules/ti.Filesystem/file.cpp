/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "file.h"
#include "file_stream.h"
#include "filesystem_utils.h"

#include "TiAsyncJobRunner.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Exception.h>

#include <kroll/utils/zip_utils.h>
#include <kroll/utils/file_utils.h>
#include <kroll/utils/md5_utils.h>

#ifndef OS_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef OS_LINUX
#include <sys/statvfs.h>
#endif

#ifdef OS_WIN32
#include <shellapi.h>
#include <shlobj.h>
#define MIN_PATH_LENGTH 3
#else
#define MIN_PATH_LENGTH 1
#endif


namespace ti
{
	std::string getAbsolutePath(const std::string & path)
	{
		Poco::Path pocoPath(Poco::Path::expand(path));
		std::string retpath = pocoPath.absolute().toString();

		// If the filename we were given contains a trailing slash, just remove it
		// so that users can count on reproducible results from toString.
		size_t length = retpath.length();
		if (length > MIN_PATH_LENGTH && retpath[length - 1] == Poco::Path::separator())
		{
			retpath.resize(length - 1);
		}
		return retpath;
	}
	File::File(const std::string &name)
		: StaticBoundObject("Filesystem.File"),
		filename(getAbsolutePath(name))
	{
		this->SetMethod("open", &File::Open);
		this->SetMethod("toString", &File::ToString);
		this->SetMethod("toURL", &File::ToURL);
		this->SetMethod("isFile", &File::IsFile);
		this->SetMethod("isDirectory", &File::IsDirectory);
		this->SetMethod("isHidden", &File::IsHidden);
		this->SetMethod("isSymbolicLink", &File::IsSymbolicLink);
		this->SetMethod("isExecutable", &File::IsExecutable);
		this->SetMethod("isReadonly", &File::IsReadonly);
		this->SetMethod("isWriteable", &File::IsWritable);
		this->SetMethod("isWritable", &File::IsWritable);
		this->SetMethod("resolve", &File::Resolve);
		this->SetMethod("copy", &File::Copy);
		this->SetMethod("move", &File::Move);
		this->SetMethod("rename", &File::Rename);
		this->SetMethod("touch", &File::Touch);
		this->SetMethod("createDirectory", &File::CreateDirectory);
		this->SetMethod("deleteDirectory", &File::DeleteDirectory);
		this->SetMethod("deleteFile", &File::DeleteFile);
		this->SetMethod("getDirectoryListing", &File::GetDirectoryListing);
		this->SetMethod("parent", &File::GetParent);
		this->SetMethod("exists", &File::GetExists);
		this->SetMethod("createTimestamp", &File::GetCreateTimestamp);
		this->SetMethod("modificationTimestamp", &File::GetModificationTimestamp);
		this->SetMethod("name", &File::GetName);
		this->SetMethod("extension", &File::GetExtension);
		this->SetMethod("nativePath", &File::GetNativePath);
		this->SetMethod("size", &File::GetSize);
		this->SetMethod("spaceAvailable", &File::GetSpaceAvailable);
		this->SetMethod("createShortcut", &File::CreateShortcut);
		this->SetMethod("setExecutable", &File::SetExecutable);
		this->SetMethod("setReadonly", &File::SetReadonly);
		this->SetMethod("setWriteable", &File::SetWritable);
		this->SetMethod("setWritable", &File::SetWritable);
		this->SetMethod("unzip", &File::Unzip);
		this->SetMethod("MD5Digest",&File::MD5Digest);
	}

	File::~File()
	{
	}

	void File::Open(const ValueList& args, KValueRef result)
	{
		args.VerifyException("open", "?ibb");

		FileStream* stream = new FileStream(this->filename);
		result->SetObject(stream);

		// Perform open operation before returning stream object
		KValueRef openResult(Value::NewUndefined());
		stream->Open(args, openResult);
		if (openResult->ToBool() == false)
		{
			// Failed to open stream, return null
			result->SetNull();
		}
	}

	void File::ToURL(const ValueList& args, KValueRef result)
	{
		std::string url(URLUtils::PathToFileURL(this->filename));
		result->SetString(url.c_str());
	}

	void File::ToString(const ValueList& args, KValueRef result)
	{
		Logger::Get("File")->Debug("ToString: %s", filename.c_str());
		result->SetString(this->filename.c_str());
	}

	void File::IsFile(const ValueList& args, KValueRef result)
	{
		bool bRet = FileUtils::IsFile(this->filename);
		result->SetBool(bRet);
	}

	void File::IsDirectory(const ValueList& args, KValueRef result)
	{
		bool bRet = FileUtils::IsDirectory(this->filename);
		result->SetBool(bRet);
	}

	void File::IsHidden(const ValueList& args, KValueRef result)
	{
		bool bRet = FileUtils::IsHidden(this->filename);
		result->SetBool(bRet);
	}

	void File::IsSymbolicLink(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			bool isLink = file.isLink();
			result->SetBool(isLink);
		}
		catch (Poco::FileNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::IsExecutable(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			result->SetBool(file.canExecute());
		}
		catch (Poco::FileNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::IsReadonly(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			result->SetBool(file.canRead() && !file.canWrite());
		}
		catch (Poco::FileNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::IsWritable(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			result->SetBool(file.canWrite());
		}
		catch (Poco::FileNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::Resolve(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string pathToResolve = args.at(0)->ToString();

			Poco::Path path(this->filename);
			path.resolve(pathToResolve);

			ti::File* file = new ti::File(path.toString());
			result->SetObject(file);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::Copy(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string dest(FilesystemUtils::FilenameFromValue(args.at(0)));
			Poco::File from(this->filename);
			from.copyTo(dest);
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::Move(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string dest(FilesystemUtils::FilenameFromValue(args.at(0)));
			Poco::File from(this->filename);
			from.moveTo(dest);
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::Rename(const ValueList& args, KValueRef result)
	{
		try
		{
			std::string name = args.at(0)->ToString();
			Poco::File f(this->filename);
			Poco::Path p(this->filename);
			p.setFileName(name);
			f.renameTo(p.toString());
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::Touch(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File f(this->filename);
			result->SetBool(f.createFile());
		}
		catch (Poco::Exception& exc)
		{
			Logger::Get("File")->Error("exception while creating file: %s", exc.what());
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::CreateDirectory(const ValueList& args, KValueRef result)
	{
		bool recursive = false;
		if(args.size() > 0)
		{
			recursive = args.at(0)->ToBool();
		}

		bool bRet = FileUtils::CreateDirectory(this->filename, recursive);
		result->SetBool(bRet);
	}

	void File::DeleteDirectory(const ValueList& args, KValueRef result)
	{
		try
		{
			bool deleteContents = false;
			if(args.size() > 0)
			{
				deleteContents = args.at(0)->ToBool();
			}

			Poco::File dir(this->filename);
			bool deleted = false;
			if(dir.exists() && dir.isDirectory())
			{
				dir.remove(deleteContents);

				deleted = true;
			}
			result->SetBool(deleted);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::DeleteFile(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			bool deleted = false;
			if(file.exists() && file.isFile())
			{
				file.remove();

				deleted = true;
			}
			result->SetBool(deleted);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetDirectoryListing(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File dir(this->filename);

			if(dir.exists() && dir.isDirectory())
			{
				std::vector<std::string> files;
				dir.list(files);

				KListRef fileList = new StaticBoundList();
				for(size_t i = 0; i < files.size(); i++)
				{
					std::string entry = files.at(i);
					// store it as the fullpath
					std::string filename = kroll::FileUtils::Join(this->filename.c_str(),entry.c_str(),NULL);
					ti::File* file = new ti::File(filename);
					KValueRef value = Value::NewObject((KObjectRef) file);
					fileList->Append(value);
				}

				KListRef list = fileList;
				result->SetList(list);
			}
			else
			{
				result->SetNull();
			}
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetParent(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::Path path(this->filename);

			ti::File* file = new ti::File(path.parent().toString());
			result->SetObject(file);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetExists(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			bool exists = file.exists();
			result->SetBool(exists);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetCreateTimestamp(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			Poco::Timestamp ts = file.created();

			result->SetDouble(ts.epochMicroseconds());
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetModificationTimestamp(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			Poco::Timestamp ts = file.getLastModified();

			result->SetDouble(ts.epochMicroseconds());
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetName(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::Path path(this->filename);
			result->SetString(path.getFileName().c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetExtension(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::Path path(this->filename);

			if(path.isDirectory())
			{
				result->SetNull();
			}
			else
			{
				result->SetString(path.getExtension().c_str());
			}
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetNativePath(const ValueList& args, KValueRef result)
	{
		result->SetString(this->filename.c_str());
	}

	void File::GetSize(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);

			result->SetDouble(file.getSize());
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::GetSpaceAvailable(const ValueList& args, KValueRef result)
	{	
		double diskSize = -1.0;
		std::string error;

#ifdef OS_OSX
		NSString *p = [NSString stringWithCString:this->filename.c_str() encoding:NSUTF8StringEncoding];
		unsigned long avail = [[[[NSFileManager defaultManager] fileSystemAttributesAtPath:p] objectForKey:NSFileSystemFreeSize] longValue];
		diskSize = (double)avail;
#elif defined(OS_WIN32)
		unsigned __int64 i64FreeBytesToCaller;
		unsigned __int64 i64TotalBytes;
		unsigned __int64 i64FreeBytes;
		std::wstring wideFilename(::UTF8ToWide(this->filename));
		if (GetDiskFreeSpaceExW(
			wideFilename.c_str(),
			(PULARGE_INTEGER) &i64FreeBytesToCaller,
			(PULARGE_INTEGER) &i64TotalBytes,
			(PULARGE_INTEGER) &i64FreeBytes))
		{
			diskSize = (double)i64FreeBytesToCaller;
		}
		else
		{
			error = Win32Utils::QuickFormatMessage(GetLastError());
		}
#elif defined(OS_LINUX)
		struct statvfs stats;
		if (statvfs(this->filename.c_str(), &stats) == 0)
		{
			unsigned long avail = stats.f_bsize * static_cast<unsigned long>(stats.f_bavail);
			diskSize = (double)avail;
		}
#endif
		if ( diskSize >=0.0 )
		{
			result->SetDouble(diskSize);
		}
		else 
		{
			throw ValueException::FromString("Cannot determine diskspace on filename '" + this->filename + "' with error message " +error);
		}
	}

	void File::CreateShortcut(const ValueList& args, KValueRef result)
	{
		if (args.size()<1)
		{
			throw ValueException::FromString("createShortcut takes a parameter");
		}
		std::string from = this->filename;
		std::string to(FilesystemUtils::FilenameFromValue(args.at(0)));

#if defined(OS_WIN32)
		HRESULT hResult;
		IShellLinkW* shellLink;
		result->SetBool(false);

		if(from.length() == 0 || to.length() == 0) {
			std::string ex = "Invalid arguments given to createShortcut()";
			throw ValueException::FromString(ex);
		}

		if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL,
			CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*) &shellLink)))
			throw ValueException::FromString(
				"Could not create shortcut: failed to create ShellLink instance");

		// set path to the shortcut target and add description
		shellLink->SetPath(::UTF8ToWide(from).c_str());
		shellLink->SetDescription(L"File shortcut");

		IPersistFile* persistFile;
		if (!SUCCEEDED(shellLink->QueryInterface(IID_IPersistFile, (LPVOID*) &persistFile)))
		{
			shellLink->Release();
			throw ValueException::FromString(
				"Could not create shortcut: failed to query PersistFile interface");
		}

		// ensure to ends with .lnk
		if (to.substr(to.size() - 4) != ".lnk")
			to.append(".lnk");
		std::wstring wideTo(::UTF8ToWide(to.c_str()));
		if (SUCCEEDED(persistFile->Save(wideTo.c_str(), TRUE)))
			result->SetBool(true);

		shellLink->Release();
#else
		result->SetBool(symlink(this->filename.c_str(), to.c_str()) == 0);
#endif
	}

	void File::SetExecutable(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			file.setExecutable(args.at(0)->ToBool());
			result->SetBool(file.canExecute());
		}
		catch (Poco::FileNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::SetReadonly(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			file.setReadOnly(true);
			result->SetBool(file.canRead() && !file.canWrite());	
		}
		catch (Poco::FileNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	void File::SetWritable(const ValueList& args, KValueRef result)
	{
		try
		{
			Poco::File file(this->filename);
			file.setWriteable(true);
			result->SetBool(file.canWrite());
		}
		catch (Poco::FileNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException&)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}

	/**
	 * Function: Unzip
	 *   unzip this file to destination
	 *
	 * Parameters:
	 *   dest - destination directory to unzip this file
	 *
	 * Returns:
	 *   true if succeeded
	 */
	void File::Unzip(const ValueList& args, KValueRef result)
	{
		std::string destDir;
		if (args.size() < 2)
		{
			throw ValueException::FromString("please provide at least 2 args");
		}

		Poco::Path pocoPath(Poco::Path::expand(args.at(0)->ToString()));
		destDir = pocoPath.absolute().toString();

		KMethodRef onCompleteCallback = NULL;
		if (args.at(1)->IsMethod())
		{
			onCompleteCallback = args.at(1)->ToMethod();
		}
		else
		{
			throw ValueException::FromString("invalid argument - second argument must be method callback");
		}

		KMethodRef progressCallback = NULL;
		if (args.size() > 2)
		{
			if (args.at(2)->IsMethod())
			{
				progressCallback = args.at(2)->ToMethod();
			}
			else
			{
				throw ValueException::FromString("invalid argument - third argument must be method callback");
			}
		}
		struct UnzipStruct * str = new UnzipStruct(this, destDir, onCompleteCallback, progressCallback);
		TiAsyncJobRunnerSingleton::Instance()->enqueue(new TiThreadTarget(&File::UnzipThread, str));

		//this->Unzip(destDir, onCompleteCallback, progressCallback);
	}

	void File::UnzipThread(void * param)
	{
		struct UnzipStruct * str = static_cast<struct UnzipStruct *>(param);
		if (str->file)
		{
			str->file->Unzip(str->destDir, str->onCompleteCallback, str->progressCallback);
		}
	}

	void File::Unzip(const std::string & destDir, KMethodRef onCompleteCallback, KMethodRef progressCallback) const
	{
		bool ret = false;
		std::string err;
		try
		{
			Poco::File from(this->filename);
			Poco::File to(destDir.c_str());
			std::string from_s = from.path();
			std::string to_s = to.path();
			bool dir_exists = true;

			if (!to.exists())
			{
				if (!to.createDirectory())
				{
					dir_exists = false;
					err = "Error creating directory " + to_s;
				}
			}
			else if (!to.isDirectory())
			{
				dir_exists = false;
				err = "destination must be a directory";
			}

			if (dir_exists)
			{
				kroll::FileUtils::Unzip(from_s,to_s);
				ret = true;
			}
		}
		catch (Poco::FileNotFoundException&)
		{
			err = "file not found";
		}
		catch (Poco::PathNotFoundException&)
		{
			err = "path not found";
		}
		catch (Poco::Exception& exc)
		{
			err = exc.displayText();
		}

		if (!onCompleteCallback)
			return;

		ValueList cb_args;
		cb_args.push_back(Value::NewBool(ret));
		if (!ret)
		{
			cb_args.push_back(Value::NewString(err));
		}
		RunOnMainThread(onCompleteCallback, cb_args);
	}


	void File::MD5Digest(const ValueList& args, KValueRef result)
	{
		if (args.size() < 1)
		{
			throw ValueException::FromString("please provide call back as argument");
		}

		KMethodRef onCompleteCallback = NULL;
		if (args.at(0)->IsMethod())
		{
			onCompleteCallback = args.at(0)->ToMethod();
		}
		else
		{
			throw ValueException::FromString("invalid argument - second argument must be method callback");
		}

		struct MD5DigestStruct * str = new MD5DigestStruct(this, onCompleteCallback);
		TiAsyncJobRunnerSingleton::Instance()->enqueue(new TiThreadTarget(&File::MD5DigestThread, str));
	}

	void File::MD5DigestThread(void * param)
	{
		struct MD5DigestStruct * str = static_cast<struct MD5DigestStruct *>(param);
		if (str->file)
		{
			str->file->getMD5Digest(str->onCompleteCallback);
		}
	}

	void File::getMD5Digest(KMethodRef onCompleteCallback) const
	{
		if (onCompleteCallback)
		{
			ValueList args;
			try
			{
				std::string md5_string = MD5Utils::calculate_md5_of_file(this->filename);
				args.push_back(Value::NewBool(true));
				args.push_back(Value::NewString(md5_string));
			}
			catch (std::exception & e)
			{
				args.push_back(Value::NewBool(false));
				args.push_back(Value::NewString(e.what()));
			}

			RunOnMainThread(onCompleteCallback, args);
		}
	}
}
