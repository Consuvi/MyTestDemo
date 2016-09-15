
#include "windows.h"
#include "process.h"
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <iostream>
#include <stdlib.h>
using namespace std;

typedef function<void(void)> StdClosure;

class AutoLock
{
public:
	AutoLock(CRITICAL_SECTION * lock)
	{
		_lock = lock;
		if (_lock->LockCount == 0)
		{
			InitializeCriticalSection(_lock);
		}
		EnterCriticalSection(_lock);
	}
	~AutoLock()
	{
		LeaveCriticalSection(_lock);
		//DeleteCriticalSection(_lock);
	}
private:
	CRITICAL_SECTION * _lock;
};

#define MAX_THREAD_NUM 2
CRITICAL_SECTION wait_signal_lock;
class ThreadManager
{
public:

	ThreadManager() 
	{
		//InitializeCriticalSection(&wait_signal_lock);
		cout << "Construct ThreadManager" << endl;
		hEvent = CreateEvent(0, 0, 0, "ThreadManager_Pull_task_Event");
		hEvent_Idle = CreateEvent(0, 0, 0, "ThreadManager_Pull_task_Event");
		ResetEvent(hEvent_Idle);
		reset();
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)pull_task,this,0,0);//创建拉任务线程
	}
	~ThreadManager() 
	{
		cout << "Destruct ThreadManager" << endl;
	}

	static ThreadManager * GetInstance()
	{
		static ThreadManager manager;
		return &manager;
	}

	static UINT idle_loop(void * lp)
	{
		ThreadManager * pThis = (ThreadManager*)lp;
		pThis->DoIdle();
		return 0;
	}

	int GetIdleTaskSize()
	{
		AutoLock lock(&lock_task_idle);
		return IdleTaskList_bak.size();
	}

	static unsigned int pull_task(void * lp)
	{
		ThreadManager * pThis = (ThreadManager*)lp;
		int size = 0;
		while (1)
		{
			if (WAIT_TIMEOUT == pThis->wait())
			{
				pThis->reset();
				while (pThis->GetThreadCount())
				{
					pThis->swap_task();
					if ((size = pThis->GetTaskCount()) == 0)
					{
						Sleep(1000);
					}
					else
					{
						break;
					}
				}
				pThis->swap_idle();
				int idle_count = pThis->GetIdleTaskSize();
				for (int i=0;i < idle_count; i++)
				{//如果任务列表没有任务了,就执行空闲任务
					pThis->swap_task();
					if((size = pThis->GetTaskCount()) == 0)
					{
						while (pThis->get_thread_size())
						{
							Sleep(1000);
						}
						cout << "create idle thread" << endl;
						CreateThread(0, 0, (LPTHREAD_START_ROUTINE)idle_loop, pThis, 0, 0);
					}
					else
					{
						break;
					}
				}
				continue;
			}
			else
			{
				pThis->reset();
				pThis->swap_task();
				size = pThis->GetTaskCount();
			}

			for (int i = 0;i<size;i++)
			{
				cout << "pull_task" << endl;
				while (pThis->GetThreadCount() >= MAX_THREAD_NUM)
				{
					Sleep(1000);
				}
				CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_loop, pThis, 0, 0);
			}
			//等待所有任务创建完
			while (pThis->GetTaskCount())
			{
				Sleep(1000);
			}
		}
		return 0;
	}

	static unsigned int thread_loop(void * lp)
	{
		ThreadManager * pThis = (ThreadManager*)lp;
		try
		{
			pThis->run();
		}
		catch (...)
		{
			puts("catch");
		}
		return 0;
	}

	void DoIdle()
	{
		AutoThread t(this);
		AutoLock lock(&lock_task_idle);
		if (IdleTaskList_bak.size())
		{
			StdClosure cb = IdleTaskList_bak.front();
			IdleTaskList_bak.pop_front();
			lock.~AutoLock();
			cb();
		}
	}

	int GetTaskCount()
	{
		AutoLock lock(&lock_task);
		return TaskList_bak.size();
	}

	int wait()
	{
		return WaitForSingleObject(hEvent, 3000);
	}
	void reset()
	{
		ResetEvent(hEvent);
	}

	//开始执行任务
	void start()
	{
		SetEvent(hEvent);
	}

	void save_thread()
	{
		DWORD id = GetCurrentThreadId();
		AutoLock lock(&thread_lock);
		thread_pool[id] = GetCurrentThread();
	}

	void swap_idle()
	{
		AutoLock lock(&lock_task_idle);
		IdleTaskList.swap(IdleTaskList_bak);
	}

	void push_idle(std::function<void()> & cb)
	{
		IdleTaskList.push_back(cb);
	}

	class AutoThread
	{
	public:
		AutoThread(ThreadManager * tm)
		{
			_tm = tm;
			_tm->save_thread();
		}
		~AutoThread()
		{
			_tm->remove_thread();
		}

	private:
		ThreadManager * _tm;
	};

	void remove_thread()
	{
		DWORD id = GetCurrentThreadId();
		AutoLock lock(&thread_lock);
		thread_pool.erase(id);
	}

	int get_thread_size()
	{
		AutoLock lock(&thread_lock);
		return thread_pool.size();
	}
	//执行任务,先插入的先执行
	void run()
	{/*
		cout << "run" << endl;*/
		AutoThread t(this);
		{
			AutoLock lock(&lock_task);
			if (TaskList_bak.size() == 0)
			{
				return;
			}
			StdClosure cb = TaskList_bak.front();
			puts("pop task");
			TaskList_bak.pop_front();
			puts("end pop task");
			lock.~AutoLock();
			cb();
		}
		puts("done");
	}

	//一次性拉所有任务
	void swap_task()
	{
		AutoLock lock(&lock_task);
		puts("swap task");
		TaskList.swap(TaskList_bak);
	}
	//添加任务链表
	void push_task(StdClosure & cb)
	{
		TaskList.push_back(cb);
		start();
	}

	int GetThreadCount()
	{
		AutoLock lock(&thread_lock);
		return thread_pool.size();
	}
