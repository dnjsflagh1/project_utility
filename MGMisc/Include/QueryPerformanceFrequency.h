/********************************************************************************
*				@高精度计时器					*	
********************************************************************************/
#ifndef _QUERYPERFORMANCEFREQUENCY_H_
#define _QUERYPERFORMANCEFREQUENCY_H_
/******************************************************************************/
#include "Singleton.h"
/******************************************************************************/
namespace MG
{
	class QueryPerformanceFrequency
	{
	public:
		QueryPerformanceFrequency();
		~QueryPerformanceFrequency();

		Bool			initialize(Bool offsetMode = false);
		void			start(void);
		Dbl				stop(void);	

		Dbl				getTimeOffset();

	private:
		LARGE_INTEGER			mStart;
		LARGE_INTEGER			mEnd;
		LARGE_INTEGER			mFreq; 
		Bool					mIsSupport;
		Bool					mIsStart;
		Bool					mOffsetMode;
	};
}


#endif //_QUERYPERFORMANCEFREQUENCY_H_