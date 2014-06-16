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
			//�ȴ�����10����ǿ���ж������߳�
			Sleep(100);//��Ϣ100������
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
						// ��ֹ�߳�
						// ToDo ����ȹ����̣߳�Ȼ����ֹ
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
		//���ȼ�
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

		//�̶߳��г�ʼ��
		if(mTaskQueue == NULL || mThreadQueue == NULL)
		{
			delete mTaskQueue;
			mTaskQueue = NULL;
			delete mThreadQueue;
			mThreadQueue = NULL;
		}
		//�����߳�
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
						if(errCount1 >= 20)		//�������20�η������,��ʱ1000����(1��)
							throw;
						Sleep(50);				//��Ϣ50�����ٷ���һ��"
						i--;
						continue;
					}
					if(!newThread)
					{
						errCount2++;
						if(errCount2 >= 20)		//�������20�η������,��ʱ1000����(1��)
						Sleep(50);				//��Ϣ50�����ٷ���һ��"
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
			//������� �����߳�
			mThreadQueue->mLockPool.lock();
			try
			{
				task = poolProcess(thd);
			}
			catch(...){}
			mThreadQueue->mLockPool.unlock();

			//������� ִ������
			if(task)
			{
				try
				{
					
					//����lasty�ӿ�ɾ�� while( task = task->taskExec(thd)) ){}
					task->taskExec(thd);
				}
				catch(...)
				{
					DYNAMIC_ASSERT(false);
				}
#ifdef THREADPOOL_TEST
				printf("taskExec_ing �߳�ID%d,\t�߳���%d,\t������%d,\t ��ǰ������%d\n",thd->getThreadID(),thd->getPool()->_getCurr(),thd->getPool()->_getCurrFree(),thd->getPool()->getTaskCount());
#endif	
			}
			//������ ���ٵ�ǰ�߳�
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
			//��ȡ�������
			else
			{
				CloseHandle(thd->mHandle);
				delete thd;
				break;
			}

			//�����߳�״̬Ϊ����
			mThreadQueue->mLockUpdate.lock();
			try
			{
				thd->mFreeFlag = true;
				mFree ++;
			}
			catch(...){}
			mThreadQueue->mLockUpdate.unlock();

#ifdef THREADPOOL_TEST
			printf("after_taskExec�߳�ID%d,\t�߳���%d,\t������%d,\t ��ǰ������%d\n",thd->getThreadID(),thd->getPool()->_getCurr(),thd->getPool()->_getCurrFree(),thd->getPool()->getTaskCount());
#endif

		}
	}
	//-----------------------------------------------------------------------------
	Task	* ThreadPool::poolProcess( Thread *thd )
	{
		//�����߳� ��ȡ����
		Task* retTask = NULL;
		U32 waitTime = 0;
		
		while(true)
		{
			//�������ʱ�� ȡ������------------------------
			if ( retTask == NULL)
			{	
				//�����߳�Խ�� ������ʱ�����Խ��
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
			//����ؽ�Ҫ������----------------------------------
			if (mTaskExitFlag)
			{
				if (retTask)
				{
					//������������
					mTaskQueue->removeAllTask();
				}
				else if(mExitFlag)
				{
					//�����ٺ��˳�
					break;
				}
				else
				{
					//�ȴ�����ȫ����
					Sleep(3);
				}
			} 
			//�ɹ���ȡ���� && ��û������-----------------------
			else if (retTask)
			{
				//�����߳���Ϣ
				mThreadQueue->mLockUpdate.lock();
				try
				{
					//�߳���������ʱ
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
			//û�л�ȡ������һ����˵�̻߳�ȽϿ��У�------------
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