/******************************************************************************/
#ifndef _TASK_H_
#define _TASK_H_
/******************************************************************************/
#include "Lock.h"

/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//Task ��������� �̳߳�ר��
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
		Ptr				getHandle();					//��ȡ�����̵߳ľ��
		U32				getThreadID();					//��ȡ�����̵߳ı�־ID

	private:
		Thread			*volatile	mThread;
		TaskQueue		*volatile	mTaskQueue;
		Task			*volatile	mNextTask;
	};

	/******************************************************************************/
	//TaskQueue ������� �̳߳�ר��
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
	//ModeTask��������� TaskModeThreadר��
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
	//ModeTaskQueue ������� ModeTaskThread �� ModeTaskListenThreadר��
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

        // ����һ������
		Bool					addTask(ModeTask *task);

		// ɾ��һ������
		Bool					removeTask(U32 type,U32 key);

        // ��ȡһ�����񣬴˴��Ǵӵ�ǰ��������л�ã������ǰ����Ϊ�գ����ԭʼ������ȥ��ȡ
		ModeTask*				getTask();

        // �Ƴ���������
		void					removeAllTask();
		
        // ��ȡ��������
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