public:
	list<StdClosure> TaskList;//任务链表
	list<StdClosure> TaskList_bak;//任务链表缓冲
	list<StdClosure> IdleTaskList;//空闲任务链表
	list<StdClosure> IdleTaskList_bak;//空闲任务链表缓冲
	HANDLE hEvent;//等待创建任务线程,来了一条就创建一个线程去执行
	HANDLE hEvent_Idle;//等待创建任务线程,来了一条就创建一个线程去执行
	CRITICAL_SECTION lock_task;
	map<DWORD,HANDLE> thread_pool;
	CRITICAL_SECTION thread_lock;
	CRITICAL_SECTION lock_task_idle;
};

struct Data
{
	string name;
	int id;
	char * data;
	
	~Data()
	{
		printf("data 析构 addr: %p %d %s\n", data,id,data);
		if(data)
		delete data;
		data = NULL;
		name = "";
		id = 0;
	}
};

void test1(Data & dt)
{
	printf("test data addr: %p \n", &dt);
	for (int i = 0;i<20;i++)
	{
		cout << dt.name.c_str() << dt.id << endl;
		Sleep(1000);
	}
}

void test(string & str)
{
	for (int i = 0;i<20;i++)
	{
		cout << str.c_str() << endl;
		Sleep(1000);
	}
}

void PostIdleTask(std::function<void(void)> cb)
{
	ThreadManager::GetInstance()->push_idle(cb);
}

void PostTask(StdClosure  cb)
{
	//cout << "PostTask" << endl;
	ThreadManager::GetInstance()->push_task(cb);
}

void test2(vector<Data> & dts)
{
	for (auto a : dts)
	{
		cout << a.name.c_str() << a.id << endl;
		if (a.id == 2)
		{
			dts._Pop_back_n(3);
		}
		Sleep(1000);
	}
}

int main(int argc,char * argv[])
{
	cout << "begin" << endl;
	ThreadManager * tm = ThreadManager::GetInstance();
	{
		/*vector<Data> datas;
		for (int i = 0;i<10;i++)
		{
			datas.push_back(Data{"sssss",i,0 });
		}*/
 		Data da;
 		da.id = 8;
 		da.data = new char[10];
 		memset(da.data, 0, 10);
 		strcpy(da.data, "hello");
 		printf("main 绑定 %p %s\n",&da.data, da.data);
		std::bind(test1, da);
		PostTask(std::bind(test1,da));
		//PostTask(std::bind(test2, datas));
	}
// 	{
// 		string str = "ssssssssss";
// 		PostIdleTask(std::bind(test, str));
// 	}
// 	//Sleep(100);
// 	PostIdleTask(std::bind(test, string("4444")));
// 	PostIdleTask(std::bind(test, string("11111")));
// 	PostIdleTask(std::bind(test, string("2222222")));
// 	PostIdleTask(std::bind(test, string("333333")));
	while (true);
	return 0;
}