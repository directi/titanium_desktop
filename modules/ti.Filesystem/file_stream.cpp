/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "file_stream.h"
#include <cstring>
#include <sstream>

namespace ti 
{
	Logger* getLogger()
	{
		return Logger::Get("Filesystem.FileStream");
	}

	FileStream::FileStream(const std::string& filenameIn)
		: StaticBoundObject("Filesystem.FileStream"),
		stream()
	{
#ifdef OS_OSX
		// in OSX, we need to expand ~ in paths to their absolute path value
		// we do that with a nifty helper method in NSString
		this->filename = [[[NSString stringWithUTF8String:filenameIn.c_str()]
			 stringByExpandingTildeInPath] fileSystemRepresentation];
#else
		this->filename = filenameIn;
#endif

		this->SetMethod("open", &FileStream::Open);
		this->SetMethod("close", &FileStream::Close);
		this->SetMethod("read", &FileStream::Read);
		this->SetMethod("readLine", &FileStream::ReadLine);
		this->SetMethod("write", &FileStream::Write);
		this->SetMethod("writeLine", &FileStream::WriteLine);
		this->SetMethod("ready", &FileStream::Ready);
		this->SetMethod("isOpen", &FileStream::IsOpen);
		this->SetMethod("seek", &FileStream::Seek);
		this->SetMethod("tell", &FileStream::Tell);
	}

	FileStream::~FileStream()
	{
		this->Close();
	}

	void FileStream::Open(const ValueList& args, KValueRef result)
	{
		args.VerifyException("open", "?ibb");

		this->mode = (FileStreamMode) args.GetInt(0, MODE_READ);
		bool binary = args.GetBool(1, false);
		bool append = args.GetBool(2, false);
		result->SetBool(this->Open(this->mode, binary, append));
	}

