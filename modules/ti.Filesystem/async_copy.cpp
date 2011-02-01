/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "async_copy.h"
#include "filesystem_binding.h"
#include "TiAsyncJobRunner.h"

#include <kroll/thread_manager.h>
#include <kroll/utils/file_utils.h>
#include <iostream>
#include <sstream>
#include <Poco/File.h>
#include <boost/filesystem.hpp>

#ifndef OS_WIN32
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif

namespace ti
{
	std::string getFileName(const std::string& path)
	{
		boost::filesystem::path boost_path(path.c_str());
		return boost_path.leaf();
	}


	Logger* GetLogger()
	{
		return Logger::Get("Filesystem.AsyncCopy");
	}

	AsyncCopy::AsyncCopy(const std::vector<std::string> &files,
		const std::string destination,
		KMethodRef callback)
		: StaticBoundObject("Filesystem.AsyncCopy"),
		files(files),
		destination(destination),
		callback(callback),
		stopped(false)
	{
		TiAsyncJobRunnerSingleton::Instance()->enqueue(new TiThreadTarget(&AsyncCopy::Run, this));
	}

	AsyncCopy::~AsyncCopy()
	{
	}

	void AsyncCopy::Copy(const std::string& src, const std::string& dest)
	{
		Logger* logger = GetLogger();
		bool isLink = FileUtils::IsLink(src);

		logger->Debug("file=%s dest=%s link=%i", src.c_str(), dest.c_str(), isLink);
#ifndef OS_WIN32
		if (isLink)
		{
			char linkPath[PATH_MAX];
			ssize_t length = readlink(src.c_str(), linkPath, PATH_MAX);
			linkPath[length] = '\0';

			std::string newPath (dest);
			const char *destPath = newPath.c_str();
			unlink(destPath); // unlink it first, fails in some OS if already there
			int result = symlink(linkPath, destPath);

			if (result == -1)
			{
				std::string err = "Copy failed: Could not make symlink (";
				err.append(destPath);
				err.append(") from ");
				err.append(linkPath);
				err.append(" : ");
				err.append(strerror(errno));
				throw kroll::ValueException::FromString(err);
			}
		}
#endif
		if (!isLink && FileUtils::IsDirectory(src))
		{
			bool recursive = true;
			FileUtils::CreateDirectory(dest, recursive);
			std::vector<std::string> files;
			Poco::File from(src);
			from.list(files);
			std::vector<std::string>::iterator i = files.begin();
			while(i!=files.end())
			{
				std::string fn = (*i++);
				std::string sp = FileUtils::Join(src.c_str(), fn.c_str(),NULL);
				std::string dp = FileUtils::Join(dest.c_str(), fn.c_str(),NULL);
				this->Copy(sp, dp);
			}
		}
		else if (!isLink)
		{
			// in this case it's a regular file
			Poco::File s(src);
			s.copyTo(dest.c_str());
		}
	}

	void AsyncCopy::run()
	{
		Logger* logger = GetLogger();
		logger->Debug("Job started: dest=%s, count=%i", this->destination.c_str(), this->files.size());
		FileUtils::CreateDirectory(this->destination, /*recursive*/ true);
		int c = 0;

		std::vector<std::string>::const_iterator iter = this->files.begin();
		while (!this->stopped && iter!=this->files.end())
		{
			bool err_copy = false;
			std::string file = (*iter++);
			c++;

			logger->Debug("File: path=%s, count=%i\n", file.c_str(), c);
			try
			{
				if (FileUtils::IsDirectory(file))
				{
					this->Copy(file, this->destination);
				}
				else
				{
					std::string destPath = FileUtils::Join(this->destination.c_str(), getFileName(file).c_str(),NULL);
					this->Copy(file, destPath);
				}
				logger->Debug("File copied");

				KValueRef value = Value::NewString(file);
				ValueList args;
				args.push_back(value);
				args.push_back(Value::NewInt(c));
				args.push_back(Value::NewInt(this->files.size()));
				args.push_back(Value::NewBool(true));
				RunOnMainThread(this->callback, args);

				logger->Debug("Callback executed");
			}
			catch (ValueException &ex)
			{
				err_copy = true;
				logger->Error(std::string("Error: ") + ex.ToString() + " for file: " + file);
			}
			catch (std::exception &ex)
			{
				err_copy = true;
				logger->Error(std::string("Error: ") + ex.what() + " for file: " + file);
			}
			catch (...)
			{
				err_copy = true;
				logger->Error(std::string("Unknown error during copy: ") + file);
			}
			try
			{
				if(err_copy)
				{
					KValueRef value = Value::NewString(file);
					ValueList args;
					args.push_back(value);
					args.push_back(Value::NewInt(c));
					args.push_back(Value::NewInt(this->files.size()));
					args.push_back(Value::NewBool(false));
					RunOnMainThread(this->callback, args);
				}
			}
			catch(...)
			{
				err_copy = true;
				logger->Error(std::string("Unknown error during copy: ") + file);
			}

		}
		this->stopped = true;

		logger->Debug(std::string("Job finished"));
	}

	void AsyncCopy::Run(void* data)
	{
		START_KROLL_THREAD;

		AsyncCopy* ac = static_cast<AsyncCopy*>(data);
		ac->run();

		END_KROLL_THREAD;
	}

	void AsyncCopy::ToString(const ValueList& args, KValueRef result)
	{
		result->SetString("[Async Copy]");
	}

	void AsyncCopy::Cancel(const ValueList& args, KValueRef result)
	{
		if (!this->stopped)
		{
			this->stopped = true;
			result->SetBool(true);
		}
		result->SetBool(!this->stopped);
	}
}
