
#ifndef _NAMED_MUTEX_H_
#define _NAMED_MUTEX_H_

#include <base.h>
#include <string>

#ifdef OS_WIN32
#include <windows.h>
#endif

#ifdef OS_OSX
#import <Foundation/Foundation.h>
#import <semaphore.h>

class NamedMutexException : public std::exception
{
	const std::string text;
	public:
		NamedMutexException(const char * _text) : text(_text) {}
		virtual ~NamedMutexException() throw() {}
		virtual const char * what() const throw()
		{
			return text.c_str();
		}
};
#endif

namespace UTILS_NS
{
	class KROLL_API NamedMutex
	{
	public:
		NamedMutex(const std::string& name);
		~NamedMutex();
		void lock();
		bool tryLock();
		void unlock();

	private:
		const std::string  _name;
#ifdef OS_WIN32
		HANDLE       _mutex;
#elif OS_OSX
		sem_t* _sem;
#else
		int _lockfd; // lock file descriptor
		int _semfd;  // file used to identify semaphore
		int _semid;  // semaphore id
#endif
#ifndef OS_WIN32
		std::string getFileName();
#endif
	};
}
#endif // _NAMED_MUTEX_H_