	bool FileStream::Open(FileStreamMode mode, bool binary, bool append)
	{
		// close the prev stream if needed
		this->Close();

		try
		{
			std::ios::openmode flags = (std::ios::openmode) 0;
			bool output = false;

			if (binary)
			{
				flags|=std::ios::binary;
			}

			if (mode == MODE_APPEND)
			{
				flags|=std::ios::out|std::ios::app;
				output = true;
			}
			else if (mode == MODE_WRITE)
			{
				flags |= std::ios::out|std::ios::trunc;
				output = true;
			}
			else if (mode == MODE_READ)
			{
				flags |= std::ios::in;
			}
			else if (mode == MODE_READ_WRITE)
			{
				flags |= std::ios::in | std::ios::out;
			}

			this->stream.open(this->filename.c_str(), flags);
#ifndef OS_WIN32
			if (output)
			{
				chmod(this->filename.c_str(),S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
			}
#endif
		}
		catch (std::exception& e)
		{
			getLogger()->Error("Error in open. Exception: %s", e.what());
			throw ValueException::FromString(e.what());
		}
		return true;
	}

	void FileStream::Close(const ValueList& args, KValueRef result)
	{
		bool closed = this->Close();
		result->SetBool(closed);
	}

	bool FileStream::Close()
	{
		bool bRet = false;
		try
		{
			if (this->stream.is_open())
			{
				this->stream.close();
				bRet = true;
			}
		}
		catch (std::exception& e)
		{
			getLogger()->Error("Error in close. Exception: %s",e.what());
			throw ValueException::FromString(e.what());
		}
		return bRet;
	}

	void FileStream::Write(const ValueList& args, KValueRef result)
	{
		args.VerifyException("write", "s|o|n");

		char *text = NULL;
		int size = 0;
		std::string data;
		if (args.at(0)->IsObject())
		{
			KObjectRef b = args.at(0)->ToObject();
			AutoPtr<Bytes> bytes = b.cast<Bytes>();
			if (!bytes.isNull())
			{
				text = (char*)bytes->Get();
				size = (int)bytes->Length();
			}
		}
		else if (args.at(0)->IsString())
		{
			data = args.at(0)->ToString();
			text = (char*)data.c_str();
		}
		else if (args.at(0)->IsInt())
		{
			std::stringstream ostr;
			ostr << args.at(0)->ToInt();
			text = (char*)ostr.str().c_str();
			size = ostr.str().length();
		}
		else if (args.at(0)->IsDouble())
		{
			std::stringstream ostr;
			ostr << args.at(0)->ToDouble();
			text = (char*)ostr.str().c_str();
			size = ostr.str().length();
		}
		else
		{
			throw ValueException::FromString("Could not write given type");
		}

		if (size == 0)
		{
			size = strlen(text);
		}

		if (text == NULL || size <= 0)
		{
			result->SetBool(false);
			return;
		}

		Write(text, size);
		result->SetBool(true);
	}

	void FileStream::Write(char *text, int size)
	{
		try
		{
			this->stream.write(text, size);
		}
		catch (std::exception& e)
		{
			getLogger()->Error("Error in write. Exception: %s",e.what());
			throw ValueException::FromString(e.what());
		}
	}

	void FileStream::Read(const ValueList& args, KValueRef result)
	{
		args.VerifyException("read", "?i");

		try
		{
			if (args.size() >= 1)
			{
				int size = args.GetInt(0);
				if (size <= 0)
				{
					throw ValueException::FromString("File.read() size must be greater than zero");
				}

				char* buffer = new char[size + 1];
				this->stream.read(buffer, size);

				int readCount = this->stream.gcount();
				if (readCount > 0)
				{
					// Store read data into a byte blob
					buffer[readCount] = '\0';
					result->SetObject(new Bytes(buffer, readCount, false));
				}
				else
				{
					// No data read, must be at EOF
					result->SetNull();
				}
			}
			else
			{
				// If no read size is provided, read the entire file.
				// TODO: this is not a very efficent method and should be deprecated
				// and replaced by buffered stream transports
				std::vector<char> buffer;
				char data[4096];

				while (!this->stream.eof())
				{
					this->stream.read((char*)&data, 4095);
					int length = this->stream.gcount();
					if (length > 0)
					{
						buffer.insert(buffer.end(), data, data+length);
					}
					else break;
				}

				if(buffer.size())
				{
					result->SetObject(new Bytes(&(buffer[0]), buffer.size()));
				}
				else
				{
					char * buf = NULL;
					result->SetObject(new Bytes(buf, 0, true));
				}
			}
		}
		catch (std::exception& e)
		{
			getLogger()->Error("Error in read. Exception: %s",e.what());
			throw ValueException::FromString(e.what());
		}
	}

	void FileStream::ReadLine(const ValueList& args, KValueRef result)
	{
		try
		{
			if (this->stream.eof())
			{
				// close the file
				result->SetNull();
			}
			else
			{
				std::string line;
				std::getline(this->stream, line);
#ifdef OS_WIN32
				// In some cases std::getline leaves a CR on the end of the line in win32 -- why God, why?
				if (!line.empty())
				{
					char lastChar = line.at(line.size()-1);
					if (lastChar == '\r') {
						line = line.substr(0, line.size()-1);
					}
				}
#endif
				if (line.empty() || line.size() == 0)
				{
					if (this->stream.eof())
					{
						// if this is EOF, return null
						result->SetNull();
					}
					else
					{
						// this is an empty line, just empty Bytes object.
						result->SetObject(new Bytes());
					}
				}
				else
				{
					result->SetObject(new Bytes(line));
				}
			}
		}
		catch (std::exception& e)
		{
			getLogger()->Error("Error in readLine. Exception: %s",e.what());
			throw ValueException::FromString(e.what());
		}
	}

	void FileStream::WriteLine(const ValueList& args, KValueRef result)
	{
		args.VerifyException("writeLine", "s|o|n");

		char *text = NULL;
		int size = 0;
		if (args.at(0)->IsObject())
		{
			KObjectRef b = args.at(0)->ToObject();
			AutoPtr<Bytes> bytes = b.cast<Bytes>();
			if (!bytes.isNull())
			{
				text = (char*)bytes->Get();
				size = (int)bytes->Length();
			}
		}
		else if (args.at(0)->IsString())
		{
			text = (char*)args.at(0)->ToString().c_str();
		}
		else if (args.at(0)->IsInt())
		{
			std::stringstream ostr;
			ostr << args.at(0)->ToInt();
			text = (char*)ostr.str().c_str();
			size = ostr.str().length();
		}
		else if (args.at(0)->IsDouble())
		{
			std::stringstream ostr;
			ostr << args.at(0)->ToDouble();
			text = (char*)ostr.str().c_str();
			size = ostr.str().length();
		}
		else
		{
			throw ValueException::FromString("Could not write given type");
		}

		if (size==0)
		{
			size = strlen(text);
		}

		if (text == NULL || size <= 0)
		{
			result->SetBool(false);
			return;
		}

		std::string astr = text;
#ifdef OS_WIN32
		astr += "\r\n";
#else
		astr += "\n";
#endif
		Write((char*)astr.c_str(),astr.length());
		result->SetBool(true);
	}

	void FileStream::Ready(const ValueList& args, KValueRef result)
	{
		result->SetBool((this->stream.is_open() == true)
			&& (this->stream.eof() == false));
	}

	void FileStream::IsOpen(const ValueList& args, KValueRef result)
	{
		result->SetBool(this->stream.is_open());
	}

	void FileStream::Seek(const ValueList& args, KValueRef result)
	{
		args.VerifyException("seek", "i?i");

		int offset = args.GetInt(0);
		std::ios::seekdir dir = (std::ios::seekdir)args.GetInt(1, std::ios::beg);

		if (mode == MODE_READ)
		{
			this->stream.seekg(offset, dir);
		}
		else if (mode == MODE_APPEND
			|| mode == MODE_WRITE
			|| mode == MODE_READ_WRITE)
		{
			this->stream.seekp(offset, dir);
		}
		else
		{
			throw ValueException::FromString("FileStream must be opened before seeking");
		}
	}

	void FileStream::Tell(const ValueList& args, KValueRef result)
	{
		if (mode == MODE_READ)
		{
			result->SetInt(this->stream.tellg());
		}
		else if (mode == MODE_APPEND
			|| mode == MODE_WRITE
			|| mode == MODE_READ_WRITE)
		{
			result->SetInt(this->stream.tellp());
		}
		else
		{
			throw ValueException::FromString("FileStream must be opend before using tell");
		}
	}

}
