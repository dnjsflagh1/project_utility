/******************************************************************************/
#include "stdafx.h"
#include "Task.h"
#include "Thread.h"
#include "ThreadPool.h"
/******************************************************************************/

namespace MG
{
	//-----------------------------------------------------------------------------
	Task::Task()
	{
		mThread = NULL;
		this->mNextTask = NULL;
		this->mTaskQueue = NULL;
	}
	//-----------------------------------------------------------------------------
	Bool Task::getExitFlag()
	{
		if (mThread)
		{
			if(((I32)mThread->mThreadPool->mTaskExitFlag) != 0)
			{
				return true;
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	Task::~Task()
	{
		
	}
	//-----------------------------------------------------------------------------
	Ptr Task::getHandle()
	{
		if (mThread)
		{
			return mThread->getHandle();
		}
		return NULL;
	}
	//-----------------------------------------------------------------------------
	U32 Task::getThreadID()
	{
		if (mThread)
		{
			return mThread->getThreadID();
		}
		return 0;
	}
	//-----------------------------------------------------------------------------
	Task* Task::taskExec( Thread* thd )
	{
		//Task* nextTask = mNextTask;

		mThread = thd;
		process();
		mThread = NULL;

		free();

		return NULL;
	}

	//=================================================================================
	TaskQueue::TaskQueue( I32 max ):mMaxSignalCount(max),mTaskCount(0),mHead(NULL),mTail(NULL)
	{
		mCriticalQueue.lock();
		try
		{
			//��ʼʱ���Դ�������
			mSemaphoreAdd.create(0,mMaxSignalCount,NULL);
			//��ʼ������ʱ������ �����Ի�ȡ
			mSemaphoreGet.create(mMaxSignalCount,mMaxSignalCount,NULL);
			if(!mCriticalQueue || !mSemaphoreAdd || !mSemaphoreGet)
			{
				DYNAMIC_ASSERT(false);
				throw;
			}
		}
		catch(...)
		{
			mCriticalQueue.unlock();
			DYNAMIC_ASSERT(false);
			throw;
		}
		mCriticalQueue.unlock();
	}
	//-----------------------------------------------------------------------------
	TaskQueue::~TaskQueue()
	{
		removeAllTask();
	}
	//-----------------------------------------------------------------------------
	void TaskQueue::removeAllTask()
	{
		Task* task = NULL;
		mCriticalQueue.lock();
		while(mHead)
		{
			task = mHead;
			mHead = mHead->mNextTask;
			task->mNextTask = NULL;
			task->free();
		}
		mCriticalQueue.unlock();
	}
	//-----------------------------------------------------------------------------
	void TaskQueue::addTask( Task *task )
	{
		if (!task)
		{
			return;
		}
		//...

		mSemaphoreGet.lock();
		mCriticalQueue.lock();
		try
		{
			if(task->mTaskQueue == NULL)
			{
				task->mTaskQueue = this;
				task->mNextTask	= 0;
				if(mTail)
				{
					mTail->mNextTask = task;
					mTail = task;
				}else
				{
					mHead = task;
					mTail = task;
				}
				mTaskCount++;
				mSemaphoreAdd.unlock();
			}
		}
		catch(...)
		{
			DYNAMIC_ASSERT(false);
		}
		mCriticalQueue.unlock();
	}
	//-----------------------------------------------------------------------------
	Task* TaskQueue::getTask( U32 waitTime )
	{
		Task * task = NULL;
		//�߳�����Խ�� �ȴ���ʱ��Ӧ��Խ��
		//�����߳�������ȡ����ʱ ������������߳���������
		if(mSemaphoreAdd.timeLock(waitTime) == WAIT_OBJECT_0)//����
		{
			mCriticalQueue.lock();
			try
			{
				if(mHead ==	mTail)
				{
					task = mHead;
					mHead = NULL;
					mTail = NULL;
				}
				else
				{
					task = mHead;
					mHead = mHead->mNextTask;
				}
				if(task)
				{
					task->mTaskQueue = NULL;
					mTaskCount--;
					mSemaphoreGet.unlock();
				}
			}
			catch(...)
			{
				DYNAMIC_ASSERT(false);
			}
			mCriticalQueue.unlock();
		}

		return task;
	}
	//-----------------------------------------------------------------------------
	U32 TaskQueue::getTaskCount()
	{
		return mTaskCount;
	}

	//=================================================================================
	ModeTask::ModeTask(U32 type, U32 key )
	{
		mTaskQueue  = NULL;
		mNextTask   = NULL;
		mKey        = key;
		mType       = type;
	}
	//-----------------------------------------------------------------------------
	void ModeTask::taskExec()
	{
		process();
		free();
	}
	//-----------------------------------------------------------------------------
	ModeTask::~ModeTask()
	{
		
	}
	//-----------------------------------------------------------------------------
	void ModeTask::reset( U32 type,U32 key /*= 0*/ )
	{
		mTaskQueue = NULL;
		mNextTask = NULL;
		mType = type;
		mKey = key;
	}
	//-----------------------------------------------------------------------------
	Bool ModeTaskQueue::addTask( ModeTask *task )
	{
		if (!task)
		{
			DYNAMIC_ASSERT(false);
			return false;
		}

		mCriticalQueue.lock();
		try
		{
			// ��׼����Task�����ֵ����ͬ�����滻����
			if (mPrepareTaskCount >= mMaxTaskCount && mMaxTaskCount != 0)
			{
				
				if (mHead->mKey == task->mKey)
				{
					mTail->mNextTask	= task;
					mTail				= task;
					mTail->mNextTask	= NULL;
					task->mTaskQueue	= this;

					ModeTask* tempTask	= mHead;
					mHead	= mHead->mNextTask;
					tempTask->free();
				}
				else
				{	ModeTask* lastTask = mHead;
					ModeTask* tempTask = lastTask->mNextTask;
					while ( (tempTask != NULL) && (tempTask->mKey != task->mKey) )
					{
						lastTask = tempTask;
						tempTask = lastTask->mNextTask;
					}
					if (tempTask == NULL)
					{
						task->free();
						mCriticalQueue.unlock();
						return false;
					}
					else
					{
						mTail->mNextTask	= task;
						mTail				= task;
						mTail->mNextTask	= NULL;
						task->mTaskQueue	= this;
						
						lastTask->mNextTask = tempTask->mNextTask;
						tempTask->free();
						mCriticalQueue.unlock();
						return true;
					}
				}
			}
			else
			{
				if(task->mTaskQueue == NULL)
				{
					task->mTaskQueue = this;
					task->mNextTask	= NULL;
					if(mTail)
					{
						mTail->mNextTask = task;
						mTail = task;
					}else
					{
						mHead = task;
						mTail = task;
					}
					mPrepareTaskCount++;
				}
				else
				{
					DYNAMIC_ASSERT(false);
				}
			}

		}
		catch(...)
		{
			DYNAMIC_ASSERT(false);
			mCriticalQueue.unlock();
			return false;
			
		}
		mCriticalQueue.unlock();
		return true;
	}

    //-----------------------------------------------------------------------------
	ModeTask* ModeTaskQueue::getTask()
	{
        ModeTask* task = NULL;
        
        // �����ǰ�б�Ϊ�����ԭʼ��������ȥ��ȡ    
        if ( mCurrHandleTaskList.empty() )
        {
            mCriticalQueue.lock();
            {
                ModeTask* tempTask = NULL;

                while(mHead)
                {
                    tempTask = mHead;
                    mCurrHandleTaskList.push( tempTask );
                    mHead = mHead->mNextTask;
                    tempTask->mNextTask = NULL;
                }

                mPrepareTaskCount = 0;

                mTail = NULL;
            }
			
            mCriticalQueue.unlock();
        }

        // �ӵ�ǰ�б��е���һ������
        if ( mCurrHandleTaskList.empty() == false )
        {
            task = mCurrHandleTaskList.front();
            mCurrHandleTaskList.pop();
        }
		return task;
	}

    //-----------------------------------------------------------------------------
	void ModeTaskQueue::removeAllTask()
	{
		ModeTask* task = NULL;

        while( task = getTask() )
        {
            task->free();
        }
	}

    //-----------------------------------------------------------------------------
	U32 ModeTaskQueue::getTaskCount()
	{
		return mPrepareTaskCount;
	}

    //-----------------------------------------------------------------------------
	ModeTaskQueue::ModeTaskQueue( I32 max )
	{
		mMaxTaskCount = max;
		mPrepareTaskCount = 0;
		mHead = NULL;
		mTail = NULL;
	}

    //-----------------------------------------------------------------------------
	ModeTaskQueue::~ModeTaskQueue()
	{
        removeAllTask();
	}

	Bool ModeTaskQueue::removeTask( U32 type,U32 key )
	{
		
		return false;
	}
	//-----------------------------------------------------------------------------
}