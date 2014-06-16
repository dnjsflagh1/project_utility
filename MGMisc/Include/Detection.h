#ifndef _DETECTION_h_
#define _DETECTION_h_
#include "Lock.h"
#include "MGKTimer.h"
namespace MG
{
	/******************************************************************************/
	class Detection
	{
	public:
		Detection();
		virtual U64		getCurrDetection() = 0;
		virtual	U64		getMinDetection() = 0;
		virtual U64		getMaxDetection() = 0;
		virtual Flt		getAverDetection() = 0;
		virtual U32		getCallCount() = 0;
		virtual Flt		getCallRate() = 0;
		virtual Char*	getName() = 0;
		void			setThreadAffinityMask();
		MGKTimer		mTimer;
	private:
		Bool			mIsSetThreadAffinityMask;
	};
	/******************************************************************************/

	class FunDetection : public Detection
	{
		friend class FunDetectionCalc;
		friend class DetectionMgr;
	public:

		~FunDetection();
		
		U64	getCurrDetection();
		U64	getMinDetection();
		U64 getMaxDetection();
		Flt	getAverDetection();
		U32	getCallCount();
		Flt getCallRate();
		Char* getName();

	private:
		FunDetection(CChar* name);

	private:
		U64 mCurrDetection;
		U64 mMaxDetection;
		U64 mMinDetection;
		std::vector<U64> mAllDetection;
		U32 mCallCount;
		Flt	mCallRate;
		Char mName[64];
		Critical cs;
	};

	/******************************************************************************/

	class FunDetectionCalc
	{
	public:
		FunDetectionCalc(Detection* detection);
		~FunDetectionCalc();
	private: 
		FunDetection* mDetection;
		U32 mStratTime;
		U32 mEndTime;
	};
	/******************************************************************************/
}

#endif 
