#ifndef Mutex_H
#define Mutex_H
#include <pthread.h>

class Mutex
{
public:
	Mutex();
	virtual ~Mutex();

	int Lock();
	int Unlock();

private:
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);

private:
	pthread_mutex_t m_Mutex;
};

Mutex::Mutex()
{
	int  r = pthread_mutex_init(&m_Mutex, 0);
	if(r != 0)
	{
		throw "In Mutex::Mutex(), pthread_mutex_init error";
	}
}

Mutex::~Mutex()
{
	int r = pthread_mutex_destroy(&m_Mutex);
	if(r != 0)
	{
		throw "In Mutex::~Mutex(), pthread_mutex_destroy error";
	}
}

int Mutex::Lock()
{
	int r = pthread_mutex_lock(&m_Mutex);
	if(r != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int Mutex::Unlock()
{
	int r = pthread_mutex_unlock(&m_Mutex);
	if(r != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
#endif