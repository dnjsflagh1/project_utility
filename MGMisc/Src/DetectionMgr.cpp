#include "stdafx.h"
#include "DetectionMgr.h"
namespace MG
{
	//-----------------------------------------------------------------------------
	DetectionMgr::DetectionMgr()
	{
		mIsPrint = true;
		mTimerMode = DetectionTimeMode_TimeOp;
	}
//-----------------------------------------------------------------------------
	DetectionMgr::~DetectionMgr()
	{

	}
//-----------------------------------------------------------------------------
	Detection* DetectionMgr::getFunDetection( CChar* funName )
	{
		Detection* p = _find(funName);
		if (p)
		{
			return p;
		}
		else
		{
			p = new FunDetection(funName);
			mDetections[funName] = p;
			return p;
		}
			
	}
//-----------------------------------------------------------------------------
	U64 DetectionMgr::getCurrDetection( CChar* funName )
	{
		Detection* p = _find(funName);
		if (p)
		{
			return p->getCurrDetection();
		}
		return 0;
	}
//-----------------------------------------------------------------------------
	U64 DetectionMgr::getMaxDetection( CChar* funName )
	{
		Detection* p = _find(funName);
		if (p)
		{
			return p->getMaxDetection();
		}
		return 0;
	}
//-----------------------------------------------------------------------------
	Flt DetectionMgr::getAverDetection( CChar* funName )
	{
		Detection* p = _find(funName);
		if (p)
		{
			return p->getAverDetection();
		}
		return 0.0f;
	}
//-----------------------------------------------------------------------------
	U32 DetectionMgr::getCallCount( CChar* funName )
	{
		Detection* p = _find(funName);
		if (p)
		{
			return p->getCallCount();
		}
		return 0;
	}
//-----------------------------------------------------------------------------
	Flt DetectionMgr::getCallRate( CChar* funName )
	{
		Detection* p = _find(funName);
		if (p)
		{
			return p->getCallRate();
		}
		return 0.0f;
	}
//-----------------------------------------------------------------------------
	Detection* DetectionMgr::_find( CChar* name)
	{
		DetectionIt it = mDetections.find(name);
		DetectionIt itEnd = mDetections.end();
		if (it != itEnd)
		{
			return it->second;
		}
		return NULL;
	}
//-----------------------------------------------------------------------------
	void DetectionMgr::setPrint( Bool isPrint )
	{
		mIsPrint = isPrint;
	}
//-----------------------------------------------------------------------------
	Bool DetectionMgr::getPrint()
	{
		return mIsPrint;
	}
//-----------------------------------------------------------------------------
	MG::DetectionTimeMode DetectionMgr::getTimeMode()
	{
		return mTimerMode;
	}

};