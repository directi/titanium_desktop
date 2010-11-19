/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_XMLLOGGERFILE_H_
#define _TI_XMLLOGGERFILE_H_

#ifdef OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <string>

#include <kroll/kroll.h>
#include <kroll/LoggerFile.h>

namespace ti
{
	class XMLLoggerFile
		: public kroll::LoggerFile
	{
		public:
			XMLLoggerFile(const std::string &filename, const std::string &_rootXMLText, const std::string &_xsltFile);
			virtual ~XMLLoggerFile();

			virtual void dumpToFile();

		private:
			const std::string rootXMLText;
			const std::string xsltFile;

			void createStubFile() const;
	};
}

#endif