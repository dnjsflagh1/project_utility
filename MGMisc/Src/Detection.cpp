#include "stdafx.h"
#include "Detection.h"
#include "DetectionMgr.h"
namespace MG
{

	//-----------------------------------------------------------------------------
	FunDetection::FunDetection( CChar* name )
	{
		DYNAMIC_ASSERT(name);
		mCurrDetection = 0;
		mMaxDetection = 0;
		mMinDetection = 9999999;
		mCallCount = 0;
		if (name)
		{
			MGStrOp::sprintf(mName,64,"%s",name);
		}
		mTimer.cpuSpeedMHz();
	}
	//-----------------------------------------------------------------------------
	FunDetection::~FunDetection()
	{
	}
	//-----------------------------------------------------------------------------
	U64 FunDetection::getCurrDetection()
	{
		return mCurrDetection;
	}
	//-----------------------------------------------------------------------------
	Flt FunDetection::getAverDetection()
	{
		cs.lock();
		if (mAllDetection.empty())
		{
			cs.unlock();
			return 0.0f;
			
		}
	
		std::vector<U64>::iterator it = mAllDetection.begin();
		Flt tempAll = 0;
		
		for (; it != mAllDetection.end(); it++)
		{
			tempAll += *it;
		}
		cs.unlock();
		return tempAll / mAllDetection.size();
	}
	//-----------------------------------------------------------------------------
	U32 FunDetection::getCallCount()
	{

		return mCallCount;

	}
	//-----------------------------------------------------------------------------
	Flt FunDetection::getCallRate()
	{
		Flt temp = getAverDetection();
		if (temp > 0.0001)
		{
			return	1.0f / temp;
		}
		return 0.0f;
	}
	//-----------------------------------------------------------------------------
	Char* FunDetection::getName()
	{
		return mName;
	}
	//-----------------------------------------------------------------------------
	U64 FunDetection::getMaxDetection()
	{
		return mMaxDetection;
	}
	//-----------------------------------------------------------------------------
	U64 FunDetection::getMinDetection()
	{
		return mMinDetection;
	}
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	FunDetectionCalc::FunDetectionCalc( Detection* detection )
	{
		mDetection = (FunDetection*)detection;
		//mDetection->setThreadAffinityMask();
		//mDetection->mTimer.start();
		if (DetectionMgr::getInstance().getTimeMode() == DetectionTimeMode_TimeOp)
		{
			mStratTime = MGTimeOp::getCurrTick();
		}
		else
		{
			mDetection->setThreadAffinityMask();
			mDetection->mTimer.start();
		}
		
	}
	//-----------------------------------------------------------------------------
	FunDetectionCalc::~FunDetectionCalc()
	{
		if (mDetection)
		{
			if (DetectionMgr::getInstance().getTimeMode() == DetectionTimeMode_TimeOp)
			{
				mEndTime = MGTimeOp::getCurrTick();
				mDetection->mCurrDetection = mEndTime - mStratTime;
			}
			else
			{
				mDetection->mCurrDetection = mDetection->mTimer.cyclesToMillis( mDetection->mTimer.stop(), mDetection->mTimer.getCpuMHz());
			}

			if (mDetection->mCurrDetection > mDetection->mMaxDetection)
				 mDetection->mMaxDetection = mDetection->mCurrDetection;
			if (mDetection->mCurrDetection < mDetection->mMinDetection)
				mDetection->mMinDetection = mDetection->mCurrDetection;
			mDetection->mCallCount++;
			//mDetection->cs.lock();
			if (mDetection->mAllDetection.size() > 1024)
			{
				mDetection->mAllDetection.clear();
			}
			mDetection->mAllDetection.push_back(mDetection->mCurrDetection);
			if (MG::DetectionMgr::getInstance().getPrint())
			{
				MG_LOG(out_game_logic,"函数名%s:\t当前消耗 = %d,最大消耗 = %d,平均消耗 = %f,最小消耗 = %d,调用次数 = %d\n",
					mDetection->getName(),mDetection->getCurrDetection(),mDetection->getMaxDetection(),mDetection->getAverDetection(),mDetection->getMinDetection(),mDetection->getCallCount());
			}
			//mDetection->cs.unlock();
		}

	}
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	void Detection::setThreadAffinityMask()
	{
		if (!mIsSetThreadAffinityMask)
		{
			::SetThreadAffinityMask(::GetCurrentThread(),1);
			mIsSetThreadAffinityMask = true;
		}
	}
	//-----------------------------------------------------------------------------
	Detection::Detection()
	{
		mIsSetThreadAffinityMask = false;
	}

}