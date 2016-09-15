
#include "windows.h"
#include "Dbghelp.h"
#include <functional>
#include <ctime>
#include <iostream>
#include <vector>
#include <list>
#pragma comment(lib,"Dbghelp.lib")
//#include <boost/bind.hpp>
// 
 using namespace std;

#if 0
class A
{
public:

	typedef std::function<void(int)> callback;

	void test()
	{
		cbb(5);
		cout << "test" << endl;
	}


	void SetCallBack(callback cb)
	{
		cbb = cb;
	}

	callback cbb;
};

class B
{
public:

	A a;
	void run()
	{
		a.SetCallBack(std::bind(&B::callBack, this, std::placeholders::_1, 5));
		a.test();
		cout << "run b" << endl;
	}

	void callBack(int a, int b)
	{
		cout << "callback b" << a << endl;
	}

	B(char * in)
	{
		tmp = new char[10];
		memset(tmp,0,10);
		strcat(tmp, in);
	}
	
	~B()
	{
		delete tmp;
		tmp = NULL;
	}
	char * tmp;
};
#endif 

long WINAPI writedump(_In_ struct _EXCEPTION_POINTERS *ExceptionInfo )
{
	cout << "writedump" << __LINE__ << endl;
	HANDLE hFile = CreateFile(("MiniDump.dmp"), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		cout << "writedump" << __LINE__ << endl;
		// Create the minidump

		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = ExceptionInfo;
		mdei.ClientPointers = FALSE;

		MINIDUMP_TYPE mdt = MiniDumpNormal;

		BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hFile, mdt, (ExceptionInfo != 0) ? &mdei : 0, 0, 0);


		// Close the file

		CloseHandle(hFile);

	}
	else
	{
		cout << "writedump" << __LINE__ << endl;
	}
	return 0;
}
const DWORD kVCThreadNameException = 0x406D1388;

typedef struct tagTHREADNAME_INFO {
	DWORD dwType;  // Must be 0x1000.
	LPCSTR szName;  // Pointer to name (in user addr space).
	DWORD dwThreadID;  // Thread ID (-1=caller thread).
	DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;

// This function has try handling, so it is separated out of its caller.
void SetNameInternal(DWORD thread_id, const char* name)
{
	//只在调试的时候生效 
	if (!::IsDebuggerPresent())
		return;
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = thread_id;
	info.dwFlags = 0;

	__try
	{
		RaiseException(kVCThreadNameException, 0, sizeof(info) / sizeof(DWORD), reinterpret_cast<DWORD_PTR*>(&info));
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

int main(int argc, char * argv[])
{
	//设置线程名
	SetNameInternal(GetCurrentThreadId(),"ssssss");
	HANDLE hCurThread = OpenThread(PROCESS_ALL_ACCESS, 0, GetCurrentThreadId());
	
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)writedump);
	int i = 0;
	int b = 5 / i;
	char * buf = new char[2];
	delete buf;
	buf[0] = 1;
	return 0;
	/*list<int> v1,v2,tmp;
	for (int i = 5;i>=0;i--)
	{
		v1.push_back(i);
	}

	for (int i = 10; i >= 6; i--)
	{
		v2.push_back(i);
	}

	swap(v1, v2);
	return 0;

	char * buf1 = new char[50];
	memset(buf1, 0, 50);
	char * buf2 = new char[50];
	memset(buf2, 0, 50);

	strcpy(buf1, "hello");
	strcpy(buf2, "world");
	swap(buf1, buf2);
	cout << buf1 << endl;
	cout << buf2 << endl;
	return 0;

	B b1("hello");
	B b2 = std::move(b1);
	b1.~B();
	cout << b2.tmp << endl;
	return 0;*/
	
	/*B b;
	b.run();
	return 0;*/
}