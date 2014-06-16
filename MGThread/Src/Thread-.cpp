/******************************************************************************/
#include "stdafx.h"
#include "Thread.h"
#include "ThreadPool.h"
#include "Task.h"
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
    Bool OneThreadHandle::create(void (*callback)(Ptr),Ptr ptr)
    {
        destory();
        mCallback = callback;
        mPtr = ptr;
        mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			mIsRun = true;
			ResumeThread( mHandle );
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
	Bool OneThreadHandle::create(void (*callback)(Ptr,U32),Ptr ptr)
	{
		destory();
		mCallback1 = callback;
		mPtr = ptr;
		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			mIsRun = true;
			ResumeThread( mHandle );
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
		if(mHandle)
		{
			CloseHandle(mHandle);
			mHandle = NULL;
			Sleep(5);
		}
		mCallback = NULL;
		mCallback1 = NULL;
		mIsRun = false;
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
	MG::Bool ModeTaskThread::create( Ptr ptr, I32 maxTaskCount , Int priority  )
	{
		destory();

		mPtr = ptr;
     
        mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
			mIsRun = true;
			if (priority > 0)
			{
				SetThreadPriority( mHandle, priority );
			}
			mDestory = 0;
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
				Sleep(10);
				CloseHandle(mHandle);
				mHandle = NULL;
				Sleep(5);
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
	MG::Bool ModeTaskListenThread::create( Ptr ptr, I32 maxTaskCount , Int priority  )
	{
		destory();

		mPtr = ptr;
		//mTaskQueue = new ModeTaskQueue(maxTaskCount);
		mHandle = (Ptr)_beginthreadex( NULL, 0, threadFunction, (LPVOID)this, CREATE_SUSPENDED, &mThreadId );
		if (mHandle > 0)
		{
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
				Sleep(10);
				CloseHandle(mHandle);
				mHandle = NULL;
				Sleep(5);
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
		catch (...)
		{
			DYNAMIC_ASSERT_LOG(false,"OneThreadHandle 线程异常");
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
}