/******************************************************************************/
#ifndef _NETCONNECT_H_
#define _NETCONNECT_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{

	/******************************************************************************/
	//NetConnectInfo 链接相关信息
	/******************************************************************************/
	struct NetConnectInfo
	{
		Flt		mComingRateBytes;
		Flt		mComingLastRateBytes;
		Flt		mComingRateMaxBytes;	
		Flt		mComingRateMinBytes;
		I64		mComingBytes;

		Flt		mComingRateCounts;
		Flt		mComingLastRateCounts;
		Flt		mComingRateMaxCounts;	
		Flt		mComingRateMinCounts;
		I32		mComingCounts;

		I32		mLastComingTime;

		Flt		mGoingRateBytes;
		Flt		mGoingLastRateBytes;
		Flt		mGoingRateMaxBytes;	
		Flt		mGoingRateMinBytes;
		I64		mGoingBytes;

		Flt		mGoingRateCounts;
		Flt		mGoingLastRateCounts;
		Flt		mGoingRateMaxCounts;
		Flt		mGoingRateMinCounts;
		I32		mGoingCounts;

		I32		mLastGoingTime;

		U64		mStartTime;

        I32     mWaitSendPacketCount;
        I32     mMaxWaitSendPacketCount;

        I32     mWaitRecvPacketCount;
        I32     mMaxWaitRecvPacketCount;

		NetConnectInfo()
		{
			clear();
		}

		void clear()
		{
			mComingRateBytes = 0.0f;
			mComingLastRateBytes = 0.0f;
			mComingRateMaxBytes = 0.0f;	
			mComingRateMinBytes = 0.0f;
			mComingBytes = 0;

			mComingRateCounts = 0.0f;
			mComingLastRateCounts = 0.0f;
			mComingRateMaxCounts = 0.0f;	
			mComingRateMinCounts = 0.0f;
			mComingCounts = 0;

			mLastComingTime = 0;

			mGoingRateBytes = 0.0f;
			mGoingLastRateBytes = 0.0f;
			mGoingRateMaxBytes = 0.0f;	
			mGoingRateMinBytes = 0.0f;
			mGoingBytes = 0;

			mGoingRateCounts = 0.0f;
			mGoingLastRateCounts = 0.0f;
			mGoingRateMaxCounts = 0.0f;	
			mGoingRateMinCounts = 0.0f;
			mGoingCounts = 0;
			mLastGoingTime = 0;

			mStartTime = 0;

            mWaitSendPacketCount = 0;
            mMaxWaitSendPacketCount = 0;
            mWaitRecvPacketCount = 0;
            mMaxWaitRecvPacketCount = 0;
		}
	};
	
}

#endif