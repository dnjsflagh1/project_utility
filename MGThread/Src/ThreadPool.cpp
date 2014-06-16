/******************************************************************************/
#include "stdafx.h"
#include "ThreadPool.h"
#include "Task.h"
#include "Lock.h"
#include "Thread.h"

#pragma warning(disable:4127)
/******************************************************************************/

namespace MG
{

	//-----------------------------------------------------------------------------
	ThreadPool* ThreadPool::createPool( U32 idleThreadCount /*= 0*/,U32 maxThreadCount /*= 16*/, U32 maxTaskCount /*= 256 */,int priority /*= THREAD_PRIORITY_NORMAL*/ )
	{
		return new ThreadPool(idleThreadCount,maxThreadCount,maxTaskCount,priority);
	}
	//-----------------------------------------------------------------------------
	void ThreadPool::destroyPool()
	{
		mTaskExitFlag	= 1;
		mExitFlag		= 1;
		for(long waitCount =0;
			mThreadQueue && mThreadQueue->mThreadQueue && (waitCount < 40);
			waitCount++)
		{
			//等待超过10秒钟强行中断所有线程
			Sleep(100);//休息100毫秒钟
		}
		if(mThreadQueue)
		{
			mThreadQueue->mLockUpdate.lock();
			try{
				Thread	* thd;
				while(mThreadQueue && mThreadQueue->mThreadQueue)
				{
					thd = mThreadQueue->mThreadQueue;
					if(thd && thd->mHandle)
					{
						// 终止线程
						// ToDo 最好先挂起线程，然后终止
						TerminateThread(thd->mHandle,-1);
						mThreadQueue->delThread(thd);
						mCurr--;
						mFree -= thd->mFreeFlag ? 1 : 0;
						delete thd;
					}
				}
			}catch(...){}
			mThreadQueue->mLockUpdate.unlock();
		}
		delete this;
	}
	//-----------------------------------------------------------------------------
	void ThreadPool::addTask( Task *task )
	{
		mTaskQueue->addTask(task);
	}
	//-----------------------------------------------------------------------------
	I32 ThreadPool::getTaskCount()
	{
		return mTaskQueue->mTaskCount;
	}
	//-----------------------------------------------------------------------------
	ThreadPool::ThreadPool( I32 idleThreadCount,I32 maxThreadCount,I32 maxTaskCount,I32 priority )
		:mMax( (maxThreadCount < 1) ? 1 : maxThreadCount )
		,mIdle( (idleThreadCount < 1) ? 1 : ((idleThreadCount > mMax) ? mMax : idleThreadCount) )
		,mPriority(priority)
		,mCurr(0)
		,mFree(0)
		,mExitFlag(0)
		,mTaskExitFlag(0)
		,mTaskQueue(new TaskQueue(maxTaskCount))
		,mThreadQueue(new ThreadQueue())
	{
		//优先级
		if( mPriority != THREAD_PRIORITY_NORMAL &&
			mPriority != THREAD_PRIORITY_ABOVE_NORMAL &&
			mPriority != THREAD_PRIORITY_BELOW_NORMAL &&
			mPriority != THREAD_PRIORITY_HIGHEST &&
			mPriority != THREAD_PRIORITY_IDLE &&
			mPriority != THREAD_PRIORITY_LOWEST &&
			mPriority != THREAD_PRIORITY_TIME_CRITICAL)
		{
			*const_cast<I32*>(&mPriority) = THREAD_PRIORITY_NORMAL;
		}

		//线程队列初始化
		if(mTaskQueue == NULL || mThreadQueue == NULL)
		{
			delete mTaskQueue;
			mTaskQueue = NULL;
			delete mThreadQueue;
			mThreadQueue = NULL;
		}
		//分配线程
		mThreadQueue->mLockPool.lock();
		try
		{
			mThreadQueue->mLockUpdate.lock();
			try
			{
				Thread * newThread = NULL;
				I32 errCount1 = 0;
				I32	errCount2 = 0;
				for(int i = 0; i < mIdle; i++)
				{
					try
					{
						newThread = new Thread(this);
					}
					catch(...)
					{
						errCount1++;
						if(errCount1 >= 20)		//最多允许20次分配错误,延时1000毫秒(1秒)
							throw;
						Sleep(50);				//休息50毫秒再分配一次"
						i--;
						continue;
					}
					if(!newThread)
					{
						errCount2++;
						if(errCount2 >= 20)		//最多允许20次分配错误,延时1000毫秒(1秒)
						Sleep(50);				//休息50毫秒再分配一次"
						i--;
						continue;
					}
					mThreadQueue->addThread(newThread);
				}
			}
			catch(...)
			{
				mThreadQueue->mLockUpdate.unlock();
				throw;
			}
			mThreadQueue->mLockUpdate.unlock();
		}
		catch(...)
		{
			mThreadQueue->mLockPool.unlock();
			throw;
		}
		mThreadQueue->mLockPool.unlock();

	}
	//-----------------------------------------------------------------------------
	ThreadPool::~ThreadPool()
	{
		ThreadQueue* threadQueue= mThreadQueue;
		if(threadQueue)
		{
			mThreadQueue = NULL;
			delete threadQueue;
		}
		TaskQueue* taskQueue = mTaskQueue;
		if(taskQueue)
		{
			mTaskQueue = NULL;
			delete taskQueue;
		}
	}
	//-----------------------------------------------------------------------------
	void ThreadPool::threadExcute( Thread * thd)
	{
		Task* task = NULL;
		while(true)
		{
			//获得任务 管理线程
			mThreadQueue->mLockPool.lock();
			try
			{
				task = poolProcess(thd);
			}
			catch(...){}
			mThreadQueue->mLockPool.unlock();

			//任务存在 执行任务
			if(task)
			{
				try
				{
					
					//由于lasty接口删除 while( task = task->taskExec(thd)) ){}
					task->taskExec(thd);
				}
				catch(...)
				{
					DYNAMIC_ASSERT(false);
				}
#ifdef THREADPOOL_TEST
				printf("taskExec_ing 线程ID%d,\t线程数%d,\t空闲数%d,\t 当前任务数%d\n",thd->getThreadID(),thd->getPool()->_getCurr(),thd->getPool()->_getCurrFree(),thd->getPool()->getTaskCount());
#endif	
			}
			//池销毁 销毁当前线程
			else if(mExitFlag)
			{
				mThreadQueue->mLockUpdate.lock();
				try
				{
					if(thd->mFreeFlag)
					{
						thd->mFreeFlag	= false;
						mFree--;
						mCurr--;
						mThreadQueue->delThread(thd);
						CloseHandle(thd->mHandle);
						try
						{
							delete thd;
						}
						catch(...){}
					}
				}
				catch(...){}
				mThreadQueue->mLockUpdate.unlock();
				break;
			}
			//获取任务出错
			else
			{
				CloseHandle(thd->mHandle);
				delete thd;
				break;
			}

			//重置线程状态为空闲
			mThreadQueue->mLockUpdate.lock();
			try
			{
				thd->mFreeFlag = true;
				mFree ++;
			}
			catch(...){}
			mThreadQueue->mLockUpdate.unlock();

#ifdef THREADPOOL_TEST
			printf("after_taskExec线程ID%d,\t线程数%d,\t空闲数%d,\t 当前任务数%d\n",thd->getThreadID(),thd->getPool()->_getCurr(),thd->getPool()->_getCurrFree(),thd->getPool()->getTaskCount());
#endif

		}
	}
	//-----------------------------------------------------------------------------
	Task	* ThreadPool::poolProcess( Thread *thd )
	{
		//管理线程 获取任务
		Task* retTask = NULL;
		U32 waitTime = 0;
		
		while(true)
		{
			//获得阻塞时间 取得任务------------------------
			if ( retTask == NULL)
			{	
				//空闲线程越多 阻塞的时间可以越长
				if (mFree < mIdle && mCurr < mMax)
				{
					if (mFree < (I32)(0.618 * mIdle + 0.5))
					{
						waitTime = 0;
					}
					else
					{
						waitTime = 50;
					}
				}
				else
				{
					if (mFree == mIdle)
					{
						waitTime = 1000;
					}
					else
					{
						waitTime = 50;
					}
				}

				retTask = mTaskQueue->getTask(waitTime);
			}
			//如果池将要被销毁----------------------------------
			if (mTaskExitFlag)
			{
				if (retTask)
				{
					//销毁所有任务
					mTaskQueue->removeAllTask();
				}
				else if(mExitFlag)
				{
					//池销毁后退出
					break;
				}
				else
				{
					//等待池完全销毁
					Sleep(3);
				}
			} 
			//成功获取任务 && 池没有销毁-----------------------
			else if (retTask)
			{
				//更新线程信息
				mThreadQueue->mLockUpdate.lock();
				try
				{
					//线程数量不足时
					if (mFree <= 1 && mCurr < mMax)
					{
						Thread* newThread = new Thread(this);
						if(newThread)
						{
							mThreadQueue->addThread(newThread);
						}
					}
					mFree--;
					thd->mFreeFlag = false;
				}
				catch(...)
				{
					mThreadQueue->mLockUpdate.unlock();
					DYNAMIC_ASSERT(false);
					//LOGSYS
				}
				mThreadQueue->mLockUpdate.unlock();
				break;
			}
			//没有获取到任务（一般来说线程会比较空闲）------------
			else
			{
				mThreadQueue->mLockUpdate.lock();
				try
				{
					if(mFree > mIdle && waitTime >= 50 )
					{
						thd->mFreeFlag = false;
						mFree--;
						mCurr--;
						mThreadQueue->delThread(thd);
						mThreadQueue->mLockUpdate.unlock();
						break;
					}
					if ( mFree < mIdle && mCurr < mMax)
					{
						Thread* newThread = new Thread(this);
						if(newThread)
						{
							mThreadQueue->addThread(newThread);
						}
					}
				}catch(...)
				{
					mThreadQueue->mLockUpdate.unlock();
					DYNAMIC_ASSERT(false);
					//LOGSYS
				}
				mThreadQueue->mLockUpdate.unlock();
			}
		}

		return retTask;
	}
	//-----------------------------------------------------------------------------
}