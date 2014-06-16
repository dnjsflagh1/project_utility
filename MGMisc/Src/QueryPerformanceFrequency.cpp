/******************************************************************************/
#include "stdafx.h"
#include "QueryPerformanceFrequency.h"
/******************************************************************************/
namespace MG
{
	//-----------------------------------------------------------------------------
	QueryPerformanceFrequency::QueryPerformanceFrequency()
	{
		mIsSupport = false;
		mIsStart = false;
	}
	//-----------------------------------------------------------------------------
	QueryPerformanceFrequency::~QueryPerformanceFrequency()
	{
		
	}
	//-----------------------------------------------------------------------------
	MG::Bool QueryPerformanceFrequency::initialize(Bool offsetMode)
	{
		if (::QueryPerformanceFrequency(&mFreq))
		{
			mIsSupport = true;
			if (offsetMode)
			{
				start();
			}
		}
		else
		{
			mIsSupport = false;
		}

		return mIsSupport;
	}
	//-----------------------------------------------------------------------------
	void QueryPerformanceFrequency::start( void )
	{
		if (mIsStart)
		{
			return;
		}
		else
		{
			if(::QueryPerformanceCounter(&mStart))
			{
				mIsStart = true;
			}
			else
			{
				mIsStart = false;
			}
		}
		
	}
	//-----------------------------------------------------------------------------
	MG::Dbl QueryPerformanceFrequency::stop( void )
	{
		if (mIsSupport && mIsStart)
		{
			mIsStart = false;
			if (::QueryPerformanceCounter(&mEnd))
			{
				return(mEnd.QuadPart - mStart.QuadPart) / (Dbl)mFreq.QuadPart;
			}
			
		}
		mIsStart = false;
		U32 err = GetLastError();
		return 0.0f;
	}

	Dbl QueryPerformanceFrequency::getTimeOffset( )
	{
		Dbl timeOffset = stop();
		start();
		return timeOffset;
	}
	//-----------------------------------------------------------------------------
}

