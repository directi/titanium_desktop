/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "XMLLoggerFile.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#ifndef OS_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef OS_LINUX
#include <sys/statvfs.h>
#endif

#ifdef OS_WIN32
#define MIN_PATH_LENGTH 3
#else
#define MIN_PATH_LENGTH 1
#endif


namespace ti
{
	XMLLoggerFile::XMLLoggerFile(const std::string &filename, const std::string &_rootXMLText, const std::string &_xsltFile)
		: LoggerFile(filename),
		rootXMLText(_rootXMLText),
		xsltFile(_xsltFile)
	{
	}

	XMLLoggerFile::~XMLLoggerFile()
	{
	}

	void XMLLoggerFile::dumpToFile()
	{
		std::list<std::string> *tempWriteQueue = NULL;

		if(!writeQueue.empty())
		{
			Poco::Mutex::ScopedLock lock(loggerMutex);
			tempWriteQueue = new std::list<std::string>(writeQueue.size());
			std::copy(writeQueue.begin(), writeQueue.end(), tempWriteQueue->begin()); 
			writeQueue.clear();
		}
		if (tempWriteQueue)
		{
			Poco::File file(this->filename);
			if(!file.exists())
			{
				createStubFile();
			}
			try
			{
				std::fstream stream;
				stream.open(filename.c_str(), std::ios_base::out | std::ios_base::in);

				if (stream.is_open())
				{
					stream.seekp(-(3 + rootXMLText.size()), std::ios_base::end);

					while (!tempWriteQueue->empty())
					{
						stream.write(tempWriteQueue->front().c_str(), tempWriteQueue->front().size());
						tempWriteQueue->pop_front();
					}
					std::string rootEnd = "</";
					rootEnd += rootXMLText + ">";
					stream.write(rootEnd.c_str(), rootEnd.size());

					stream.close();
					delete tempWriteQueue;
					tempWriteQueue = NULL;
				}
			}
			catch (...)
			{
				if(tempWriteQueue != NULL)
					delete tempWriteQueue;
				throw;
			}
		}
	}
	void XMLLoggerFile::createStubFile() const
	{
		std::ofstream stream;
		stream.open(filename.c_str(), std::ofstream::app);

		if (stream.is_open())
		{
			const std::string xmlversion("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
			stream.write(xmlversion.c_str(), xmlversion.size());

			if(xsltFile.size())
			{
				std::string xslt = "<?xml-stylesheet type=\"text/xsl\" href=\"";
				xslt += xsltFile;
				xslt += "\"?>\n";
				stream.write(xslt.c_str(), xslt.size());
			}

			std::string rootStart = "<";
			rootStart += rootXMLText + ">\n";
			std::string rootEnd = "</";
			rootEnd += rootXMLText + ">";
			stream.write(rootStart.c_str(), rootStart.size());
			stream.write(rootEnd.c_str(), rootEnd.size());
			stream.close();
		}
	}

}
