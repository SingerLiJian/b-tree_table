#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
#include "BPlusTree.h"
#include "Table.h"

class Thread
{
public:
	Thread();
	~Thread();
	int ADD(void *ft);
    int Search(void *ft);
	int WaitForDeath();
	int ForDeath();

private:
    static void* StartFunctionOfAdd(void *ft);
    static void* StartFunctionOfSearch(void *ft);
private:
	pthread_t m_ThreadID; 
};

Thread::Thread()
{
}

Thread::~Thread()
{
}

int Thread::ADD(void *ft)
{	
	int r = pthread_create(&m_ThreadID, 0, StartFunctionOfAdd, ft);
	if(r != 0)
	{
		return -1;
	}
	return 0;
}

int Thread::Search(void *ft)
{	
	int r = pthread_create(&m_ThreadID, 0, StartFunctionOfSearch, ft);
	if(r != 0)
	{
		return -1;
	}
	return 0;
}

void* Thread::StartFunctionOfAdd(void *ft)
{
	table* t = (table*)ft;
    t->appendRAND();
}

void* Thread::StartFunctionOfSearch(void *ft)
{
	table* t = (table*)ft;
    int r = t->searchBPlus();
}

int Thread::WaitForDeath()
{
	int r = pthread_join(m_ThreadID, 0);
	if(r != 0)
	{
		return -1;
	}
	return 0;
}

int Thread::ForDeath()
{
	int r= pthread_detach(m_ThreadID);
	if(r != 0)
	{
		return -1;
	}
	return 0;
}
#endif
