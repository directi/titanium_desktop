
#include "NamedMutex.h"

namespace UTILS_NS
{
#if defined(linux) || defined(__CYGWIN__)
	union semun
	{
		int                 val;
		struct semid_ds*    buf;
		unsigned short int* array;
		struct seminfo*     __buf;
	};
#elif defined(__hpux)
	union semun
	{
		int              val;
		struct semid_ds* buf;
		ushort*          array;
	};
#endif

#ifndef OS_WIN32
std::string NamedMutexImpl::getFileName()
{
#if defined(sun) || defined(__APPLE__) || defined(__QNX__)
	std::string fn = "/";
#else
	std::string fn = "/tmp/";
#endif
	fn.append(_name);
	fn.append(".mutex");
	return fn;
}
#endif

NamedMutex::NamedMutex(const std::string& name)
	: _name(name)
#ifdef OS_WIN32
	, _mutex(CreateMutexA(NULL, FALSE, _name.c_str()))
#endif
{
#ifdef OS_WIN32
	if (!_mutex) 
	{
		throw std::exception("cannot create named mutex");
	}
#else
	std::string fileName = getFileName();
#if OS_OSX
	_sem = sem_open(fileName.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 1);
	if ((long) _sem == (long) SEM_FAILED) 
	{
		std::string err("cannot create named mutex (sem_open() failed)" + _name);
		throw std::exception(err.c_str());
	}
#else
	int fd = open(fileName.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd != -1)
		close(fd);
	else
	{
		std::string err("cannot create named mutex (lockfile)" + _name);
		throw std::exception(err.c_str());
	}
	key_t key = ftok(fileName.c_str(), 0);
	if (key == -1)
	{
		std::string err("cannot create named mutex (ftok() failed)" + _name);
		throw std::exception(err.c_str());
	}
	_semid = semget(key, 1, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | IPC_CREAT | IPC_EXCL);
	if (_semid >= 0)
	{
		union semun arg;
		arg.val = 1;
		semctl(_semid, 0, SETVAL, arg);
	}
	else if (errno == EEXIST)
	{
		_semid = semget(key, 1, 0);
	}
	else
	{
		std::string err("cannot create named mutex (semget() failed)" + _name);
		throw std::exception(err.c_str());
	}
#endif // OS_OSX

#endif // OS_WIN32
}


NamedMutex::~NamedMutex()
{
#ifdef OS_WIN32
	CloseHandle(_mutex);
#elif OS_OSX
	sem_close(_sem);
#endif
}


void NamedMutex::lock()
{
#ifdef OS_WIN32
	std::string err;
	switch (WaitForSingleObject(_mutex, INFINITE))
	{
	case WAIT_OBJECT_0:
		return;
	case WAIT_ABANDONED:
		err = "cannot lock named mutex (abadoned)" + _name;
		throw std::exception(err.c_str());
	default:
		err = "cannot lock named mutex" + _name;
		throw std::exception(err.c_str());
	}
#elif OS_OSX
	int err;
	do
	{
		err = sem_wait(_sem);
	}
	while (err && errno == EINTR);
	if (err)
	{
		std::string errstr("cannot lock named mutex" + _name);
		throw std::exception(errstr.c_str());
	}
#else
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op  = -1;
	op.sem_flg = SEM_UNDO;
	int err;
	do
	{
		err = semop(_semid, &op, 1);
	}
	while (err && errno == EINTR);
	if (err)
	{
		std::string errstr("cannot lock named mutex" + _name);
		throw std::exception(errstr.c_str());
	}
#endif
}


bool NamedMutex::tryLock()
{
#ifdef OS_WIN32
	std::string err;
	switch (WaitForSingleObject(_mutex, 0))
	{
	case WAIT_OBJECT_0:
		return true;
	case WAIT_TIMEOUT:
		return false;
	case WAIT_ABANDONED:
		err = "cannot lock named mutex (abadoned)" + _name;
		throw std::exception(err.c_str());
	default:
		err = "cannot lock named mutex" + _name;
		throw std::exception(err.c_str());
	}
#elif OS_OSX
	return sem_trywait(_sem) == 0;
#else
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op  = -1;
	op.sem_flg = SEM_UNDO | IPC_NOWAIT;
	return semop(_semid, &op, 1) == 0;
#endif
}


void NamedMutex::unlock()
{
#ifdef OS_WIN32
	ReleaseMutex(_mutex);
#elif OS_OSX
	if (sem_post(_sem) != 0)
	{
		std::string err("cannot unlock named mutex" + _name);
		throw std::exception(err.c_str());
	}
#else
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op  = 1;
	op.sem_flg = SEM_UNDO;
	if (semop(_semid, &op, 1) != 0)
	{
		std::string err("cannot unlock named mutex" + _name);
		throw std::exception(err.c_str());
	}
#endif
}
}