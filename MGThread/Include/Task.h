/******************************************************************************/
#ifndef _TASK_H_
#define _TASK_H_
/******************************************************************************/
#include "Lock.h"

/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//Task 任务抽象类 线程池专用
	/******************************************************************************/
	class Thread;
	class TaskQueue;
	class Task
	{													
		friend class ThreadPool;
		friend class TaskQueue;

	public:
		Task();		
		virtual			~Task();

		virtual	void	free() = 0;
		virtual	void	process() = 0;

	protected:
		Bool			getExitFlag();
		Task*			taskExec(Thread* thread);
		Ptr				getHandle();					//获取处理线程的句柄
		U32				getThreadID();					//获取处理线程的标志ID

	private:
		Thread			*volatile	mThread;
		TaskQueue		*volatile	mTaskQueue;
		Task			*volatile	mNextTask;
	};

	/******************************************************************************/
	//TaskQueue 任务队列 线程池专用
	/******************************************************************************/
	class TaskQueue
	{
		friend class ThreadPool;

	private:
		TaskQueue(I32 max);
		~TaskQueue();

	private:
		void					addTask(Task *task);
		Task*					getTask(U32 waitTime);
		void					removeAllTask();
		U32						getTaskCount();
		
	private:
		U32		const			mMaxSignalCount;
		U32		volatile		mTaskCount;
		Task	*volatile		mHead;
		Task	*volatile		mTail;
		Critical				mCriticalQueue;
		Semaphore				mSemaphoreAdd;
		Semaphore				mSemaphoreGet;

	private:
		TaskQueue(const TaskQueue&);
		TaskQueue& operator=(const TaskQueue&);
	};

	/******************************************************************************/
	//ModeTask任务抽象类 TaskModeThread专用
	/******************************************************************************/
	class ModeTask
	{													
		friend class ModeTaskQueue;
		friend class ModeTaskListenThread;
		friend class ModeTaskListenThreadNew;
		friend class ModeTaskThread;
	public:

		ModeTask(U32 type,U32 key = 0);		
		virtual			~ModeTask();
		virtual	void	    free() = 0;
		virtual	void	    process() = 0;
		void			    taskExec();
		void				reset(U32 type,U32 key = 0);

	private:

		ModeTaskQueue		*mTaskQueue;
		ModeTask			*mNextTask;
		U32		            mType;
		U32					mKey;
	};
	/******************************************************************************/
	//ModeTaskQueue 任务队列 ModeTaskThread 和 ModeTaskListenThread专用
	/******************************************************************************/
	class ModeTaskQueue
	{

		friend class ModeTaskThread;
		friend class ModeTaskListenThread;
		friend class ModeTaskListenThreadNew;

	private:
		ModeTaskQueue(I32 max = 0);
		~ModeTaskQueue();

	private:

        // 增加一个任务
		Bool					addTask(ModeTask *task);

		// 删除一个任务
		Bool					removeTask(U32 type,U32 key);

        // 获取一个任务，此处是从当前处理队列中获得，如果当前队列为空，则从原始队列中去获取
		ModeTask*				getTask();

        // 移除所有任务
		void					removeAllTask();
		
        // 获取任务数量
        U32						getTaskCount();

	private:

		U32						mPrepareTaskCount;
		U32						mMaxTaskCount;
		ModeTask*				mHead;
		ModeTask*				mTail;
		Critical				mCriticalQueue;
        std::queue<ModeTask*>   mCurrHandleTaskList;
        
	private:

		ModeTaskQueue(const TaskQueue&);
		ModeTaskQueue& operator=(const TaskQueue&);
	};

}
#endif