/******************************************************************************/
#include "stdafx.h"
#include "MGKTimer.h"
/******************************************************************************/
namespace MG
{
	UInt MGKTimer::mCpuMHz = 0;
	//-----------------------------------------------------------------------------
	MGKTimer::MGKTimer( void )
	{
		mOverhead = 0;
		start();
		mOverhead = stop();
		mCpuMHz = cpuSpeedMHz();
	}

	//-----------------------------------------------------------------------------
	void MGKTimer::start( void )
	{
		mStartcycle = GetCycleCount();
	}

	//-----------------------------------------------------------------------------
	MG::U64 MGKTimer::stop( void )
	{
		return GetCycleCount() - mStartcycle - mOverhead;
	}

	//-----------------------------------------------------------------------------
	UInt MGKTimer::cpuSpeedMHz()
	{
		U64 startCycle = GetCycleCount();
		Sleep(100);
		U64 cputime = GetCycleCount() - startCycle;
		return UInt(cputime/100000);
	}
	//-----------------------------------------------------------------------------
	U64 MGKTimer::cyclesToMillis(U64 timeCycles,UInt speedMhz)
	{
		return  timeCycles / speedMhz / 1000;
	}
	//-----------------------------------------------------------------------------
	MG::U64 MGKTimer::cyclesToNanos( U64 timeCycles, UInt speedMhz )
	{
		return timeCycles * 1000 / speedMhz;
	}

	MG::UInt MGKTimer::getCpuMHz()
	{
		return mCpuMHz;
	}

	//-----------------------------------------------------------------------------

}
