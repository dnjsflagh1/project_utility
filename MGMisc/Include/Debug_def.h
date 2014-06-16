#ifndef _DEBUG_DEFINE_H_
#define _DEBUG_DEFINE_H_
//#define _WIN32_WINDOWS		0x0500
#define EXCEPTION_DMP
#include <crtdbg.h>
//#include <stdio.h>
//#include "Windows.h"
//#include <malloc.h>
#define  _NO_CVCONST_H


#pragma warning( disable : 4996 )
namespace MG
{
	extern char DmpPath[1024];
	/*
	 *	VC6.0需要安装最新的Platform SDK － Debugging Tools for Windows
	 */

	//公用函数
	void*	memdup(const void* ptr, int size);
	int		DumpCallStack(LPEXCEPTION_POINTERS pException, DWORD code);//GetExceptionInformation(), GetExceptionCode()

	//安装自己的异常处理器
	void	InstallMyExceptHandle(const char* path);
	void	UnInstallMyExceptHandle();

	LONG WINAPI MyExceptHandle(struct _EXCEPTION_POINTERS *ExceptionInfo);
	void MessageOut(const char* msg, bool bPrint = false);
	void MessageOut(const wchar_t* msg, bool bPrint = false);


	BOOL DumpMemory(LPVOID lpVoid, int nSize, BOOL timeStamp=FALSE);
	BOOL CheckHeap(LPVOID lpVoid, int nSize);
	BOOLEAN Sprintf_s(char * dest, size_t size, char* format, ...);

//公用宏
#define CHECK_PTR(p) 			(IsBadReadPtr(p, sizeof(void*)) == 0 ? TRUE : FALSE)
#define CHECK_WRITE_PTR(p, n)	(IsBadWritePtr(p, n) == 0 ? TRUE : FALSE)
#define DUMP_CALLSTACK()        MG::DumpCallStack(GetExceptionInformation(), GetExceptionCode())
#define CLOSE_PROCESS()			{DUMP_STRING_TIME("COLSE_PROCESS\n");ExitProcess(0);}
#define MEMZERO(p)				memset(p, 0, sizeof(p))
#define STRNCPY(d, s, n)        {strncpy(d, s, n); d[n-1] = 0;}

#define DUMP_MEMORY(p,n)		MG::DumpMemory(p, n)
#define DUMP_MEMORY_TIME(p,n)	MG::DumpMemory(p, n, TRUE)
#define DUMP_STRING(str)		DUMP_MEMORY(str, strlen(str))
#define DUMP_STRING_TIME(str)	DUMP_MEMORY_TIME(str, (int)strlen(str))
#define CHECK_MEMORY(p, n)		::CheckHeap(p, n)
#define SAFESTRING(s, size)		{s[size-1] = 0;}


#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

//DEBUG
#ifdef _DEBUG
	#define ASSERT_DEF(f)	\
						do		\
						{		\
							if (!(f) && _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, 0, 0)) \
							__asm { int 3 }	\
						} while (0)	

	void Trace(char* lpszFormat, ...);
	
	#define OUT_STRING(s)			::OutputDebugStringA(s)
	#define TRACE_DEF					::Trace
#else//release
	#define ASSERT(f)			((void)0)
	#define OUT_STRING(s)		((void)0)
	#define TRACE				((void)0)
#endif	//_DEBUG


#if	1
	#ifdef	EXCEPTION_DMP
		#define MG_EXCEPTION_BEGIN __try{
		#define MG_EXCEPTION_END(funName) }_except(MG::RecordExceptionInfo(GetExceptionInformation(),funName)){}
	#else
		#define MG_EXCEPTION_BEGIN __try{
		#define MG_EXCEPTION_END(funName)	}__except(DUMP_CALLSTACK()){char buf[MAX_PATH];MG::Sprintf_s(buf, _countof(buf), funName);DUMP_STRING_TIME(buf);}
	#endif
#else
	#define MG_EXCEPTION_BEGIN 
	#define MG_EXCEPTION_END()
#endif 

}
#endif