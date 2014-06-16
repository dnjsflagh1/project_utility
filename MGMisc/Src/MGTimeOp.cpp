/******************************************************************************/
#include "stdafx.h"
#include "MGTimeOp.h"
#include <STDIO.H>
#include <Mmsystem.h>
#include <WINDOWS.H>
#include <TIME.H>
#include <sys/TIMEB.H>
#pragma comment(lib,"Winmm.lib")

/******************************************************************************/


namespace MG
{
	//-----------------------------------------------------------------------------
	U64 g_jchtimegetby_timeatsecond_offset = 0;
	U32	g_jchtimegetby_tick_offset = 0;
	U32	g_jchtimegetby_tick_start = GetTickCount();
	//-----------------------------------------------------------------------------
	void MGTimeOp::syncTime(U64 xStdTime)
	{
		U64 xCurrent = time(NULL);
		g_jchtimegetby_timeatsecond_offset = xStdTime - xCurrent;
	}
	//-----------------------------------------------------------------------------
	U64 MGTimeOp::getCurrTimestamp()
	{
		U64 xCurrent = time(NULL);
		return xCurrent + g_jchtimegetby_timeatsecond_offset;
	}
	//-----------------------------------------------------------------------------
	Bool MGTimeOp::inSameHour(U64 xTime)
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr,xTest;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		gmtime_s(&xTest,(time_t*)&xTime);
		if(xCurr.tm_year == xTest.tm_year && xCurr.tm_yday == xTest.tm_yday && xCurr.tm_hour == xTest.tm_hour)
			return true;
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool MGTimeOp::inSameDay(U64 xTime)
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr,xTest;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		gmtime_s(&xTest,(time_t*)&xTime);
		if(xCurr.tm_year == xTest.tm_year && xCurr.tm_yday == xTest.tm_yday)
			return true;
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool MGTimeOp::inSameMonth(U64 xTime)
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr,xTest;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		gmtime_s(&xTest,(time_t*)&xTime);
		if(xCurr.tm_year == xTest.tm_year && xCurr.tm_mon == xTest.tm_mon)
			return true;
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool MGTimeOp::inSameWeek(U64 xTime)
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr,xTest;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		gmtime_s(&xTest,(time_t*)&xTime);
		if(xCurr.tm_year != xTest.tm_year || xCurr.tm_mon != xTest.tm_mon || abs(xCurr.tm_wday - xTest.tm_wday) >= 7)
			return false;
		I32 nWeakC = xCurr.tm_wday;
		if(nWeakC == 0) nWeakC = 7;
		I32 nWeekT = xTest.tm_wday;
		if(nWeekT == 0) nWeekT = 7;

		if((xCurrent < xTime && nWeakC > nWeekT) || (xCurrent > xTime && nWeakC < nWeekT))
		{
			return false;	
		}
		return true;
	}
	//-----------------------------------------------------------------------------
	Bool MGTimeOp::timePass(U64 xCheckTime)
	{
		U64 xCurrent = getCurrTimestamp();
		if(xCurrent >= xCheckTime)
			return true;
		return false;
	}
	//-----------------------------------------------------------------------------
	U64 MGTimeOp::getFutureTime(U64 xStart,U32 dwOffset)
	{
		return xStart + dwOffset;
	}
	//-----------------------------------------------------------------------------
	U64 MGTimeOp::getFutureTime(I32 nDays)
	{
		U64 xCurrent = getCurrTimestamp();
		return xCurrent + nDays*24*60*60;
	}
	//-----------------------------------------------------------------------------
	U64 MGTimeOp::getFutureTime(I32 Hours,I32 Minutes,I32 Seconds)
	{
		U64 xCurrent = getCurrTimestamp();
		return xCurrent + Hours*60*60 + Minutes*60 + Seconds;
	}
	//-----------------------------------------------------------------------------
	U64 MGTimeOp::getTimeInToday(U32 dwOffset)
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		xCurr.tm_hour = 0;
		xCurr.tm_min = 0;
		xCurr.tm_sec = 0;
		return mktime(&xCurr) + dwOffset;
	}
	//-----------------------------------------------------------------------------
	U64 MGTimeOp::getTimeInToday(I32 Hour,I32 Minute,I32 Second)
	{
		return getTimeInToday(Hour*60*60 + Minute*60 + Second);
	}
	//-----------------------------------------------------------------------------
	U64 MGTimeOp::createtimestamp(U32 wYear,U8 bMonth,U8 bDay,U8 bHour /* = 0 */,U8 bMinute /* = 0 */,U8 bSecond /* = 0 */)
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		xCurr.tm_hour = bHour;
		xCurr.tm_mday = bDay;
		xCurr.tm_min = bMinute;
		xCurr.tm_mon = bMonth - 1;
		xCurr.tm_sec = bSecond;
		xCurr.tm_wday = 0;
		xCurr.tm_yday = 0;
		xCurr.tm_year = wYear - 1900;
		return _mkgmtime(&xCurr);
	}
	//-----------------------------------------------------------------------------
	U8 MGTimeOp::getWeekDay()
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		I32 nWeakC = xCurr.tm_wday;
		if(nWeakC == 0) nWeakC = 7;
		return U8(nWeakC);
	}
	//-----------------------------------------------------------------------------
	U8 MGTimeOp::getMonthDay()
	{
		U64 xCurrent = getCurrTimestamp();
		tm xCurr;
		gmtime_s(&xCurr,(time_t*)&xCurrent);
		return U8(xCurr.tm_mday);
	}
	//-----------------------------------------------------------------------------
	I32 MGTimeOp::getDaysBeforeDeadline(U64 xDeadline)
	{
		U64 xCurrent = getCurrTimestamp();
		if(xCurrent >= xDeadline) return 0;
		return I32((xDeadline - xCurrent)/(24*60*60));
	}
	//-----------------------------------------------------------------------------
	I32 MGTimeOp::getHoursBeforeDeadline(U64 xDeadline)
	{
		U64 xCurrent = getCurrTimestamp();
		if(xCurrent >= xDeadline) return 0;
		return I32((xDeadline - xCurrent)/(60*60));
	}
	//-----------------------------------------------------------------------------
	U32 MGTimeOp::getCurrTick()
	{
		return GetTickCount() - g_jchtimegetby_tick_start - g_jchtimegetby_tick_offset;
	}
	//-----------------------------------------------------------------------------
	void MGTimeOp::setSvrTick(U32 dwTick)
	{
		g_jchtimegetby_tick_offset = GetTickCount() - g_jchtimegetby_tick_start - dwTick;
	}
	//-----------------------------------------------------------------------------
	SYSTEMTIME MGTimeOp::getLocalTime(U64 xTime)
	{
		SYSTEMTIME xLocalTime;
		tm tmTime;
		memset(&xLocalTime,0,sizeof(xLocalTime));
		memset(&tmTime,0,sizeof(tmTime));

		errno_t err = localtime_s(&tmTime, (time_t*)&xTime);
		if(0 == err)
		{
			xLocalTime.wYear	= (WORD)tmTime.tm_year + 1900;
			xLocalTime.wMonth	= (WORD)tmTime.tm_mon + 1;
			xLocalTime.wDay		= (WORD)tmTime.tm_mday;
            xLocalTime.wHour    = (WORD)tmTime.tm_hour;
            xLocalTime.wMinute  = (WORD)tmTime.tm_min;
            xLocalTime.wSecond  = (WORD)tmTime.tm_sec;
		}
		return xLocalTime;
	}
	//-----------------------------------------------------------------------------
}