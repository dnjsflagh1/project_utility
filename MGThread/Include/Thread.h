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
	//Thread 线程  线程池专用
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
		ThreadPool *getPool();									//获取线程所在的池对象

	private:
		static U32 API				threadProc( void* thd );
		Ptr							getHandle();				//获取线程的HANDLE句柄
		U32							getThreadID();				//获取线程的ID标志

	private:
		Bool			volatile	mFreeFlag;					//线程是否空闲标志
		Ptr							mHandle;					//线程的HANDLE句柄
		U32							mThreadId;					//线程的ID标志
		ThreadPool		*volatile	mThreadPool;				//池指针
		Thread			*volatile	mLast;						//队列指针
		Thread			*volatile	mNext;						//队列指针

	public:
		static void SetThreadName(U32 dwThreadID, Char8* szThreadName);
			
	};

	/******************************************************************************/
	//ThreadQueue 线程队列
	/******************************************************************************/
	class ThreadQueue
	{
		friend class ThreadPool;
	private:
		ThreadQueue();

		void addThread(Thread* thd);
		void delThread(Thread* thd);

	private:
		Thread 	*volatile	mThreadQueue;						//池所拥有的线程队列
		Critical			mLockPool;
		Critical			mLockUpdate;
	};

	/******************************************************************************/
	//单线程处理
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
		// 线程用到的外部数据
		Ptr mPtr;
		// 线程句柄
		Ptr mHandle;
		
		U32	mThreadId;

		// 线程是否运行
		volatile Bool mIsRun;
	};

	/******************************************************************************/
	//单线程处理
	/******************************************************************************/
	class ModeTask;
	class ModeTaskQueue;
	class ModeTaskThread
	{
	public:
		ModeTaskThread();
		~ModeTaskThread();

        // 创建线程
        // priority 线程优先级别: -1为默认
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
		// 线程用到的外部数据
		Ptr mPtr;
		// 线程句柄
		Ptr mHandle;
		// 线程是否运行
		volatile Bool mIsRun;
		
		U32 mThreadId;

		// 双队列一个用于外部添加，一个用于线程遍历
		std::map<U32,ModeTaskQueue*> mTaskMapsForAdd;
		Critical					 mTaskMapsForAddCs;
		std::queue<ModeTaskQueue*>	 mTaskQueueForTraversal;
		Critical					 mTaskQueueForTraversalCs;
		
		InterLocked					 mDestory;

	};

	/******************************************************************************/
	//监听任务线程
	/******************************************************************************/
	class ModeTaskListenThread
	{
	public:
		ModeTaskListenThread();
		virtual	~ModeTaskListenThread();

		// 创建线程
		// priority 线程优先级别: -1为默认
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
		// 线程用到的外部数据
		Ptr mPtr;
		// 线程句柄
		Ptr mHandle;
		// 线程是否运行
		volatile Bool mIsRun;

		U32	mThreadId;
		// 双队列一个用于外部添加，一个用于线程遍历
		std::map<U32,ModeTaskQueue*> mTaskMapsForAdd;
		Critical					 mTaskMapsForAddCs;
		std::queue<ModeTaskQueue*>	 mTaskQueueForTraversal;
		Critical					 mTaskQueueForTraversalCs;

		InterLocked					 mDestory;

	};

	/******************************************************************************/
	//线程监视器
	/******************************************************************************/
	struct ThreadInfo
	{
		Str8	threadName;			//线程名
		U32		threadId;			//线程ID
		Ptr		threadHandle;		//线程句柄
		Flt		threadCpu;			//线程cpu使用 %比
		Flt		threadKernelCpu;	//线程内核态cpu使用 %比
		Flt		threadUserCpu;		//线程用户态cpu使用 %比

		Int		threadKernelCpuTime;//线程内核态占用的cpu时间（s）
		Int		threadUserCpuTime;	//线程用户态占用的cpu时间（s）
		Int		threadCpuTime;		//线程占用的cpu时间（s）

		FILETIME createTime;		//线程创建时间
		FILETIME exitTime;			//线程结束时间

		FILETIME lastKernelTime;	//一个计算周期内花掉的内核态cpu时间
		FILETIME lastUserTime;		//一个计算周期内花掉的用户态cpu时间
		FILETIME kernelTime;		//一个计算周期内花掉的内核态cpu时间
		FILETIME userTime;			//一个计算周期内花掉的用户态cpu时间

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
		// 线程用到的外部数据
		Ptr mPtr;
		// 线程句柄
		Ptr mHandle;

		U32	mThreadId;

		// 线程是否运行
		volatile Bool mIsRun;
	};
	//----------------------------------------------------------
	class ModeTaskListenThreadNew
	{
	public:
		ModeTaskListenThreadNew();
		virtual	~ModeTaskListenThreadNew();

		// 创建线程
		// priority 线程优先级别: -1为默认
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
		// 线程用到的外部数据
		Ptr mPtr;
		// 线程句柄
		Ptr mHandle;
		// 线程是否运行
		volatile Bool mIsRun;

		U32	mThreadId;
		// 双队列一个用于外部添加，一个用于线程遍历
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

		// 计算频率
		Int								mCalcFrequency;	
		// 打印日志
		Bool							mIsPrintLog;
		// 监视开启
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