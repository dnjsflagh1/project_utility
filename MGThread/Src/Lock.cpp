/******************************************************************************/
#include "stdafx.h"
#include "Lock.h"
/******************************************************************************/

namespace MG
{
	#define OP_RWLOCKER_CATEGORY 7008

    /******************************************************************************/
    //Critical 互斥锁 
    /******************************************************************************/
	Critical::Critical(U32 spinCount)
        :mSpinCount(spinCount)
        ,mCreate(false)
	{
        // 创建临界区
		create();
	}
	//-----------------------------------------------------------------------------
	Critical::~Critical()
	{
        // 销毁临界区
        destroy();
	}
	//-----------------------------------------------------------------------------
	Critical::operator Bool() const
	{
		return mCreate;
	}

    //-----------------------------------------------------------------------------
    void Critical::create()
    {
        //判断是否已经创建
        if(!mCreate)
        {
            //创建临界区
            InitializeCriticalSectionAndSpinCount(&mCriticalSection,0x80000000 + mSpinCount);
            //设置标志位
            mCreate	= true;
        }
    }

    //-----------------------------------------------------------------------------
    void Critical::destroy()
    {
        if(mCreate)
        {
            //加锁
            lock();
            //设置标志位
            mCreate =false;
            //释放临界区
            DeleteCriticalSection(&mCriticalSection);
        }
    }

	//-----------------------------------------------------------------------------
	void Critical::lock() const
	{
        DYNAMIC_ASSERT(mCreate);
        EnterCriticalSection(&mCriticalSection);
	}
	//-----------------------------------------------------------------------------
	Bool Critical::tryLock() const
	{
        DYNAMIC_ASSERT(mCreate);
        return Bool( TryEnterCriticalSection(&mCriticalSection) != 0 );
	}
	//-----------------------------------------------------------------------------
	void Critical::unlock() const
	{
        DYNAMIC_ASSERT(mCreate);
        LeaveCriticalSection(&mCriticalSection);
	}

    /******************************************************************************/
    //CriticalArmor 带保护的互斥锁 
    /******************************************************************************/
	CriticalArmor::CriticalArmor( const Critical& critical ):mCritical(critical),mLockNum(0)
	{

	}
	//-----------------------------------------------------------------------------
	CriticalArmor::~CriticalArmor()
	{
		while(mLockNum)
		{
			unlock();
		}
	}
	//-----------------------------------------------------------------------------
	CriticalArmor::operator Bool() const
	{
		return mCritical;
	}

	void CriticalArmor::lock() const
	{
		mCritical.lock();
		mLockNum++;
	}
	//-----------------------------------------------------------------------------
	I32 CriticalArmor::trylock() const
	{
		I32 ret = mCritical.tryLock();
		if (ret)
		{
			mLockNum++;
		}
		return ret;
	}
	//-----------------------------------------------------------------------------
	void CriticalArmor::unlock() const
	{
		if (mLockNum)	
		{
			mLockNum--;
			mCritical.unlock();
		}
	}

    /******************************************************************************/
    //Semaphore 信号量 核心对象
    /******************************************************************************/

	Semaphore::Semaphore()
	{
		mHamdle = NULL;
		mMaxCount = 0;
	}
	//-----------------------------------------------------------------------------
	Semaphore::Semaphore( U32 initialCount,U32 maxCount,Char16* name /*= NULL*/ )
	{
		mMaxCount = maxCount;
		mHamdle = NULL;
		create(initialCount,maxCount,name);
	}
	//-----------------------------------------------------------------------------
	Semaphore::~Semaphore()
	{
		destroy();
	}
	//-----------------------------------------------------------------------------
	Semaphore::operator Bool()
	{
		return mHamdle != NULL;
	}
	//-----------------------------------------------------------------------------
	void Semaphore::destroy()
	{
		if (mHamdle)
		{
			CloseHandle(mHamdle);
			mHamdle = NULL;
		}
	}
	//-----------------------------------------------------------------------------
	I32 Semaphore::create( I32 initialCount,I32 maxCount,Char16* name /*= NULL*/ )
	{
		if (mHamdle == NULL)
		{
			mHamdle = CreateSemaphoreW(0,initialCount,maxCount,name);
			return 1;
		}
		else
		{
			return 0;
		}
		
	}
	//-----------------------------------------------------------------------------
	U32 Semaphore::lock()
	{
		return WaitForSingleObject(mHamdle,INFINITE);
	}
	//-----------------------------------------------------------------------------
	U32 Semaphore::tryLock()
	{
		return WaitForSingleObject(mHamdle,0);
	}
	//-----------------------------------------------------------------------------
	U32 Semaphore::timeLock( U32 milliSeconds )
	{
		return WaitForSingleObject(mHamdle,milliSeconds);
	}
	//-----------------------------------------------------------------------------
	I32 Semaphore::unlock( U32 relCount /*=1*/ )
	{
		return ReleaseSemaphore(mHamdle,relCount,0);
	}

