/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "zipFile.h"
#include "filesystem_binding.h"
#include <kroll/thread_manager.h>
#include <Poco/Zip/Decompress.h>
#include <Poco/Delegate.h>
#include <Poco/File.h>

#include <iostream>
#include <sstream>

#ifndef OS_WIN32
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif



namespace ti
{
	ZipFile::ZipFile(const std::string & zipFileName) :
			StaticBoundObject("Filesystem.ZipFile"),
			zipFileName(zipFileName),
			onDecompressCompleteCallback(NULL),
			decompressProgressCallback(NULL)
	{
		/**
		 * @tiapi(method=True,name=ZipFile.getFileCount) gives count of number of files in the archieve
		 */
		this->SetMethod("getFileCount",&ZipFile::GetFileCount);

		/**
		 * @tiapi(method=True,name=ZipFile.decompressAllFiles) decompresses the zip file
		 */
		this->SetMethod("decompressAllFiles",&ZipFile::DecompressAll);

		// Check weather the file exist or not
		std::ifstream inp(zipFileName.c_str(), std::ios::binary);
		if(!inp)
		{
			throw ValueException::FromString("Error opening file: " + zipFileName);
		}

	}
	ZipFile::~ZipFile()
	{
		abort();
		/*if (this->thread)
		{
			this->thread->tryJoin(10); // precaution, should already be stopped
			delete this->thread;
			this->thread = NULL;
		}*/
	}

	void ZipFile::ToString(const ValueList& args, KValueRef result)
	{
        std::string str("[Zip Decompress: " + this->zipFileName + "]");
		result->SetString(str.c_str());
	}

	void ZipFile::GetFileCount(const ValueList& args, KValueRef result)
	{
		result->SetDouble(getFileCount());
	}

	int ZipFile::getFileCount() const
	{
		int count=0;
		std::ifstream inp("c:\\test.zip", std::ios::binary);
		if(!inp)
		{
			throw ValueException::FromString("unable to open file");
		}

		typedef Poco::Zip::ZipArchive za_t;
		za_t za(inp);
		for (za_t::FileInfos::const_iterator i = za.fileInfoBegin();
			i != za.fileInfoEnd(); ++i)
		{
			if (!i->second.isDirectory()) ++count;
		}
		return count;
	}

	bool MyUnzipCallback(char* message, int current,
			int total, void* data)
	{
		return false;
	}

	void ZipFile::DecompressAll(const ValueList& args, KValueRef result)
	{
		std::string destDir;
		if (args.size() < 2)
		{
			throw ValueException::FromString("please provide at least 2 args");
		}
		if (args.at(0)->IsString())
		{
			destDir = args.at(0)->ToString();
		}
		else
		{
			throw ValueException::FromString("invalid argument - first argument must be destDir(string)");
		}

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

		if(this->onDecompressCompleteCallback)
		{
			throw ValueException::FromString("Another decompress is already in progress please try after some time.");
		}

		this->onDecompressCompleteCallback = onCompleteCallback;
		this->decompressProgressCallback = progressCallback;
		DecompressAll(destDir);
	}

	bool ZipFile::createDirectory(const std::string & dir, std::string & error) const
	{
		Poco::File pocodir(dir.c_str());
		if (!pocodir.exists())
		{
			if (!pocodir.createDirectory())
			{
				error = "Error creating directory " + pocodir.path();
				return false;
			}
		}
		else if (!pocodir.isDirectory())
		{
			error = "destination must be a directory";
			return false;
		}
		return true;
	}

	void ZipFile::notifyDecompressComplete(bool ret, const std::string & err) const
	{
		if (this->onDecompressCompleteCallback)
		{
			ValueList cb_args;
			cb_args.push_back(Value::NewBool(ret));
			if (!ret)
			{
				cb_args.push_back(Value::NewString(err));
			}
			RunOnMainThread(this->onDecompressCompleteCallback, cb_args, false);
		}
	}

	void ZipFile::DecompressAll(const std::string & destDir)
	{
		bool ret = false;
		std::string err;
		try
		{
			Poco::File from(this->zipFileName);
			Poco::File to(destDir);

			if (createDirectory(destDir, err))
			{
				kroll::FileUtils::Unzip(from.path(), to.path());
				ret = true;
			}
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			err = "file not found";
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			err = "path not found";
		}
		catch (Poco::Exception& exc)
		{
			err = exc.displayText();
		}
		catch (...)
		{
			err = "unknown error";
		}
		notifyDecompressComplete(ret, err);

		this->onDecompressCompleteCallback = NULL;
		this->decompressProgressCallback = NULL;
	}
}
