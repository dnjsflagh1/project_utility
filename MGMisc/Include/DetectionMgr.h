#ifndef _DETECTIONMGR_H_
#define _DETECTIONMGR_H_

#include "Detection.h"
#include "Singleton.h"
namespace MG
{
	enum DetectionTimeMode
	{
		DetectionTimeMode_KTimer,
		DetectionTimeMode_TimeOp,
	};
	class DetectionMgr
	{
	public:
		~DetectionMgr();
		 SINGLETON_INSTANCE(DetectionMgr)

		Detection*	getFunDetection(CChar* funName);
		U64			getCurrDetection(CChar* funName);
		U64			getMaxDetection(CChar* funName);
		Flt			getAverDetection(CChar* funName);
		U32			getCallCount(CChar* funName);
		Flt			getCallRate(CChar* funName);

		void				setPrint(Bool isPrint);
		Bool				getPrint();
		void				setTimeMode( DetectionTimeMode mode);
		DetectionTimeMode	getTimeMode();

	public:
		typedef std::map<Str,Detection*>::iterator DetectionIt;
				
	private:
		Detection*					_find(CChar* name);
		DetectionMgr();
		std::map<Str,Detection*>	mDetections;
		Bool						mIsPrint;
		DetectionTimeMode			mTimerMode;

	};


#if 1

	#ifdef OPEN_DETECTION
		#define  FUNDETECTION( name ) MG::FunDetectionCalc x(MG::DetectionMgr::getInstance().getFunDetection(name));
	#else
		#define	 FUNDETECTION( name )
	#endif // OPEN_DETECTION

#else

	#define	 FUNDETECTION( name )

#endif
}



#endif 
