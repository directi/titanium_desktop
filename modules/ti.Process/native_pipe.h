/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
 
#ifndef _NATIVE_PIPE_H_
#define _NATIVE_PIPE_H_

#include "pipe.h"
#include <kroll/kroll.h>
#include <kroll/utils/Thread.h>

namespace ti
{
	class NativePipe : public Pipe
	{
	public:
		NativePipe(bool isReader);
		virtual ~NativePipe();
		void StartMonitor();
		void StartMonitor(KMethodRef readCallback);
		virtual void StopMonitors();
		virtual int Write(BytesRef bytes);
		void PollForWriteIteration();
		virtual void Close();
		virtual void CloseNative();
		virtual void CloseNativeRead() = 0;
		virtual void CloseNativeWrite() = 0;
		inline void SetReadCallback(KMethodRef cb) { this->readCallback = cb; }

	protected:
		bool closed;
		bool isReader;
		std::vector<KObjectRef> attachedObjects;
		kroll::Thread writeThread;
		kroll::Thread readThread;
		KMethodRef readCallback;
		Logger* logger;
		boost::mutex buffersMutex;
		std::queue<BytesRef> buffers;
		bool readThreadRunning;
		bool writeThreadRunning;

		void PollForReads();
		void PollForWrites();
		virtual void RawWrite(BytesRef bytes);
		virtual int RawRead(char *buffer, int size) = 0;
		virtual int RawWrite(const char *buffer, int size) = 0;
	};
}

#endif
