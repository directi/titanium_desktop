/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef FILE_STREAM_H_
#define FILE_STREAM_H_

#include <kroll/kroll.h>
#include <string>
#include <fstream>

#ifdef OS_OSX
#import <Foundation/Foundation.h>
#endif


namespace ti 
{
	enum FileStreamMode
	{
		MODE_READ = 1,
		MODE_APPEND,
		MODE_WRITE,
		MODE_READ_WRITE
	};


	class FileStream : public StaticBoundObject 
	{
	public:
		FileStream(const std::string& filenameIn);
		virtual ~FileStream();

		// Used by File.open()
		void Open(const ValueList& args, KValueRef result);

	private:
		std::string filename;
		std::fstream stream;
		FileStreamMode mode;

		bool Open(FileStreamMode mode, bool binary = false, bool append = false);
		void Write(char *,int);
		bool Close();

		void Close(const ValueList& args, KValueRef result);
		void Write(const ValueList& args, KValueRef result);
		void Read(const ValueList& args, KValueRef result);
		void ReadLine(const ValueList& args, KValueRef result);
		void WriteLine(const ValueList& args, KValueRef result);
		void Ready(const ValueList& args, KValueRef result);
		void IsOpen(const ValueList& args, KValueRef result);
		void Seek(const ValueList& args, KValueRef result);
		void Tell(const ValueList& args, KValueRef result);
	};

}

#endif 
