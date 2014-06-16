/******************************************************************************/
#include "stdafx.h"
#include "Thread.h"
#include "ThreadPool.h"
#include "Task.h"
#include <MMSystem.h>
/******************************************************************************/

namespace MG
{

    //-------------------------------------------------------------------------------
	Thread::Thread( ThreadPool *threadPool ):mThreadPool(threadPool),mFreeFlag(true)
	{
		//mHandle = CreateThread(0,0,threadProc,this,0,(unsigned long*)(&mThreadId));
		mHandle = (Ptr)_beginthreadex(NULL,0,threadProc,(LPVOID)this,0,&mThreadId);

		DYNAMIC_ASSERT_LOG(mHandle,"create thread fail!");
		if( mHandle == NULL)
		{
			throw;
		}
		mThreadPool->mCurr++;
		mThreadPool->mFree++;
	}

    //-------------------------------------------------------------------------------
	Thread::~Thread()
	{
		//closehandle 放在线程中处理	
	}

    //-------------------------------------------------------------------------------
	U32 API Thread::threadProc( void* thd )
	{

		try
		{
			Thread* tempThread = (Thread*)thd;
			ThreadPool* pool = tempThread->getPool();
			SetThreadPriority(GetCurrentThread(),pool->mPriority);
			pool->threadExcute(tempThread);
			SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
		}
		catch (const runtime_error &e)
		{
			std::string what = "Thread 线程异常";
			what += e.what();
			DYNAMIC_ASSERT_LOG(false,what.c_str());
		}
		catch (...)
		{
			DYNAMIC_ASSERT_LOG(false,"Thread 线程异常");
		}

		return 0;
	}

    //-------------------------------------------------------------------------------
	Ptr Thread::getHandle()
	{
		return mHandle;
	}