    /******************************************************************************/
    // InterLocked
    /******************************************************************************/

    Char8 InterLocked::mSuportAcquire = 0;

	InterLocked::InterLocked( I32 initVal )
	{
		if(mSuportAcquire == 0)
		{
			suportAcquire();
		}
		InterlockedExchange((long*)&mValue,initVal);
	}
	//-----------------------------------------------------------------------------
	InterLocked::InterLocked( const InterLocked &val )
	{
		if(mSuportAcquire == 0)
		{
			suportAcquire();
		}
		InterlockedExchange((long*)&mValue,val);
	}
	//-----------------------------------------------------------------------------
	InterLocked::operator I32() const
	{
		return mValue;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator=( I32 val )
	{
		assign(val);
		return val;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator=( InterLocked &val )
	{
		assign(val);
		return val;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator++()
	{
		return increment();
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator++( int )
	{
		I32 ret = increment();
		return ret - 1;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator--()
	{
		return decrement();
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator--( int )
	{
		I32 ret = decrement();
		return ret - 1;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator+=( I32 val )
	{
		I32 ret = add(val);
		return ret + val;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator+=( InterLocked &val )
	{
		I32 ret = add(I32(val));
		return ret + val;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator-=( I32 val )
	{
		I32 ret = add(-val);
		return ret - val;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::operator-=( InterLocked &val )
	{
		I32 ret = add(I32(val));
		return ret - val;
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::setZero()
	{
		return assign(0);
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::increment()
	{
        if(mSuportAcquire == 0)
        {
            suportAcquire();
        }

		if( mSuportAcquire > 0)
		{
			return	InterlockedIncrementAcquire((long*)&mValue);
		}
		else if(mSuportAcquire <0)
		{
			return	InterlockedIncrement((long*)&mValue);
		}
		else
		{
			return -1;
		}
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::decrement()
	{
        if(mSuportAcquire == 0)
        {
            suportAcquire();
        }

		if( mSuportAcquire > 0)
		{
			return	InterlockedDecrementAcquire((long*)&mValue);
		}
		else if(mSuportAcquire <0)
		{
			return	InterlockedDecrement((long*)&mValue);
		}
		else
		{
			return -1;
		}
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::add( I32 val )
	{
		return InterlockedExchangeAdd((long*)&mValue,val);
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::assign( I32 newVal )
	{
		return InterlockedExchange((long*)&mValue,newVal);
	}
	//-----------------------------------------------------------------------------
	I32 InterLocked::compareAssign( I32 comperand,I32 newVal )
	{
        if(mSuportAcquire == 0)
        {
            suportAcquire();
        }

		if(mSuportAcquire > 0)
		{
			return InterlockedCompareExchangeAcquire((long*)&mValue,newVal,comperand);
		}
		else if(mSuportAcquire < 0)
		{
			return InterlockedCompareExchange((long*)&mValue,newVal,comperand);
		}else
		{
			return -1;
		}
	}
	//-----------------------------------------------------------------------------
	Bool InterLocked::suportAcquire()
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx = FALSE;
		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		// If that fails, try using the OSVERSIONINFO structure.
		ZeroMemory(&osvi, SIZE(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = SIZE(OSVERSIONINFOEX);
		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( FALSE == bOsVersionInfoEx )
		{
			osvi.dwOSVersionInfoSize = SIZE(OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			{
				return false;
			}
		}
		U32 l_ver = osvi.dwMajorVersion *0x10000 +osvi.dwMinorVersion;
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT 
			&& l_ver >=0x50002)	//Microsoft Windows Server&nbsp;2003 family
		{
			mSuportAcquire	=1;
		}
		else
		{
			mSuportAcquire	=-1;
		}
		return true;
	}

    /******************************************************************************/
    //RefArmor
    /******************************************************************************/


	RefArmor::RefArmor( InterLocked &count ):mCount(count)
	{
		++mCount;
	}

	RefArmor::~RefArmor()
	{
		--mCount;
	}


    /******************************************************************************/
    //RWLocker
    /******************************************************************************/

    //-----------------------------------------------------------------------------
    RWLocker::RWLocker()
    {
        mThreadLockList.reserve(32);
    }

    //-----------------------------------------------------------------------------
    RWLocker::~RWLocker()
    {
		destroyAllOneThreadLocker();
    }

    //-----------------------------------------------------------------------------
    void RWLocker::writeLock()
    {
		lockAllThread();
    }

    //-----------------------------------------------------------------------------
    void RWLocker::writeUnLock()
    {
		unLockAllThread();
    }

    //-----------------------------------------------------------------------------
    void RWLocker::readLock()
    {
		DWORD threadId = GetCurrentThreadId();
		RWLocker::OneThreadLocker* locker =  createOrRetrieveOneThreadLocker( threadId );
		locker->mThreadCs.lock();
    }

    //-----------------------------------------------------------------------------
    void RWLocker::readUnLock()
    {
		DWORD threadId = GetCurrentThreadId();
		RWLocker::OneThreadLocker* locker =  createOrRetrieveOneThreadLocker( threadId );
		locker->mThreadCs.unlock();
    }

    //-----------------------------------------------------------------------------
    RWLocker::OneThreadLocker* RWLocker::createOrRetrieveOneThreadLocker( UInt threadID )
    {
        RWLocker::OneThreadLocker* locker =  retrieveOneThreadLocker( threadID );

        if ( locker == NULL )
        {
			DYNAMIC_ASSERT_LOG( mThreadLockList.size() < 32, "locker is to much!");

			locker = MG_POOL_NEW_T( OneThreadLocker, OP_RWLOCKER_CATEGORY )();
			locker->mThreadID = threadID;

            mThreadLockListCs.lock();
            {
				mThreadLockList.push_back( locker );
            }
            mThreadLockListCs.unlock();

            locker =  retrieveOneThreadLocker( threadID );

            DYNAMIC_ASSERT_LOG( locker != NULL, "locker is null!");
        }

        return locker;
    }

    //-----------------------------------------------------------------------------
    RWLocker::OneThreadLocker* RWLocker::retrieveOneThreadLocker( UInt threadID )
    {
        RWLocker::OneThreadLocker* locker       = NULL;
        RWLocker::OneThreadLocker* matchLocker  = NULL;

        UInt count = mThreadLockList.size();

        for ( UInt i=0; i<count; i++ )
        {
            locker = mThreadLockList[i];

            if ( locker->mThreadID == threadID )
            {
                matchLocker = locker;
                break;
            }
        }

        return matchLocker;
    }

	//-----------------------------------------------------------------------------
	void RWLocker::destroyAllOneThreadLocker()
	{
		mThreadLockListCs.lock();
		{
			RWLocker::OneThreadLocker* locker	= NULL;

			UInt count = mThreadLockList.size();

			for ( UInt i=0; i<count ; i++ )
			{
				locker = mThreadLockList[i];

				MG_POOL_DELETE_T( locker, OneThreadLocker, OP_RWLOCKER_CATEGORY );
			}
			mThreadLockList.clear();
		}
		mThreadLockListCs.unlock();
	}

    //-----------------------------------------------------------------------------
    void RWLocker::lockAllThread()
    {
        mThreadLockListCs.lock();
        
        RWLocker::OneThreadLocker* locker   = NULL;
        UInt count = mThreadLockList.size();

        for ( UInt i=0; i<count ; i++ )
        {
            locker = mThreadLockList[i];

            locker->mThreadCs.lock();
        }
    }

    //-----------------------------------------------------------------------------
    void RWLocker::unLockAllThread()
    {        
        RWLocker::OneThreadLocker* locker   = NULL;
        UInt count = mThreadLockList.size();

        for ( UInt i=0; i<count ; i++ )
        {
            locker = mThreadLockList[i];

            locker->mThreadCs.unlock();
        }

        mThreadLockListCs.unlock();
    }

}