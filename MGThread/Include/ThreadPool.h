/******************************************************************************/
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_
/******************************************************************************/
#include "Lock.h"
#define THREADPOOL_TEST
/******************************************************************************/
namespace MG
{
	class Task;
	class TaskManager;
	class TaskQueue;
	class ThreadQueue;

	class ThreadPool										//�̳߳ض���
	{
		friend class Thread;

	public:
		static ThreadPool* createPool(U32 idleThreadCount = 0,U32 maxThreadCount = 16, U32 maxTaskCount = 256 ,int priority = THREAD_PRIORITY_NORMAL);

		void destroyPool();					//ɾ��һ���̳߳� 
		void addTask(Task *task);			//׷��һ������
		//long waitForTask(Task *task);		//׷��һ���ȴ�������
		I32 getTaskCount();					//�õ��̳߳�������
		
		#ifdef THREADPOOL_TEST
		I32 _getCurr()			{return mCurr;}			//���ܼ��Ҫ��
		I32 _getCurrFree()		{return mFree;}		//���ܼ��Ҫ��
		#endif

	public:
		InterLocked							mTaskExitFlag;

	private:
		ThreadPool(I32 idleThreadCount,I32 maxThreadCount,I32 maxTaskCount,I32 priority);
		virtual ~ThreadPool();

		void threadExcute(Thread * thd);
		inline Task	* poolProcess(Thread *thd);

	private:
		InterLocked					mExitFlag;

		const		I32				mPriority;
		const		I32				mMax;
		const		I32				mIdle;
		volatile	I32				mCurr;
		volatile	I32				mFree;

		TaskQueue	*	volatile	mTaskQueue;
		ThreadQueue	*	volatile	mThreadQueue;

	private:
		ThreadPool(const ThreadPool& pool);
		ThreadPool& operator=(const ThreadPool& pool);

	};
}
#endif