/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_ZIP_DECOMPRESS_H
#define _TI_ZIP_DECOMPRESS_H

#include <kroll/kroll.h>
#include <kroll/utils/zip_utils.h>

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
			KMethodRef onDecompressCompleteCallback;
			KMethodRef decompressProgressCallback;
			
			void ToString(const ValueList& args, KValueRef result);
			void GetFileCount(const ValueList& args, KValueRef result);
			void DecompressAll(const ValueList& args, KValueRef result);

			// Internal functions
			int getFileCount() const;
			bool createDirectory(const std::string & dir, std::string & error) const;
			void DecompressAll(const std::string & destDir);
			void notifyDecompressComplete(bool ret, const std::string & err) const;
	};
}

#endif
