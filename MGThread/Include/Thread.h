/******************************************************************************/
#ifndef _THREAD_H_
#define _THREAD_H_
/******************************************************************************/
#include "Lock.h"

/******************************************************************************/
namespace MG
{
	
#define MS_VC_EXCEPTION 0x406d1388
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType;        // must be 0x1000
		LPCSTR szName;       // pointer to name (in same addr space)
		DWORD dwThreadID;    // thread ID (-1 caller thread)
		DWORD dwFlags;       // reserved for future use, most be zero
	} THREADNAME_INFO;



	/******************************************************************************/
	//Thread �߳�  �̳߳�ר��
	/******************************************************************************/
	class ThreadPool;
	class Thread 
	{
		friend class ThreadPool;
		friend class ThreadQueue;
		friend class Task;

	protected:
		Thread(ThreadPool *threadPool);
		~Thread();
		ThreadPool *getPool();									//��ȡ�߳����ڵĳض���

	private:
		static U32 API				threadProc( void* thd );
		Ptr							getHandle();				//��ȡ�̵߳�HANDLE���
		U32							getThreadID();				//��ȡ�̵߳�ID��־

	private:
		Bool			volatile	mFreeFlag;					//�߳��Ƿ���б�־
		Ptr							mHandle;					//�̵߳�HANDLE���
		U32							mThreadId;					//�̵߳�ID��־
		ThreadPool		*volatile	mThreadPool;				//��ָ��
		Thread			*volatile	mLast;						//����ָ��
		Thread			*volatile	mNext;						//����ָ��

	public:
		static void SetThreadName(U32 dwThreadID, Char8* szThreadName);
			
	};

	/******************************************************************************/
	//ThreadQueue �̶߳���
	/******************************************************************************/
	class ThreadQueue
	{
		friend class ThreadPool;
	private:
		ThreadQueue();

		void addThread(Thread* thd);
		void delThread(Thread* thd);

	private:
		Thread 	*volatile	mThreadQueue;						//����ӵ�е��̶߳���
		Critical			mLockPool;
		Critical			mLockUpdate;
	};

	/******************************************************************************/
	//���̴߳���
	/******************************************************************************/
	class OneThreadHandle
	{
	public:
		OneThreadHandle();
		~OneThreadHandle();

		Bool	create(void (*callback)(Ptr),Ptr ptr,Char8* threadName = "");
		Bool	create(void (*callback)(Ptr,U32),Ptr ptr,Char8* threadName = "");
		Bool	destory();
		void*	getParam();
		Bool	isRun();
		Ptr		getHandle();
		U32		getThreadId();

	protected:
		static U32 API threadFunction(Ptr p);

	private:
		void (*mCallback)(Ptr); 

		void (*mCallback1)(Ptr,U32); 
		// �߳��õ����ⲿ����
		Ptr mPtr;
		// �߳̾��
		Ptr mHandle;
		
		U32	mThreadId;

		// �߳��Ƿ�����
		volatile Bool mIsRun;
	};

	/******************************************************************************/
	//���̴߳���
	/******************************************************************************/
	class ModeTask;
	class ModeTaskQueue;
	class ModeTaskThread
	{
	public:
		ModeTaskThread();
		~ModeTaskThread();

        // �����߳�
        // priority �߳����ȼ���: -1ΪĬ��
		Bool		create(Ptr ptr, Char8* threadName,I32 maxTaskCount = 512, Int priority = -1);
		void		destory();

		void*		getParam();
		Bool		isRun();
		Ptr			getHandle();
		U32			getThreadId();

		I32			addTask(ModeTask* task);
		Bool		removeTask(U32 type,U32 key);
		ModeTask*	getTask();
		I32			getCurrTaskCount();

	protected:

		static U32 API threadFunction(Ptr p);

	private:
		// �߳��õ����ⲿ����
		Ptr mPtr;
		// �߳̾��
		Ptr mHandle;
		// �߳��Ƿ�����
		volatile Bool mIsRun;
		
		U32 mThreadId;

		// ˫����һ�������ⲿ��ӣ�һ�������̱߳���
		std::map<U32,ModeTaskQueue*> mTaskMapsForAdd;
		Critical					 mTaskMapsForAddCs;
		std::queue<ModeTaskQueue*>	 mTaskQueueForTraversal;
		Critical					 mTaskQueueForTraversalCs;
		
		InterLocked					 mDestory;

	};

	/******************************************************************************/
	//���������߳�
	/******************************************************************************/
	class ModeTaskListenThread
	{
	public:
		ModeTaskListenThread();
		virtual	~ModeTaskListenThread();

		// �����߳�
		// priority �߳����ȼ���: -1ΪĬ��
		Bool			create(Ptr ptr,Char8* threadName, I32 maxTaskCount = 512, Int priority = -1);
		void			destory();

		void*			getParam();
		Bool			isRun();
		Ptr				getHandle();
		U32				getThreadId();

		I32				addTask(ModeTask* task);
		Bool			removeTask(U32 type,U32 key);
		ModeTask*		getTask();
		I32				getCurrTaskCount();
		
		virtual	void	listenEvent() = 0;

	protected:
		static U32 API threadFunction(Ptr p);

	private:
		// �߳��õ����ⲿ����
		Ptr mPtr;
		// �߳̾��
		Ptr mHandle;
		// �߳��Ƿ�����
		volatile Bool mIsRun;

		U32	mThreadId;
		// ˫����һ�������ⲿ��ӣ�һ�������̱߳���
		std::map<U32,ModeTaskQueue*> mTaskMapsForAdd;
		Critical					 mTaskMapsForAddCs;
		std::queue<ModeTaskQueue*>	 mTaskQueueForTraversal;
		Critical					 mTaskQueueForTraversalCs;

		InterLocked					 mDestory;

	};

	/******************************************************************************/
	//�̼߳�����
	/******************************************************************************/
	struct ThreadInfo
	{
		Str8	threadName;			//�߳���
		U32		threadId;			//�߳�ID
		Ptr		threadHandle;		//�߳̾��
		Flt		threadCpu;			//�߳�cpuʹ�� %��
		Flt		threadKernelCpu;	//�߳��ں�̬cpuʹ�� %��
		Flt		threadUserCpu;		//�߳��û�̬cpuʹ�� %��

		Int		threadKernelCpuTime;//�߳��ں�̬ռ�õ�cpuʱ�䣨s��
		Int		threadUserCpuTime;	//�߳��û�̬ռ�õ�cpuʱ�䣨s��
		Int		threadCpuTime;		//�߳�ռ�õ�cpuʱ�䣨s��

		FILETIME createTime;		//�̴߳���ʱ��
		FILETIME exitTime;			//�߳̽���ʱ��

		FILETIME lastKernelTime;	//һ�����������ڻ������ں�̬cpuʱ��
		FILETIME lastUserTime;		//һ�����������ڻ������û�̬cpuʱ��
		FILETIME kernelTime;		//һ�����������ڻ������ں�̬cpuʱ��
		FILETIME userTime;			//һ�����������ڻ������û�̬cpuʱ��

		ThreadInfo()
		{
			threadName.reserve(128);			
			threadId = 0;			
			threadHandle = NULL;;		
			threadCpu = 0;			
			threadKernelCpu = 0;	
			threadUserCpu = 0;
			threadKernelCpuTime = 0;
			threadUserCpuTime = 0;	
			threadCpuTime = 0;

			createTime.dwHighDateTime = 0;
			exitTime.dwHighDateTime = 0;
			createTime.dwLowDateTime = 0;
			exitTime.dwLowDateTime = 0;	

			kernelTime.dwHighDateTime = 0;
			userTime.dwHighDateTime = 0;
			kernelTime.dwLowDateTime = 0;
			userTime.dwLowDateTime = 0;		

		}
	};
	//----------------------------------------------------------
	class OneThreadHandleNew
	{
	public:
		OneThreadHandleNew();
		~OneThreadHandleNew();

		Bool	create(void (*callback)(Ptr),Ptr ptr,Char8* threadName);
		Bool	create(void (*callback)(Ptr,U32),Ptr ptr,Char8* threadName);
		Bool	destory();
		void*	getParam();
		Bool	isRun();
		Ptr		getHandle();
		U32		getThreadId();

	protected:
		static U32 API threadFunction(Ptr p);

	private:
		void (*mCallback)(Ptr); 

		void (*mCallback1)(Ptr,U32); 
		// �߳��õ����ⲿ����
		Ptr mPtr;
		// �߳̾��
		Ptr mHandle;

		U32	mThreadId;

		// �߳��Ƿ�����
		volatile Bool mIsRun;
	};
	//----------------------------------------------------------
	class ModeTaskListenThreadNew
	{
	public:
		ModeTaskListenThreadNew();
		virtual	~ModeTaskListenThreadNew();

		// �����߳�
		// priority �߳����ȼ���: -1ΪĬ��
		Bool			create(Ptr ptr, Char8* threadName,I32 maxTaskCount = 512, Int priority = -1);
		void			destory();

		void*			getParam();
		Bool			isRun();
		Ptr				getHandle();
		U32				getThreadId();

		I32				addTask(ModeTask* task);
		Bool			removeTask(U32 type,U32 key);
		ModeTask*		getTask();
		I32				getCurrTaskCount();

		virtual	void	listenEvent() = 0;

	protected:
		static U32 API threadFunction(Ptr p);

	private:
		// �߳��õ����ⲿ����
		Ptr mPtr;
		// �߳̾��
		Ptr mHandle;
		// �߳��Ƿ�����
		volatile Bool mIsRun;

		U32	mThreadId;
		// ˫����һ�������ⲿ��ӣ�һ�������̱߳���
		std::map<U32,ModeTaskQueue*> mTaskMapsForAdd;
		Critical					 mTaskMapsForAddCs;
		std::queue<ModeTaskQueue*>	 mTaskQueueForTraversal;
		Critical					 mTaskQueueForTraversalCs;

		InterLocked					 mDestory;
		InterLocked					 mAllTaskCount;

	};
	//----------------------------------------------------------
	class ThreadMonitor
	{
		friend class ModeTaskThread;
		friend class ModeTaskListenThread;
		friend class ModeTaskListenThreadNew;
		friend class OneThreadHandle;
		friend class OneThreadHandleNew;
	public:
		ThreadMonitor();
		~ThreadMonitor();
		SINGLETON_INSTANCE(ThreadMonitor);
		Bool						initialize();
		void						unInitialize();
		void						setCalcFrequency(Int second);
		void						setPrintLog(Bool printLog);
		void						start();

	public:
		ThreadInfo*					getThreadInfobyName(Char8* name);
		ThreadInfo*					getThreadInfobyId(U32 threadId);
		std::map<U32,ThreadInfo*>*	getAllThreadInfo();
		void						lockThreadInfos();
		void						unlockThreadInfos();

	public:
		void						addThread(Str8 threadName,U32 threadId,Ptr threadHandle);
	
	private:
		static void 				threadFunction(Ptr p,U32 threadId);

	private:
		std::map<U32,ThreadInfo*>		mThreadInfosbyId;
		std::map<Str8,ThreadInfo*>		mThreadInfosbyName;
		OneThreadHandleNew				mMonitorThread;
		Critical						mLock;

		// ����Ƶ��
		Int								mCalcFrequency;	
		// ��ӡ��־
		Bool							mIsPrintLog;
		// ���ӿ���
		volatile Bool					mIsStart;
	};
#define SET_MONITOR_THREAD
#ifdef SET_MONITOR_THREAD
#define START_MONITOR_THREAD(tID,tName,tHandle) Str8 strThreadId;\
	Str8 strThreadName = tName;\
	strThreadName += '_';\
	MGStrOp::toString(tID,strThreadId);\
	strThreadName += strThreadId;\
	Thread::SetThreadName(tID,(Char8*)strThreadName.c_str());\
	ThreadMonitor::getInstance().addThread(strThreadName,tID,tHandle);
#else 
	#define START_MONITOR_THREAD(tID,tName,tHandle)
#endif // _DEBUG


}


#endif