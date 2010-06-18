/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_ZIP_DECOMPRESS_H
#define _TI_ZIP_DECOMPRESS_H

#include <kroll/kroll.h>

#ifdef OS_WIN32
#include <windows.h>
#elif OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <string>
#include <vector>
#include <Poco/Path.h>
#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include <Poco/Zip/ZipArchive.h>


namespace ti
{
	class ZipFile : public StaticBoundObject
	{
		public:
			ZipFile(const std::string & zipFileName);
			virtual ~ZipFile();

		private:
			std::vector<std::string> files;
			const std::string zipFileName;
			KMethodRef onCompleteCallback;
			KMethodRef progressCallback;
			Poco::Thread *thread;
			
			void ToString(const ValueList& args, KValueRef result);
			void GetFileCount(const ValueList& args, KValueRef result);

			static void Run(void*);

			int getFileCount();

			void DecompressAll(const ValueList& args, KValueRef result);

			// Internal functions
			void DecompressAll(const std::string & destDir, KMethodRef onCompleteCallback, KMethodRef progressCallback);
			void onDecompressError(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info);
			void onDecompressOk(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& val);
			//void onDecompressOk(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info);
	};
}

#endif
