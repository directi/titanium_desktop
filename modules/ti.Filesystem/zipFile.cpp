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
			thread(NULL)
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
		result->SetString("[Zip Decompress: " + this->zipFileName + "]");
	}

	void ZipFile::GetFileCount(const ValueList& args, KValueRef result)
	{
		result->SetDouble(getFileCount());
	}

	int ZipFile::getFileCount()
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


		DecompressAll(destDir, onCompleteCallback, progressCallback);
		//this->thread = new Poco::Thread();
		//this->thread->start(&ZipFile::Run,this);
	}

	void File::DecompressAll(const std::string & destDir, KMethodRef onCompleteCallback, KMethodRef progressCallback) const
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

		if (!onCompleteCallback)
			return;

		ValueList cb_args;
		cb_args.push_back(Value::NewBool(ret));
		if (!ret)
		{
			cb_args.push_back(Value::NewString(err));
		}
		RunOnMainThread(onCompleteCallback, cb_args, false);
	}

	
/*	void ZipFile::DecompressAll(const std::string & destDir, KMethodRef onCompleteCallback, KMethodRef progressCallback)
	{
		this->onCompleteCallback = onCompleteCallback;
		this->progressCallback = progressCallback;
		std::ifstream inp(zipFileName.c_str(), std::ios::binary);
		if(!inp)
		{
			throw ValueException::FromString("Error opening file: " + zipFileName);
		}

		Poco::Zip::Decompress dec(inp, Poco::Path(destDir));

		dec.EError += Poco::Delegate<ZipFile, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(this, &ZipFile::onDecompressError);
		dec.EOk += Poco::Delegate<ZipFile, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(this, &ZipFile::onDecompressOk);

		dec.decompressAllFiles();
		dec.EError -= Poco::Delegate<ZipFile, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(this, &ZipFile::onDecompressError);
		dec.EOk -= Poco::Delegate<ZipFile, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(this, &ZipFile::onDecompressOk);

		// calling back oncomplete callback
		ValueList args;
		RunOnMainThread(this->onCompleteCallback, args, false);

	}*/
	void ZipFile::onDecompressError(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info)
	{
		ValueList args;
		args.push_back(Value::NewBool(false));
		args.push_back(Value::NewString(info.second));
		if (this->progressCallback)
		{
			RunOnMainThread(this->progressCallback, args, false);
		}
	}

	void ZipFile::onDecompressOk(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& val)
	{
		ValueList args;
		args.push_back(Value::NewBool(true));
		if (this->progressCallback)
		{
			RunOnMainThread(this->progressCallback, args, false);
		}
	}

}
