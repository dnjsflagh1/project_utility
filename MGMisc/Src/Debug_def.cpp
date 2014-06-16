#include "stdafx.h"
#include "Debug_def.h"
#include "Dbghelp.h"
#pragma comment(lib, "Dbghelp.lib")

namespace MG
{
	DWORD	offsetOfDebug = 0;
	CRITICAL_SECTION g_dbCriticalSection; 
	char DmpPath[1024]; 
	/*********************************************************/
	/*
	 *	公用函数
	 */
	/**********************************************************/
	void* memdup(const void* ptr, int size)
	{
		if ((ptr != NULL) && (size > 0))
		{
			void* p = malloc(size);
			if (p)
			{
				memcpy(p, ptr, size);
				return p;
			}
		}
		return NULL;
	}


	void Trace(char* lpszFormat, ...)
	{
		char msg[1024];
		_vsnprintf(msg, 1023, lpszFormat, (char*) (&lpszFormat + 1));
		msg[1023]=0;
		OUT_STRING(msg);
	}

	BOOL DumpMemory(LPVOID lpVoid, int nSize, BOOL timeStamp)
	{
	//	try
	//	{
			if (!CHECK_PTR(lpVoid) || nSize <=0)
				return FALSE;

			SYSTEMTIME date;
			GetLocalTime(&date) ;
			char tempFile[512];
			ZeroMemory(tempFile,sizeof(tempFile));
			MGStrOp::sprintf(tempFile,sizeof(tempFile) -1,"%s%02d_%02d_%02d.dump",DmpPath,date.wYear, date.wMonth, date.wDay);
			/*char szFile[256];
			_snprintf(szFile, 255, "%02d%02d%02d.dump", time.wYear, time.wMonth, time.wDay);
			szFile[255]=0;*/
			tempFile[511] = 0;
			FILE* fp = fopen(tempFile, "a+");
			if (fp)
			{
				if (timeStamp)
				{
					char szTime[256]={0};
					_snprintf(szTime, 255, "TIME%02d/%02d/%02d %02d:%02d:%02d", date.wYear, date.wMonth, date.wDay,date.wHour, date.wMinute, date.wSecond);
					szTime[255]=0;
					fwrite(szTime, 1, strlen(szTime), fp);
				}
				fwrite(lpVoid, 1, nSize, fp);
				fclose(fp);
			}
			return TRUE;
	//	}
	//	catch (...)
	//	{
	//		return FALSE;
	//	}
	}

	BOOL CheckHeap(LPVOID lpVoid, int nSize)
	{
	//	try
	//	{
			if (lpVoid == NULL)
				return FALSE;
			if (!CHECK_PTR(lpVoid))
				return FALSE;
			
			//检查头和尾
			DWORD head = *((DWORD*)lpVoid -1);
			DWORD tail = *(DWORD*)((char*)lpVoid + nSize);
			if ((tail != 0xFDFDFDFD) || (head != 0xFDFDFDFD))
				return FALSE;

			int status = _heapchk();
			if ((status == _HEAPBADNODE) || (status == _HEAPBADBEGIN))
				return FALSE;
			else
				return TRUE;
	//	}
	//	catch(...)
	//	{
	//		return FALSE;
	//	}
	}

	/**************************************************************************/
	/*
	 *	dump 调用堆栈
	 */
	/**************************************************************************/

	HANDLE g_hProcess = 0;
	#define MAX_COUNT 50

	//tag的定义
	enum BasicType  // Stolen from CVCONST.H in the DIA 2.0 SDK
	{
		btNoType = 0,
		btVoid = 1,
		btChar = 2,
		btWChar = 3,
		btInt = 6,
		btUInt = 7,
		btFloat = 8,
		btBCD = 9,
		btBool = 10,
		btLong = 13,
		btULong = 14,
		btCurrency = 25,
		btDate = 26,
		btVariant = 27,
		btComplex = 28,
		btBit = 29,
		btBSTR = 30,
		btHresult = 31
	};