    //-------------------------------------------------------------------------------
	U32 Thread::getThreadID()
	{
		return mThreadId;
	}
	//-------------------------------------------------------------------------------
	void Thread::SetThreadName( U32 dwThreadID, Char8* szThreadName )
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = szThreadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;

		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD),
				(DWORD *)&info);
		}
		_except (EXCEPTION_CONTINUE_EXECUTION)
		{
		}
	}
    //-------------------------------------------------------------------------------
	ThreadPool * Thread::getPool()
	{
		return mThreadPool;
	}
    //-------------------------------------------------------------------------------
	void ThreadQueue::addThread( Thread* thd )
	{
		thd->mLast	= 0;
		thd->mNext	= mThreadQueue;
		if(mThreadQueue)
		{
			mThreadQueue->mLast = thd;
		}
		mThreadQueue = thd;
	}

    //-------------------------------------------------------------------------------
	void ThreadQueue::delThread( Thread* thd )
	{
		if(thd->mNext)
		{
			thd->mNext->mLast = thd->mLast;
		}
		if(thd->mLast)
		{
			thd->mLast->mNext = thd->mNext;
		}
		if(mThreadQueue ==thd)
		{
			mThreadQueue = thd->mNext;
		}
	}

    //-------------------------------------------------------------------------------
	ThreadQueue::ThreadQueue():mThreadQueue(NULL)
	{
		if( mLockPool == NULL || mLockUpdate == NULL)
		{
			DYNAMIC_ASSERT(false);
			throw;
		}
	}

    //-------------------------------------------------------------------------------
    OneThreadHandle::OneThreadHandle()
        :mHandle(NULL),
        mCallback(NULL),
		mCallback1(NULL),
        mPtr(NULL),
		mThreadId(0)
    {
        
    }

    //-------------------------------------------------------------------------------
    OneThreadHandle::~OneThreadHandle()
    {
        destory();
    }

    //-------------------------------------------------------------------------------
    Bool OneThreadHandle::create(void (*callback)(Ptr),Ptr ptr,Char8* threadName)
    {
        destory();
        mCallback = callback;
        mPtr = ptr;
        mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			START_MONITOR_THREAD(mThreadId,threadName,mHandle)
			mIsRun = true;
			ResumeThread(mHandle);
			return true;
		}
		else
		{
			DYNAMIC_ASSERT_LOG( false, " OneThreadHandle::create Failed !" )
			mIsRun = false;
		}
        return false;
    }
	//-------------------------------------------------------------------------------
	Bool OneThreadHandle::create(void (*callback)(Ptr,U32),Ptr ptr,Char8* threadName)
	{
		destory();
		mCallback1 = callback;
		mPtr = ptr;
		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			START_MONITOR_THREAD(mThreadId,threadName,mHandle)
			mIsRun = true;
			ResumeThread(mHandle);
			return true;
		}
		else
		{
			DYNAMIC_ASSERT_LOG( false, " OneThreadHandle::create Failed !" )
				mIsRun = false;
		}
		return false;
	}
    //-------------------------------------------------------------------------------
    Bool OneThreadHandle::destory()
    {
		mIsRun = false;
		if(mHandle)
		{
			if (WaitForSingleObject(mHandle,10 * 1000) == WAIT_TIMEOUT)
			{
				DYNAMIC_ASSERT_LOG(false,"线程逻辑异常,10秒无法退出");
			}	
			CloseHandle(mHandle);
			mHandle = NULL;
		}
		mCallback = NULL;
		mCallback1 = NULL;
        return true;
    }

    //-------------------------------------------------------------------------------
    U32 API OneThreadHandle::threadFunction(Ptr p)
    {
		
		try
		{ 
			OneThreadHandle* mgr = (OneThreadHandle*)p;
			if (mgr)
			{
				if (mgr->mCallback != NULL)
				{
					mgr->mCallback(p);
				}
				else if (mgr->mCallback1 != NULL)
				{
					mgr->mCallback1(p,mgr->mThreadId);
				}
				mgr->mHandle = NULL;
			}
			mgr->mThreadId = 0;
		}
		catch (const runtime_error &e)
		{
			std::string what = "OneThreadHandle 线程异常";
			what += e.what();
			DYNAMIC_ASSERT_LOG(false,what.c_str());
		}
		catch (...)
		{
			DYNAMIC_ASSERT_LOG(false,"OneThreadHandle 线程异常");
		}
        return 0;
    }
	//-------------------------------------------------------------------------------
	void* OneThreadHandle::getParam()
	{
		return mPtr;
	}
	//-------------------------------------------------------------------------------
	Bool OneThreadHandle::isRun()
	{
		return mIsRun;
	}
	//-------------------------------------------------------------------------------
	Ptr OneThreadHandle::getHandle()
	{
		return mHandle;
	}
	//-------------------------------------------------------------------------------
	U32 OneThreadHandle::getThreadId()
	{
		return mThreadId;
	}
	//===============================================================================
	ModeTaskThread::ModeTaskThread()
	{
		mPtr = NULL;
		mDestory = 1;
		mIsRun = false;
		mHandle = NULL;
		mThreadId = 0;
	}

	//-------------------------------------------------------------------------------
	ModeTaskThread::~ModeTaskThread()
	{
		destory();
	}
	MG::Bool ModeTaskThread::create( Ptr ptr,Char8* threadName, I32 maxTaskCount , Int priority  )
	{
		destory();

		mPtr = ptr;
		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			/*Str8 strThreadId;
			Str8 strThreadName = threadName;
			strThreadName += '_';
			MGStrOp::toString(mThreadId,strThreadId);
			strThreadName += strThreadId;
			Thread::SetThreadName(mThreadId,(Char8*)strThreadName.c_str());
			ThreadMonitor::getInstance().addThread(strThreadName.c_str(),mThreadId,mHandle);*/
			START_MONITOR_THREAD(mThreadId,threadName,mHandle)

			mIsRun = true;
			mDestory = 0;
			if (priority > 0)
			{
				SetThreadPriority( mHandle, priority );
			}
			ResumeThread( mHandle );
			return true;
		}
		else
		{
			DYNAMIC_ASSERT_LOG( false, "ModeTaskThread::create Failed !" )
			mIsRun = false;
		}
		return false;
	}

    //-------------------------------------------------------------------------------
	// TODO: 须修改，做同步
	void ModeTaskThread::destory()
	{
		mDestory = 1;
		if ( mHandle )
		{
			mIsRun = false;
			if(mHandle)
			{
				if (WaitForSingleObject(mHandle,10 * 1000) == WAIT_TIMEOUT)
				{
					DYNAMIC_ASSERT_LOG(false,"线程逻辑异常,10秒无法退出");
				}	
				CloseHandle(mHandle);
				mHandle = NULL;
			}
		}
		while(!mTaskQueueForTraversal.empty())
		{
			mTaskQueueForTraversal.pop();
		}

		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.begin();
		for (; it != mTaskMapsForAdd.end(); it++)
		{
			ModeTaskQueue* taskQueue = it->second;
			DYNAMIC_ASSERT(taskQueue);
			delete taskQueue;
		}
		mTaskMapsForAdd.clear();
	}

    //-------------------------------------------------------------------------------
	void* ModeTaskThread::getParam()
	{
		return mPtr;
	}

    //-------------------------------------------------------------------------------
	Bool ModeTaskThread::isRun()
	{
		return mIsRun;
	}

    //-------------------------------------------------------------------------------
	Ptr ModeTaskThread::getHandle()
	{
		return mHandle;
	}

    //-------------------------------------------------------------------------------
	I32 ModeTaskThread::addTask( ModeTask* task )
	{
		if (mDestory == 1)
		{
			return -1;
		}
		if (!task)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}

		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.find(task->mType);
		if (it != mTaskMapsForAdd.end())
		{
			DYNAMIC_ASSERT(it->second);
			it->second->addTask(task);
			return it->second->getTaskCount();
		}
		else
		{
			ModeTaskQueue* taskQueue = new ModeTaskQueue();
			taskQueue->addTask(task);
			mTaskMapsForAdd[task->mType] = taskQueue;

			mTaskQueueForTraversalCs.lock();
			mTaskQueueForTraversal.push(taskQueue);
			mTaskQueueForTraversalCs.unlock();
			return 0;
		}

		return -1;
	}

    //-------------------------------------------------------------------------------
	I32 ModeTaskThread::getCurrTaskCount()
	{
		return -1;
	}

    //-------------------------------------------------------------------------------
	U32 API ModeTaskThread::threadFunction( Ptr p )
	{
		try
		{
			ModeTaskThread* modeThread = (ModeTaskThread*)p; 
			if (modeThread)
			{
				ModeTask* modeTask = NULL;
				while(modeThread->mIsRun)
				{
					modeTask = modeThread->getTask();
					if (modeTask)
					{
						modeTask->process();
						modeTask->free();
					}
					else
					{
						Sleep(5);
					}
				}
			}
		}
		catch (const runtime_error &e)
		{
			std::string what = "ModeTaskThread 线程异常";
			what += e.what();
			DYNAMIC_ASSERT_LOG(false,what.c_str());
		}
		catch (...)
		{

			DYNAMIC_ASSERT_LOG(false,"ModeTaskThread 线程异常");
		}
		return 0;
	}

    //-------------------------------------------------------------------------------
	ModeTask* ModeTaskThread::getTask()
	{
		ModeTask* task = NULL;
		ModeTaskQueue* taskQueue = NULL;
		if (mTaskQueueForTraversal.empty())
		{
			return NULL;
		}
		mTaskQueueForTraversalCs.lock();
		I32 size = mTaskQueueForTraversal.size();
		for (I32 i = 0; i < size; i++)
		{
			taskQueue = mTaskQueueForTraversal.front();
			mTaskQueueForTraversal.pop();
			task = taskQueue->getTask();
			mTaskQueueForTraversal.push(taskQueue);
			if (task)
			{
				break;
			}
		}
		mTaskQueueForTraversalCs.unlock();
		return task;
	}
	//-------------------------------------------------------------------------------
	U32 ModeTaskThread::getThreadId()
	{
		return mThreadId;
	}
	//-------------------------------------------------------------------------------
	Bool ModeTaskThread::removeTask( U32 type,U32 key )
	{
		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.find(type);
		if (it != mTaskMapsForAdd.end())
		{
			DYNAMIC_ASSERT(it->second);
			return it->second->removeTask(type,key);
		}
		return false;
	}
	//===============================================================================
	ModeTaskListenThread::ModeTaskListenThread()
	{
		mPtr = NULL;
		mIsRun = false;
		mHandle = NULL;
		mDestory = 1;
		mThreadId = 0;
	}

	//-------------------------------------------------------------------------------
	ModeTaskListenThread::~ModeTaskListenThread()
	{

	}
	MG::Bool ModeTaskListenThread::create( Ptr ptr,Char8* threadName, I32 maxTaskCount , Int priority  )
	{
		destory();

		mPtr = ptr;
		//mTaskQueue = new ModeTaskQueue(maxTaskCount);

	
		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );

		
		

		if (mHandle > 0)
		{
			START_MONITOR_THREAD(mThreadId,threadName,mHandle)
			mIsRun = true;
			mDestory = 0;
			if (priority > 0)
			{
				SetThreadPriority( mHandle, priority );
			}
			ResumeThread( mHandle );
			return true;
		}
		else
		{
			DYNAMIC_ASSERT_LOG( false, "ModeTaskListenThread::create Failed !" )
			mIsRun = false;
		}
		return false;
	}

	//-------------------------------------------------------------------------------
	// TODO: 须修改，做同步
	void ModeTaskListenThread::destory()
	{
		mDestory = 1;
		if ( mHandle )
		{
			mIsRun = false;
			if(mHandle)
			{
				if (WaitForSingleObject(mHandle,10 * 1000) == WAIT_TIMEOUT)
				{
					DYNAMIC_ASSERT_LOG(false,"线程逻辑异常,10秒无法退出");
				}	
				CloseHandle(mHandle);
				mHandle = NULL;
			}
		}
		while(!mTaskQueueForTraversal.empty())
		{
			mTaskQueueForTraversal.pop();
		}
		
		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.begin();
		for (; it != mTaskMapsForAdd.end(); it++)
		{
			ModeTaskQueue* taskQueue = it->second;
			DYNAMIC_ASSERT(taskQueue);
			delete taskQueue;
		}
		mTaskMapsForAdd.clear();
	}

	//-------------------------------------------------------------------------------
	void* ModeTaskListenThread::getParam()
	{
		return mPtr;
	}

	//-------------------------------------------------------------------------------
	Bool ModeTaskListenThread::isRun()
	{
		return mIsRun;
	}

	//-------------------------------------------------------------------------------
	Ptr ModeTaskListenThread::getHandle()
	{
		return mHandle;
	}
	//-------------------------------------------------------------------------------
	U32 ModeTaskListenThread::getThreadId()
	{
		return mThreadId;	
	}
	//-------------------------------------------------------------------------------
	I32 ModeTaskListenThread::addTask( ModeTask* task )
	{
		if (mDestory == 1)
		{
			return -1;
		}
		if (!task)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		
		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.find(task->mType);
		if (it != mTaskMapsForAdd.end())
		{
			DYNAMIC_ASSERT(it->second);
			it->second->addTask(task);
			return it->second->getTaskCount();
		}
		else
		{
			ModeTaskQueue* taskQueue = new ModeTaskQueue();
			taskQueue->addTask(task);
			mTaskMapsForAdd[task->mType] = taskQueue;

			mTaskQueueForTraversalCs.lock();
			mTaskQueueForTraversal.push(taskQueue);
			mTaskQueueForTraversalCs.unlock();
			return 0;
		}

		return -1;
	}

	//-------------------------------------------------------------------------------
	I32 ModeTaskListenThread::getCurrTaskCount()
	{
		return -1;
	}

	//-------------------------------------------------------------------------------
	U32 API ModeTaskListenThread::threadFunction( Ptr p )
	{
		try
		{
			ModeTaskListenThread* modeListenThread = (ModeTaskListenThread*)p; 
			if (modeListenThread)
			{
				ModeTask* modeTask = NULL;
				while(modeListenThread->mIsRun)
				{
					modeListenThread->listenEvent();
					
					modeTask = modeListenThread->getTask();
					if (modeTask)
					{
						modeTask->process();
						modeTask->free();
					}
					else
					{
						Sleep(5);
					}
				}
			}
		}
		catch (const runtime_error &e)
		{
			std::string what = "ModeTaskListenThread 线程异常";
			what += e.what();
			DYNAMIC_ASSERT_LOG(false,what.c_str());
		}
		catch (...)
		{
			DYNAMIC_ASSERT_LOG(false,"ModeTaskListenThread 线程异常");
		}
		return 0;
	}

	//-------------------------------------------------------------------------------
	ModeTask* ModeTaskListenThread::getTask()
	{
		ModeTask* task = NULL;
		ModeTaskQueue* taskQueue = NULL;
		if (mTaskQueueForTraversal.empty())
		{
			return NULL;
		}
		mTaskQueueForTraversalCs.lock();
		I32 size = mTaskQueueForTraversal.size();
		for (I32 i = 0; i < size; i++)
		{
			taskQueue = mTaskQueueForTraversal.front();
			mTaskQueueForTraversal.pop();
			task = taskQueue->getTask();
			mTaskQueueForTraversal.push(taskQueue);
			if (task)
			{
				break;
			}
		}
		mTaskQueueForTraversalCs.unlock();
		return task;
	}
	//-------------------------------------------------------------------------------
	Bool ModeTaskListenThread::removeTask( U32 type,U32 key )
	{
		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.find(type);
		if (it != mTaskMapsForAdd.end())
		{
			DYNAMIC_ASSERT(it->second);
			return it->second->removeTask(type,key);
		}
		return false;
	}
	//-------------------------------------------------------------------------------
	//===============================================================================
	//ThreadMonitor
	//===============================================================================
	ThreadMonitor::ThreadMonitor()
		:mCalcFrequency(1)
		,mIsPrintLog(false)
		,mIsStart(false)
	{

	}
	//-------------------------------------------------------------------------------
	ThreadMonitor::~ThreadMonitor()
	{

	}
	//-------------------------------------------------------------------------------
	Bool ThreadMonitor::initialize()
	{
		return mMonitorThread.create(threadFunction,this,"ThreadMonitor");
	}
	//-------------------------------------------------------------------------------
	void ThreadMonitor::unInitialize()
	{
		mIsStart = false;
		mMonitorThread.destory();
	}
	//-------------------------------------------------------------------------------
	void ThreadMonitor::setCalcFrequency( Int second )
	{
		mCalcFrequency = second;
	}
	//-------------------------------------------------------------------------------
	void ThreadMonitor::setPrintLog( Bool printLog )
	{
		mIsPrintLog = printLog;
	}
	//-------------------------------------------------------------------------------
	void ThreadMonitor::start()
	{
		mIsStart = true;
	}
	//-------------------------------------------------------------------------------
	ThreadInfo* ThreadMonitor::getThreadInfobyName( Char8* name )
	{
		ThreadInfo* threadInfo = NULL;
		CriticalArmor armor(mLock);
		std::map<Str8,ThreadInfo*>::iterator it = mThreadInfosbyName.find(name);
		if (it != mThreadInfosbyName.end())
		{
			threadInfo = it->second;
		}
		return threadInfo;
	}
	//-------------------------------------------------------------------------------
	ThreadInfo* ThreadMonitor::getThreadInfobyId( U32 threadId )
	{
		ThreadInfo* threadInfo = NULL;
		CriticalArmor armor(mLock);
		std::map<U32,ThreadInfo*>::iterator it = mThreadInfosbyId.find(threadId);
		if (it != mThreadInfosbyId.end())
		{
			threadInfo = it->second;
		}
		return threadInfo;
	}
	//-------------------------------------------------------------------------------
	std::map<U32,ThreadInfo*>* ThreadMonitor::getAllThreadInfo()
	{
		return &mThreadInfosbyId;
	}
	//-------------------------------------------------------------------------------
	void ThreadMonitor::addThread( Str8 threadName,U32 threadId,Ptr threadHandle )
	{
		CriticalArmor armor(mLock);
		if (mThreadInfosbyId.find(threadId) != mThreadInfosbyId.end())
		{
			DYNAMIC_ASSERT_LOG(false,"线程已经存在");
			return;
		}
		ThreadInfo* threadInfo = MG_NEW ThreadInfo;
		threadInfo->threadName = threadName;
		threadInfo->threadId = threadId;
		threadInfo->threadHandle = threadHandle;
		mThreadInfosbyId[threadId] = threadInfo;
		threadName += threadId; 
		mThreadInfosbyName[threadName] = threadInfo;
	}
	//-------------------------------------------------------------------------------
	void  ThreadMonitor::threadFunction( Ptr p,U32 threadId)
	{
		OneThreadHandleNew* thd = (OneThreadHandleNew* )p;
		if ( thd )
		{
			ThreadMonitor* monitor = (ThreadMonitor*)thd->getParam();
			static U32 timeTick = timeGetTime();
			SYSTEM_INFO		sysinfo;
			GetSystemInfo( &sysinfo );
			Int cpuCount = sysinfo.dwNumberOfProcessors;
			Int threadSize = 0;
			Flt processCpu = 0;
			while( thd->isRun())
			{
				if (monitor->mIsStart)
				{
					if (monitor->mThreadInfosbyId.empty())
					{
						Sleep(5);
					}
					else
					{
						U32 timeOffset = timeGetTime() - timeTick;
						if ( timeOffset >= monitor->mCalcFrequency * 1000)
						{
							timeTick += timeOffset;
							{
								CriticalArmor armor(monitor->mLock);
								std::map<U32,ThreadInfo*> ::iterator it = monitor->mThreadInfosbyId.begin();
								
								threadSize = monitor->mThreadInfosbyId.size();
								processCpu = 0.0f;
								Dbl tempTime = timeOffset;
								for (Int count = 0; it != monitor->mThreadInfosbyId.end(); it++,count++)
								{
									ThreadInfo* threadInfo = it->second;
									if (threadInfo)
									{

										threadInfo->lastKernelTime = threadInfo->kernelTime;
										threadInfo->lastUserTime = threadInfo->userTime;

										//Dbl tempTime = (timeGetTime() - timeTick ) * 0.001f + monitor->mCalcFrequency;
										GetThreadTimes((HANDLE)threadInfo->threadHandle,&threadInfo->createTime,&threadInfo->exitTime,&threadInfo->kernelTime,&threadInfo->userTime);
										U64 currKernelTime				= *(U64*)&threadInfo->kernelTime;
										U64 lastKernelTime				= *(U64*)&threadInfo->lastKernelTime;
										U64 currUserTime				= *(U64*)&threadInfo->userTime;
										U64 lastUserTime				= *(U64*)&threadInfo->lastUserTime;

										Flt diffKernel					= (currKernelTime - lastKernelTime) / monitor->mCalcFrequency;//(*(Flt*)(*(U64*)(&threadInfo->kernelTime) - *(U64*)(&threadInfo->lastKernelTime))) / tempTime;
										Flt diffUser					= (currUserTime - lastUserTime) / monitor->mCalcFrequency;//(*(Flt*)(*(U64*)(&threadInfo->userTime) - *(U64*)(&threadInfo->lastUserTime))) / tempTime;
										
										threadInfo->threadKernelCpu		= diffKernel * 1E-5 / (Flt)cpuCount;
										threadInfo->threadUserCpu		= diffUser * 1E-5 / (Flt)cpuCount;
										threadInfo->threadCpu			= threadInfo->threadKernelCpu + threadInfo->threadUserCpu;
										threadInfo->threadKernelCpuTime = (*((U64*)&threadInfo->kernelTime)) * 1E-5;
										threadInfo->threadUserCpuTime	= (*((U64*)&threadInfo->userTime)) * 1E-5;
										threadInfo->threadCpuTime		= threadInfo->threadKernelCpuTime + threadInfo->threadUserCpuTime;
										processCpu						+= threadInfo->threadCpu;
										if (monitor->mIsPrintLog)
										{
											MG_LOG(out_info,"线程名:%s,id=%d,句柄=0x%0x,\n\t\tcup使用:总 = %.2f,\t内核态 = %.2f,\t用户态 = %.2f\n\t\tcpu时间:总 = %d,\t内核态 = %d,\t用户态 = %d\n" ,
												threadInfo->threadName.c_str(),threadInfo->threadId,threadInfo->threadHandle,
												threadInfo->threadCpu,threadInfo->threadKernelCpu,threadInfo->threadUserCpu,
												threadInfo->threadCpuTime,threadInfo->threadKernelCpuTime,threadInfo->threadUserCpuTime);
											
											if(count + 1 == threadSize)
											{
												MG_LOG(out_warning,"(---------------进程cpu = %.2f---------------)\n",processCpu);
											}
											
										}
									}
									else
									{
										DYNAMIC_ASSERT_LOG(false,"线程监视器，内存出错！！！");
									}
								}
							}
							Sleep(5);

						}
						else
						{
							Sleep(5);
						}

					}
				}
				else
				{
					Sleep(10);
				}
			}
		}
	}
	//-------------------------------------------------------------------------------
	void ThreadMonitor::lockThreadInfos()
	{
		mLock.lock();
	}
	//-------------------------------------------------------------------------------
	void ThreadMonitor::unlockThreadInfos()
	{
		mLock.unlock();
	}
	//-------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------
	OneThreadHandleNew::OneThreadHandleNew()
		:mHandle(NULL),
		mCallback(NULL),
		mCallback1(NULL),
		mPtr(NULL),
		mThreadId(0)
	{

	}

	//-------------------------------------------------------------------------------
	OneThreadHandleNew::~OneThreadHandleNew()
	{
		destory();
	}

	//-------------------------------------------------------------------------------
	Bool OneThreadHandleNew::create(void (*callback)(Ptr),Ptr ptr,Char8* threadName)
	{
		destory();
		mCallback = callback;
		mPtr = ptr;
		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			START_MONITOR_THREAD(mThreadId,threadName,mHandle)
			mIsRun = true;
			ResumeThread(mHandle);
			return true;
		}
		else
		{
			DYNAMIC_ASSERT_LOG( false, " OneThreadHandleNew::create Failed !" )
			mIsRun = false;
		}
		return false;
	}
	//-------------------------------------------------------------------------------
	Bool OneThreadHandleNew::create(void (*callback)(Ptr,U32),Ptr ptr,Char8* threadName)
	{
		destory();
		mCallback1 = callback;
		mPtr = ptr;
		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			START_MONITOR_THREAD(mThreadId,threadName,mHandle)
			mIsRun = true;
			ResumeThread(mHandle);
			return true;
		}
		else
		{
			DYNAMIC_ASSERT_LOG( false, " OneThreadHandleNew::create Failed !" )
				mIsRun = false;
		}
		return false;
	}
	//-------------------------------------------------------------------------------
	Bool OneThreadHandleNew::destory()
	{
		mIsRun = false;
		if(mHandle)
		{
			if (WaitForSingleObject(mHandle,10 * 1000) == WAIT_TIMEOUT)
			{
				DYNAMIC_ASSERT_LOG(false,"线程逻辑异常,10秒无法退出");
			}	
			CloseHandle(mHandle);
			mHandle = NULL;
		}
		mCallback = NULL;
		mCallback1 = NULL;
		
		return true;
	}

	//-------------------------------------------------------------------------------
	U32 API OneThreadHandleNew::threadFunction(Ptr p)
	{
		try
		{ 
			OneThreadHandleNew* mgr = (OneThreadHandleNew*)p;
			if (mgr)
			{
				if (mgr->mCallback != NULL)
				{
					mgr->mCallback(p);
				}
				else if (mgr->mCallback1 != NULL)
				{
					mgr->mCallback1(p,mgr->mThreadId);
				}
				mgr->mHandle = NULL;
			}
			mgr->mThreadId = 0;
		}
		catch (const runtime_error &e)
		{
			std::string what = "OneThreadHandleNew 线程异常";
			what += e.what();
			DYNAMIC_ASSERT_LOG(false,what.c_str());
		}
		catch (...)
		{
			DYNAMIC_ASSERT_LOG(false,"OneThreadHandleNew 线程异常");
		}
		return 0;
	}
	//-------------------------------------------------------------------------------
	void* OneThreadHandleNew::getParam()
	{
		return mPtr;
	}
	//-------------------------------------------------------------------------------
	Bool OneThreadHandleNew::isRun()
	{
		return mIsRun;
	}
	//-------------------------------------------------------------------------------
	Ptr OneThreadHandleNew::getHandle()
	{
		return mHandle;
	}
	//-------------------------------------------------------------------------------
	U32 OneThreadHandleNew::getThreadId()
	{
		return mThreadId;
	}
	//===============================================================================
	ModeTaskListenThreadNew::ModeTaskListenThreadNew()
	{
		mPtr = NULL;
		mIsRun = false;
		mHandle = NULL;
		mDestory = 1;
		mThreadId = 0;
		mAllTaskCount = 0;
	}

	//-------------------------------------------------------------------------------
	ModeTaskListenThreadNew::~ModeTaskListenThreadNew()
	{

	}
	MG::Bool ModeTaskListenThreadNew::create( Ptr ptr,Char8* threadName, I32 maxTaskCount , Int priority  )
	{
		destory();

		mPtr = ptr;
		//mTaskQueue = new ModeTaskQueue(maxTaskCount);


		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		
		if (mHandle > 0)
		{
			START_MONITOR_THREAD(mThreadId,threadName,mHandle)
			if (priority > 0)
			{
				SetThreadPriority( mHandle, priority );
			}
			mIsRun = true;
			mDestory = 0;
			ResumeThread( mHandle );
			return true;
		}
		else
		{
			DYNAMIC_ASSERT_LOG( false, "ModeTaskListenThread::create Failed !" )
				mIsRun = false;
		}
		return false;
	}

	//-------------------------------------------------------------------------------
	// TODO: 须修改，做同步
	void ModeTaskListenThreadNew::destory()
	{
		mDestory = 1;
		mIsRun = false;
		if(mHandle)
		{
			if (WaitForSingleObject(mHandle,10 * 1000) == WAIT_TIMEOUT)
			{
				DYNAMIC_ASSERT_LOG(false,"线程逻辑异常,10秒无法退出");
			}	
			CloseHandle(mHandle);
			mHandle = NULL;
		}
		while(!mTaskQueueForTraversal.empty())
		{
			mTaskQueueForTraversal.pop();
		}

		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.begin();
		for (; it != mTaskMapsForAdd.end(); it++)
		{
			ModeTaskQueue* taskQueue = it->second;
			DYNAMIC_ASSERT(taskQueue);
			delete taskQueue;
		}
		mTaskMapsForAdd.clear();
	}

	//-------------------------------------------------------------------------------
	void* ModeTaskListenThreadNew::getParam()
	{
		return mPtr;
	}

	//-------------------------------------------------------------------------------
	Bool ModeTaskListenThreadNew::isRun()
	{
		return mIsRun;
	}

	//-------------------------------------------------------------------------------
	Ptr ModeTaskListenThreadNew::getHandle()
	{
		return mHandle;
	}
	//-------------------------------------------------------------------------------
	U32 ModeTaskListenThreadNew::getThreadId()
	{
		return mThreadId;	
	}
	//-------------------------------------------------------------------------------
	I32 ModeTaskListenThreadNew::addTask( ModeTask* task )
	{
		if (mDestory == 1)
		{
			return -1;
		}
		if (!task)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		mAllTaskCount++;
		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.find(task->mType);
		if (it != mTaskMapsForAdd.end())
		{
			DYNAMIC_ASSERT(it->second);
			it->second->addTask(task);
			return it->second->getTaskCount();
		}
		else
		{
			ModeTaskQueue* taskQueue = new ModeTaskQueue();
			taskQueue->addTask(task);
			mTaskMapsForAdd[task->mType] = taskQueue;

			mTaskQueueForTraversalCs.lock();
			mTaskQueueForTraversal.push(taskQueue);
			mTaskQueueForTraversalCs.unlock();
			return 0;
		}

		return mAllTaskCount;
	}

	//-------------------------------------------------------------------------------
	I32 ModeTaskListenThreadNew::getCurrTaskCount()
	{
		return mAllTaskCount;
	}

	//-------------------------------------------------------------------------------
	U32 API ModeTaskListenThreadNew::threadFunction( Ptr p )
	{
		try
		{
			ModeTaskListenThreadNew* modeListenThread = (ModeTaskListenThreadNew*)p; 
			if (modeListenThread)
			{
				ModeTask* modeTask = NULL;
				while(modeListenThread->mIsRun)
				{
					if (modeListenThread->getCurrTaskCount() < 1000)
					{
						modeListenThread->listenEvent();
					}				
					modeTask = modeListenThread->getTask();
					if (modeTask)
					{
						modeTask->process();
						modeTask->free();		
						
					}
					else
					{
						Sleep(1);
					}
				}
			}
		}
		catch (const runtime_error &e)
		{
			std::string what = "ModeTaskListenThreadNew 线程异常";
			what += e.what();
			DYNAMIC_ASSERT_LOG(false,what.c_str());
		}
		catch (...)
		{
			DYNAMIC_ASSERT_LOG(false,"ModeTaskListenThreadNew 线程异常");
		}
		return 0;
	}

	//-------------------------------------------------------------------------------
	ModeTask* ModeTaskListenThreadNew::getTask()
	{
		ModeTask* task = NULL;
		ModeTaskQueue* taskQueue = NULL;
		if (mTaskQueueForTraversal.empty())
		{
			return NULL;
		}
		mTaskQueueForTraversalCs.lock();
		I32 size = mTaskQueueForTraversal.size();
		for (I32 i = 0; i < size; i++)
		{
			taskQueue = mTaskQueueForTraversal.front();
			mTaskQueueForTraversal.pop();
			task = taskQueue->getTask();
			mTaskQueueForTraversal.push(taskQueue);
			if (task)
			{
				break;
			}
		}
		mTaskQueueForTraversalCs.unlock();
		if ( task )
		{
			mAllTaskCount--;
		}
		return task;
	}
	//-------------------------------------------------------------------------------
	Bool ModeTaskListenThreadNew::removeTask( U32 type,U32 key )
	{
		std::map<U32,ModeTaskQueue*>::iterator it = mTaskMapsForAdd.find(type);
		if (it != mTaskMapsForAdd.end())
		{
			DYNAMIC_ASSERT(it->second);
			return it->second->removeTask(type,key);
		}
		return false;
	}

}