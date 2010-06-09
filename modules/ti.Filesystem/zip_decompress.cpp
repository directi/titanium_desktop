/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "zip_decompress.h"
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
	ZipDecompress::ZipDecompress(const std::string & zipFileName) :
			StaticBoundObject("Filesystem.ZipDecompress"),
			zipFileName(zipFileName),
			thread(NULL)
	{
		/**
		 * @tiapi(method=True,name=ZipDecompress.decompressAllFiles) decompresses the zip file
		 */
		this->SetMethod("decompressAllFiles",&ZipDecompress::DecompressAll);

		// Check weather the file exist or not
		std::ifstream inp(zipFileName.c_str(), std::ios::binary);
		if(!inp)
		{
			throw ValueException::FromString("Error opening file: " + zipFileName);
		}

	}
	ZipDecompress::~ZipDecompress()
	{
		abort();
		/*if (this->thread)
		{
			this->thread->tryJoin(10); // precaution, should already be stopped
			delete this->thread;
			this->thread = NULL;
		}*/
	}

	void ZipDecompress::ToString(const ValueList& args, KValueRef result)
	{
		result->SetString("[Zip Decompress]");
	}
	void ZipDecompress::DecompressAll(const ValueList& args, KValueRef result)
	{
		std::string destDir;
		if (args.size()!=2)
		{
			throw ValueException::FromString("invalid arguments - this method takes 2 arguments");
		}
		if (args.at(0)->IsString())
		{
			destDir = args.at(0)->ToString();
		}
		else
		{
			throw ValueException::FromString("invalid argument - first argument must be destDir(string)");
		}
		KMethodRef callback;
		if (args.at(1)->IsMethod())
		{
			callback = args.at(1)->ToMethod();
		}
		else
		{
			throw ValueException::FromString("invalid argument - second argument must be method callback");
		}


		DecompressAll(destDir, callback);
		//this->thread = new Poco::Thread();
		//this->thread->start(&ZipDecompress::Run,this);
	}

	void ZipDecompress::DecompressAll(const std::string & destDir, KMethodRef callback)
	{
		this->callback = callback;
		std::ifstream inp(zipFileName.c_str(), std::ios::binary);
		if(!inp)
		{
			throw ValueException::FromString("Error opening file: " + zipFileName);
		}

		Poco::Zip::Decompress dec(inp, Poco::Path(destDir));

		dec.EError += Poco::Delegate<ZipDecompress, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(this, &ZipDecompress::onDecompressError);
		dec.EOk += Poco::Delegate<ZipDecompress, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(this, &ZipDecompress::onDecompressOk);

		dec.decompressAllFiles();
		dec.EError -= Poco::Delegate<ZipDecompress, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(this, &ZipDecompress::onDecompressError);
		dec.EOk -= Poco::Delegate<ZipDecompress, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(this, &ZipDecompress::onDecompressOk);

	}
	void ZipDecompress::onDecompressError(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info)
	{
		ValueList args;
		args.push_back(Value::NewBool(false));
		RunOnMainThread(this->callback, args, false);
	}

	void ZipDecompress::onDecompressOk(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& val)
	{
		ValueList args;
		args.push_back(Value::NewBool(true));
		RunOnMainThread(this->callback, args, false);
	}

}