	static	LPVOID __stdcall FunctionTableAccess(HANDLE hProcess, DWORD dwPCAddress);
	static	DWORD __stdcall GetModuleBase(HANDLE hProcess, DWORD dwReturnAddress);
	static	BOOL DumpSymbol(HANDLE hProcess, STACKFRAME* frame);
	static  BOOL GetLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset );

	char *	FormatOutputValue( char * pszCurrBuffer, size_t cbCurrBuffer, BasicType basicType, DWORD64 length, PVOID pAddress);
	char *	DumpTypeIndex( char * pszCurrBuffer, size_t cbBuffer, DWORD64 modBase, DWORD dwTypeIndex, unsigned nestingLevel, ULONG64 offset, bool & bHandled);
	bool	FormatSymbolValue( PSYMBOL_INFO pSym, STACKFRAME * sf, char * pszBuffer, unsigned cbBuffer );
	BasicType GetBasicType( DWORD typeIndex, DWORD64 modBase );
	BOOL	CALLBACK EnumerateSymbolsProc( PSYMBOL_INFO pSymInfo,  ULONG SymbolSize, PVOID UserContext);
	int	__cdecl PrintFile(const char * format, ...);


	void FmtCatSafe(char* buffer, int len, char* format, ...) 
	{
		if (buffer==NULL)
			return;
		if (len<=0)
			return;
		if (format==NULL)
			return;
		char msg[2048];
		va_list args;
		va_start( args, format );
		_vsnprintf( msg, 2047, format, args );
		msg[2047] = 0;
		if ((int)strlen(buffer) + (int)strlen(msg) < len)
			strcat(buffer, msg);
	}

	static void DumpExceptionCode(DWORD code)
	{
	#define EXCEPTION( x ) case x: pstr = #x; break;

		char* pstr = "";
		switch(code) 
		{
			EXCEPTION(EXCEPTION_ACCESS_VIOLATION);
			EXCEPTION(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
			EXCEPTION(EXCEPTION_BREAKPOINT);
			EXCEPTION(EXCEPTION_DATATYPE_MISALIGNMENT);
			EXCEPTION(EXCEPTION_FLT_DENORMAL_OPERAND);
			EXCEPTION(EXCEPTION_FLT_DIVIDE_BY_ZERO);
			EXCEPTION(EXCEPTION_FLT_INEXACT_RESULT);
			EXCEPTION(EXCEPTION_FLT_INVALID_OPERATION);
			EXCEPTION(EXCEPTION_FLT_OVERFLOW);
			EXCEPTION(EXCEPTION_FLT_STACK_CHECK);
			EXCEPTION(EXCEPTION_FLT_UNDERFLOW);
			EXCEPTION(EXCEPTION_ILLEGAL_INSTRUCTION);
			EXCEPTION(EXCEPTION_IN_PAGE_ERROR);
			EXCEPTION(EXCEPTION_INT_DIVIDE_BY_ZERO);
			EXCEPTION(EXCEPTION_INT_OVERFLOW)
			EXCEPTION(EXCEPTION_INVALID_DISPOSITION)
			EXCEPTION(EXCEPTION_NONCONTINUABLE_EXCEPTION)
			EXCEPTION(EXCEPTION_PRIV_INSTRUCTION)
			EXCEPTION(EXCEPTION_SINGLE_STEP)
			EXCEPTION(EXCEPTION_STACK_OVERFLOW)
		}

		char szMsg[256];
		_snprintf(szMsg, 255, "异常类型:%s\n", pstr);
		szMsg[255]=0;
		DUMP_STRING(szMsg);
	}


	static DWORD __stdcall GetModuleBase(HANDLE hProcess, DWORD dwReturnAddress)
	{
		IMAGEHLP_MODULE moduleInfo;

		if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
			return moduleInfo.BaseOfImage;
		else
		{
			MEMORY_BASIC_INFORMATION memoryBasicInfo;

			if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress,
				&memoryBasicInfo, sizeof(memoryBasicInfo)))
			{
				DWORD cch = 0;
				char szFile[MAX_PATH] = { 0 };

				cch = GetModuleFileNameA((HINSTANCE)memoryBasicInfo.AllocationBase,
											 szFile, MAX_PATH);
				SymLoadModule(hProcess,
					   NULL, ((cch) ? szFile : NULL),
					   NULL, (DWORD) memoryBasicInfo.AllocationBase, 0);
				 return (DWORD) memoryBasicInfo.AllocationBase;
			}
		}

		return 0;
	}

	//获得调用堆栈
	static int  GetCallStack(HANDLE	hProcess, LPEXCEPTION_POINTERS pException, STACKFRAME* frame, int maxCount)
	{
		PEXCEPTION_RECORD pRecord = pException->ExceptionRecord;
		PCONTEXT pContext	= pException->ContextRecord;

		STACKFRAME sf;
		memset(&sf,0,sizeof(sf));

		// 初始化stackframe结构
		sf.AddrPC.Offset = pContext->Eip;
		sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrStack.Offset = pContext->Esp;
		sf.AddrStack.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = pContext->Ebp;
		sf.AddrFrame.Mode = AddrModeFlat;
		DWORD	dwMachineType = IMAGE_FILE_MACHINE_I386;

		int count = 0;
		for(int i = 0; i < maxCount; i++)
		{
			// 获取下一个栈帧
			if(!StackWalk(dwMachineType, hProcess, GetCurrentThread(),
				&sf, pContext, NULL,
				SymFunctionTableAccess,GetModuleBase,NULL))
				break;

			// 检查帧的正确性
			if(0 == sf.AddrFrame.Offset)
				break;

			memcpy(&frame[count], &sf, sizeof(STACKFRAME));
			count++;
		}

		return count;
	}

	int DumpCallStack(LPEXCEPTION_POINTERS pException, DWORD code)
	{
		char* pStr = "DUMP_CALL_STACK\n";
	//	try
		{
		EnterCriticalSection(&g_dbCriticalSection);
		PEXCEPTION_RECORD pRecord = pException->ExceptionRecord;
		PCONTEXT pContext	= pException->ContextRecord;
		
		DUMP_MEMORY_TIME(pStr, strlen(pStr));
		DumpExceptionCode(pRecord->ExceptionCode);

		g_hProcess = GetCurrentProcess();

		STACKFRAME* pFrame = new STACKFRAME[MAX_COUNT];

	//	STACKFRAME * ppppp = NULL;
	//	ppppp->Virtual = 0;

		int count = GetCallStack(g_hProcess, pException, pFrame, MAX_COUNT);

		SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME| SYMOPT_DEFERRED_LOADS);
		if (SymInitialize(g_hProcess, NULL, TRUE))
		{	
			for(int i = 0; i < count; i++)
			{
				if (DumpSymbol(g_hProcess, &pFrame[i]))
				{
				}
			}
			SymCleanup(g_hProcess);
		}
		else
		{
			for(int i = 0; i < count; i++)
			{
				char szModule[MAX_PATH] = "";
				char szMsg[1024];
				DWORD section = 0, offset = 0;
				STACKFRAME* frame = &pFrame[i];
		
				GetLogicalAddress(  (LPVOID)frame->AddrPC.Offset,
									szModule, sizeof(szModule), section, offset );

				_snprintf(szMsg, 1023, "%08lX %s offset:%x \n", frame->AddrPC.Offset, szModule, offset);
				szMsg[1023]=0;
				DUMP_STRING(szMsg);
			}
		}
		pStr = "---------DumpCallStack----------\n";
		DUMP_STRING(pStr);
		delete []pFrame;
		LeaveCriticalSection(&g_dbCriticalSection);
		}
		/*
		catch (...)
		{
			LeaveCriticalSection(&g_dbCriticalSection);
			pStr = "DumpCallStack except";
			return EXCEPTION_EXECUTE_HANDLER ;
		}
		*/
		return EXCEPTION_EXECUTE_HANDLER ;
	}

	char g_szClassName[256];
	static BOOL DumpSymbol(HANDLE hProcess, STACKFRAME* frame)
	{
		// 正在调用的函数名字
		BYTE symbolBuffer[sizeof(SYMBOL_INFO) + 1024];
		SYMBOL_INFO * pSymbol = (SYMBOL_INFO *)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO)+1024;
		pSymbol->MaxNameLen = 1024;

		char szMsg[1024];

		CHAR szUndec[256];
		LPSTR pszSymbol = NULL;

		DWORD64 symDisplacement = 0;
		if(SymFromAddr(hProcess, frame->AddrPC.Offset, &symDisplacement,pSymbol))
		{
			pszSymbol = pSymbol->Name;
			//分析函数名
			if (UnDecorateSymbolName(pSymbol->Name, szUndec, _countof(szUndec),
				UNDNAME_COMPLETE | UNDNAME_NO_ACCESS_SPECIFIERS))
			{
				pszSymbol = szUndec;
			}

			_snprintf(szMsg, 1023, "%s() %I64d  ",  pszSymbol, symDisplacement);
			szMsg[1023]=0;
			DUMP_MEMORY(szMsg, strlen(szMsg));

			DWORD symDis;
			IMAGEHLP_LINE line;
			line.Address = sizeof(IMAGEHLP_LINE);
			if (SymGetLineFromAddr(hProcess, frame->AddrPC.Offset, &symDis, &line))
			{
				_snprintf(szMsg, 1023, "%s 第 %d 行\n",  line.FileName, line.LineNumber);
				szMsg[1023]=0;
				DUMP_MEMORY(szMsg, strlen(szMsg));
			}
			else
			{
	//			sprintf(szMsg, "\n",  line.FileName, line.LineNumber);
				DUMP_MEMORY(szMsg, strlen(szMsg));
			}	
			//end 分析函数名
			
			//分析参数
			IMAGEHLP_STACK_FRAME StackFrame;
			memset(&StackFrame, 0, sizeof(StackFrame));
			StackFrame.InstructionOffset = frame->AddrPC.Offset;
			SymSetContext(hProcess,&StackFrame, NULL );

			//get class name
			g_szClassName[0] = 0;
			char* p = strstr(pszSymbol, "::");
			if (p != NULL)
			{
				*p = 0;
				STRNCPY(g_szClassName, pszSymbol, sizeof(g_szClassName));
			}
			
			if (SymEnumSymbols(hProcess, NULL, NULL, EnumerateSymbolsProc, frame))
			{
				//
			}
		}
		else
		{
			//输出没有符号的地址
			char szModule[MAX_PATH] = "";
			DWORD section = 0, offset = 0;

			GetLogicalAddress(  (PVOID)frame->AddrPC.Offset,
								szModule, sizeof(szModule), section, offset );

			_snprintf(szMsg, 1023, "%08lX %s offset:%x \n", frame->AddrPC.Offset, szModule, offset);
			szMsg[1023]=0;
			DUMP_STRING(szMsg);
		}


		return TRUE;
	}

	//=============================================================================================
	/*	
	 *   枚举符号
	 */
	//=============================================================================================
	BOOL CALLBACK EnumerateSymbolsProc(	PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext )
	{
		char szBuffer [ 2048 ];
		MEMZERO ( szBuffer );

		if ( UserContext )
		{
			__try
			{
				if ( FormatSymbolValue( pSymInfo, ( STACKFRAME * ) UserContext,
					szBuffer, sizeof ( szBuffer ) ) )
				{
					PrintFile ( "\t%s\n", szBuffer );
				}
			}
			__except( 1 )
			{
				DWORD	dwAddress = ( DWORD ) pSymInfo->Address;
				PrintFile(	"Symbol= %08lX [%4x] %s\n",
					dwAddress, SymbolSize, pSymInfo->Name );
			}
		}
		else
		{
			if ( pSymInfo )
			{
				DWORD	dwAddress = ( DWORD ) pSymInfo->Address;
				PrintFile(	"Symbol: %08lX [%4x] %s\n",
					dwAddress, SymbolSize, pSymInfo->Name );
			}
		}

		return TRUE;
	}



	bool FormatSymbolValue(	PSYMBOL_INFO pSym, STACKFRAME * sf, char * pszBuffer, unsigned cbBuffer )
	{
		if (!pSym)
			return false;

		// Indicate if the variable is a local or parameter
		if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER )
		{
			FmtCatSafe ( pszBuffer, cbBuffer, "Parameter " );
		}
		else if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL )
		{
			FmtCatSafe ( pszBuffer, cbBuffer, "Local     " );
		}

		//符号类型，函数，指针等
		if ( pSym->Tag == SymTagFunction )   //enum SymTagEnum 
		{
			return false;
		}

		// Emit the variable name
		if ( pSym )
		{
			FmtCatSafe ( pszBuffer, cbBuffer, "(%08X ", pSym->Address );
			FmtCatSafe ( pszBuffer, cbBuffer, " [%4X] ", pSym->NameLen );
		//	if ( pSym->NameLen > 0 )
			{
				FmtCatSafe ( pszBuffer, cbBuffer, "SymName=%s)", pSym->Name );
			}
		//	else
		//	{
		//		FmtCatSafe ( pszBuffer, cbBuffer, "SymName=<EMPTY>)" );
		//	}
		}
		else
		{
			FmtCatSafe ( pszBuffer, cbBuffer, "(SymName=UnKnown)" );
		}

		ULONG64 pVariable = 0;    // Will point to the variable's data in memory

		if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE )
		{
			// if ( pSym->Register == 8 )   // EBP is the value 8 (in DBGHELP 5.1)
			{                               //  This may change!!!
				pVariable = sf->AddrFrame.Offset;
				pVariable += (ULONG64)pSym->Address;
			}
			// else
			//  return false;
		}
		else if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER )
		{
			return false;   // Don't try to report register variable
		}
		else
		{
			pVariable = (ULONG64)pSym->Address;   // It must be a global variable
		}

		//检查是否一个结构
		bool bHandled;
		DumpTypeIndex (	pszBuffer, cbBuffer,
			pSym->ModBase, pSym->TypeIndex,
			0, pVariable, bHandled );

		if ( ! bHandled )
		{
			BasicType basicType = GetBasicType( pSym->TypeIndex, pSym->ModBase );

			FormatOutputValue ( pszBuffer, cbBuffer, basicType, pSym->Size, (PVOID)pVariable ); 
		}

		//DUMP this pointer
		if (strncmp(pSym->Name, "this", 4) == 0)
		{
			bool bHandledThis = 0;
			char symbolBuf[sizeof(SYMBOL_INFO)+1024];
			SYMBOL_INFO *info = (SYMBOL_INFO*)symbolBuf;
			info->SizeOfStruct = sizeof(SYMBOL_INFO)+1024;
			info->MaxNameLen = 1024;
			
			void* pThis = (void*) *(DWORD*)pVariable;
			if ((g_szClassName[0] != 0) && CHECK_PTR(pThis))
			{	
				FmtCatSafe(pszBuffer, cbBuffer, "\n\t");
				SymGetTypeFromName(g_hProcess, pSym->ModBase, g_szClassName, info);
				{			
					DumpTypeIndex (	pszBuffer,  cbBuffer,
						info->ModBase, info->TypeIndex,
						0, *(DWORD*)pVariable, bHandledThis );
				}
			}
		}

		return true;
	}

	//=============================================================================
	//
	// DUMP 结构
	//
	//=============================================================================
	char * DumpTypeIndex(
		char * pszCurrBuffer,
		size_t	cbBuffer,
		DWORD64 modBase,
		DWORD dwTypeIndex,
		unsigned nestingLevel,
		ULONG64 offset,
		bool & bHandled )
	{
		bHandled = false;

		//检查符号是否函数, dump this指针时可能会碰到函数
		DWORD tag;
		if (SymGetTypeInfo( g_hProcess, modBase, dwTypeIndex, TI_GET_SYMTAG, &tag))
		{
			if (tag == SymTagFunction)
			{
				bHandled = TRUE;
				return pszCurrBuffer;
			}
		}
		
		// 结构名
		WCHAR * pwszTypeName;
		if (SymGetTypeInfo( g_hProcess, modBase, dwTypeIndex, TI_GET_SYMNAME, &pwszTypeName ) )
		{
			FmtCatSafe ( pszCurrBuffer, cbBuffer, " %ls", pwszTypeName );
			LocalFree( pwszTypeName );
		}
		
		// 获得children count
		DWORD dwChildrenCount = 0;
		SymGetTypeInfo( g_hProcess, modBase, dwTypeIndex, TI_GET_CHILDRENCOUNT,	&dwChildrenCount );
		if ( !dwChildrenCount )
			return pszCurrBuffer;

		// 定义一个获取children的结构，中间的函数时构造函数, TI_FINDCHILDREN要求的结构
		struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
		{
			ULONG   MoreChildIds[1024];
			FINDCHILDREN()
			{
				Count = sizeof(MoreChildIds) / sizeof(MoreChildIds[0]);
			}
		} children;

		children.Count = dwChildrenCount;
		children.Start= 0;

		if ( ! SymGetTypeInfo ( g_hProcess, modBase, dwTypeIndex, TI_FINDCHILDREN, &children ) )
		{
			return pszCurrBuffer;
		}

		FmtCatSafe ( pszCurrBuffer, cbBuffer, "\n" );

		for ( unsigned i = 0; i < dwChildrenCount; i++ )
		{
			for ( unsigned j = 0; j <= nestingLevel+1; j++ )
				FmtCatSafe ( pszCurrBuffer, cbBuffer, "\t" );

			bool bHandled2;
			DumpTypeIndex(	pszCurrBuffer, cbBuffer,
				modBase, children.ChildId[i], nestingLevel+1,
				offset, bHandled2 );

			if ( !bHandled2 )
			{
				DWORD dwMemberOffset;
				SymGetTypeInfo( g_hProcess, modBase, children.ChildId[i],
					TI_GET_OFFSET, &dwMemberOffset );

				DWORD typeId;
				SymGetTypeInfo( g_hProcess, modBase, children.ChildId[i],
					TI_GET_TYPEID, &typeId );

				ULONG64 length;
				SymGetTypeInfo(g_hProcess, modBase, typeId, TI_GET_LENGTH,&length);

				ULONG64 dwFinalOffset = offset + dwMemberOffset;

				BasicType basicType = GetBasicType(children.ChildId[i], modBase );

				FormatOutputValue( pszCurrBuffer, cbBuffer, basicType,
					length, (PVOID)dwFinalOffset ); 

				FmtCatSafe ( pszCurrBuffer, cbBuffer, "\n" );
			}
		}

		bHandled = true;
		return pszCurrBuffer;
	}

	char * FormatOutputValue(char * pszCurrBuffer, size_t cbCurrBuffer, BasicType basicType, DWORD64 length, PVOID pAddress)
	{
		if ( length == 1 )
		{
			FmtCatSafe ( pszCurrBuffer, cbCurrBuffer, " = <%X>", *(PBYTE)pAddress );
		}
		else if ( length == 2 )
		{
			FmtCatSafe ( pszCurrBuffer, cbCurrBuffer, " = <%X>", *(PWORD)pAddress );
		}
		else if ( length == 4 )
		{
			if ( basicType == btFloat )
			{
				FmtCatSafe ( pszCurrBuffer, cbCurrBuffer, " = <%f>", *(PFLOAT)pAddress);
			}
			else if ( basicType == btChar )
			{
				if ( ! IsBadStringPtr( *(PSTR*)pAddress, 32) )
				{
					FmtCatSafe (	pszCurrBuffer, cbCurrBuffer, 
						" = \"%.31s\"", *(PDWORD)pAddress );
				}
				else
				{
					FmtCatSafe (	pszCurrBuffer, cbCurrBuffer, 
						" = <%X>", *(PDWORD)pAddress );
				}
			}
			else
			{
				FmtCatSafe (	pszCurrBuffer, cbCurrBuffer, 
					" = <%X>", *(PDWORD)pAddress);
			}
		}
		else if ( length == 8 )
		{
			if ( basicType == btFloat )
			{
				FmtCatSafe (	pszCurrBuffer, cbCurrBuffer, 
					" = <%lf>", *(double *)pAddress );
			}
			else
			{
				FmtCatSafe (	pszCurrBuffer, cbCurrBuffer, 
					" = <%I64X>", *(DWORD64*)pAddress );
			}
		}

		return pszCurrBuffer;
	}

	BasicType GetBasicType( DWORD typeIndex, DWORD64 modBase )
	{
		BasicType basicType;

		if ( SymGetTypeInfo( g_hProcess, modBase, typeIndex,
			TI_GET_BASETYPE, &basicType ) )
		{
			return basicType;
		}

		// Get the real "TypeId" of the child.  We need this for the
		// SymGetTypeInfo( TI_GET_TYPEID ) call below.
		DWORD typeId;
		if ( SymGetTypeInfo (g_hProcess,modBase, typeIndex, TI_GET_TYPEID, &typeId))
		{
			if ( SymGetTypeInfo( g_hProcess, modBase, typeId, TI_GET_BASETYPE,
				&basicType ) )
			{
				return basicType;
			}
		}

		return btNoType;
	}

	int __cdecl PrintFile(const char * format, ...)
	{
		int	retValue;
		char msg[1024];

		_vsnprintf(msg, 1023, format, (char*) (&format + 1));
		msg[1023]=0;
		retValue = strlen(msg);
		DUMP_MEMORY(msg, retValue);

		return retValue;
	}

	BOOL GetLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset )
	{
		MEMORY_BASIC_INFORMATION mbi;

		if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
			return FALSE;

		DWORD hMod = (DWORD)mbi.AllocationBase;
		if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
			return FALSE;

		PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
		PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
		PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

		DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

		// Iterate through the section table, looking for the one that encompasses
		// the linear address.
		for ( unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ )
		{
			DWORD sectionStart = pSection->VirtualAddress;
			DWORD sectionEnd = sectionStart
						+ max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

			if ( (rva >= sectionStart) && (rva <= sectionEnd) )
			{
				section = i+1;
				offset = rva - sectionStart;
				return TRUE;
			}
		}

		return FALSE; 
	}

	//===================================================================
	/*
	 *	安装自己的debug handle, 处理没有被捕获的异常
	 */
	//===================================================================
	void InstallMyExceptHandle(const char* path)
	{
		//DmpPath
		StrCpy(DmpPath,path);
		InitializeCriticalSection(&g_dbCriticalSection);
		offsetOfDebug = (DWORD)&InstallMyExceptHandle;
		if (!IsDebuggerPresent())
		{
			SetUnhandledExceptionFilter(MyExceptHandle);
		}
	}
	void	UnInstallMyExceptHandle()
	{
		DeleteCriticalSection(&g_dbCriticalSection);
	}

	//===================================================================
	/*
	 *	没有被处理的异常将调用MyExceptHandle
	 */
	//===================================================================
	LONG WINAPI MyExceptHandle(struct _EXCEPTION_POINTERS *ExceptionInfo)
	{
		DUMP_STRING_TIME("ENTER MY GLOBAL EXCEPT HANDLER\n");
		if (ExceptionInfo)
		{
			char a[1024];
			_snprintf(a, 1023, "Offset of debug = %x\n", offsetOfDebug);
			a[1023]=0;
			DUMP_STRING(a);
			DumpCallStack(ExceptionInfo, 0);
		}

		DUMP_STRING("LEAVE MY GLOBAL EXCEPT HANDLER\n");
		CLOSE_PROCESS();//直接关闭进程

		return EXCEPTION_EXECUTE_HANDLER;
	}

	BOOLEAN Sprintf_s(char * dest, size_t size, char* format, ...)
	{
		if(dest && format && size > 0)
		{
			va_list args;
			va_start( args, format );
			_vsnprintf( dest, size - 1, format, args );
			dest[size - 1] = 0;

			return TRUE;
		}
	#ifdef _DEBUG
		OutputDebugString("ERROR:  Sprintf_s 's inputs has NULL or <=0\n");
		throw;
	#endif
		return FALSE;
	}

	void MessageOut(const char* msg, bool bPrint)
	{
		if (!msg)
			return;

#ifdef _DEBUG
		if (!bPrint)
			::MessageBoxA(NULL, msg, "", MB_OK);
#else
		bPrint = true;
#endif
		if (bPrint)
		{
			FILE* stream = NULL;
			if (stream = fopen("log.log", "a"))
			{
				std::string s;
				char buf[MAX_PATH];
				SYSTEMTIME t;
				GetLocalTime(&t);
				MGStrOp::sprintf(buf, MAX_PATH, "[%.4d-%.2d-%.2d %.2d:%.2d:%.2d]  ", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
				s += buf;
				StrCpy(buf, msg);
				s += buf;
				s += "\n";
				fprintf(stream, s.c_str());
				fclose(stream);
			}
		}
	}

	void MessageOut(const wchar_t* msg, bool bPrint)
	{
		if (!msg)
			return;

#ifdef _DEBUG
		if (!bPrint)
			::MessageBoxW(NULL, msg, L"", MB_OK);
#else
		bPrint = true;
#endif
		if (bPrint)
		{
			FILE* stream = NULL;
			if (stream = fopen("log.log", "a"))
			{
				std::wstring s;
				wchar_t buf[MAX_PATH];
				SYSTEMTIME t;
				GetLocalTime(&t);
				MGStrOp::sprintf(buf, MAX_PATH, L"[%.4d-%.2d-%.2d %.2d:%.2d:%.2d]  ", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
				s += buf;
				WStrnCpyS(buf,MAX_PATH,msg,MAX_PATH);
				s += buf;
				s += L"\n";
				std::string dest;
				MGStrOp::toString(s.c_str(),dest);
				fprintf(stream, dest.c_str());
				fclose(stream);
			}
		}
	}
}
