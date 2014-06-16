#include "stdafx.h"
#include "LogConsole.h"
namespace MG
{
	HANDLE	hConsole = NULL;

	LogConsole::LogConsole()
	{
		openConsole();
	}

	LogConsole::~LogConsole()
	{
		closeConsole();
	}

	Bool LogConsole::openConsole( )
	{
		if ( hConsole )
			return false;

		AllocConsole( );

		hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

		if ( hConsole )
		{
			freopen( "CONOUT$", "w", stdout );
			freopen( "CONOUT$", "w", stderr );
			freopen( "CONIN$", "r", stdin );
			return true;
		}

		return false;
	}

	Bool LogConsole::isOpen() const
	{
		return ( hConsole != NULL );
	}

	void LogConsole::closeConsole()
	{
		if ( hConsole )
		{
			SetConsoleTextAttribute( hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );

			FreeConsole();
			hConsole = NULL;
		}
	}

	void LogConsole::logWrite( LOG_TYPE outType, CChar* logStr )
	{
		DWORD len = 0;

		WORD wColor;
		switch(outType)
		{

		case out_netrecv:
			wColor = FOREGROUND_BLUE;
			break;
		case out_netsend:
			wColor = FOREGROUND_BLUE;
			break;
		case out_debug:
			wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		case out_script_dbg:
			wColor = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		case out_sys:
			wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		case out_error:
			wColor = FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		case out_warning:
			wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			break;
		case out_info:
			wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
			break;

		default:
			wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
			break;

		}

		if ( hConsole )
		{
			SetConsoleTextAttribute( hConsole, wColor );
#ifdef MG_WCHAR_T_STRINGS
			WriteConsoleW( hConsole,(CPtr)logStr, (DWORD)lstrlenW(logStr), &len, 0 );
#else
			WriteConsoleA( hConsole,(CPtr)logStr, (DWORD)lstrlenA(logStr), &len, 0 );
#endif
			
		}
		else
		{
			
#ifdef MG_WCHAR_T_STRINGS
			static Char8 buf[8192];
			wcstombs( buf, logStr, sizeof(buf) );
			fputs( buf, stdout );
#else
			fputs( logStr, stdout );
#endif

		}

#ifdef _DEBUG
		//	OutputDebugString( msg );
#endif
	}

}
