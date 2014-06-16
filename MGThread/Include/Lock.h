/******************************************************************************/
#ifndef _LOCK_H_
#define _LOCK_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{

	/******************************************************************************/
	//Critical 互斥锁 
	/******************************************************************************/
	class Critical 
	{ 
	public: 

		Critical(U32 spinCount = 4000); 
		virtual ~Critical();
		operator Bool()const;

    protected:

        //创建临界区
		void	create();
        //销毁临界区
		void	destroy();
        
	public: 

        //加锁
		void	lock()const; 
        //尝试加锁
		Bool    tryLock()const;
        //解锁
		void	unlock()const; 
	
	private: 

        //创建表示位
		Bool						mCreate;
        //检测锁时间片段长度
		U32							mSpinCount;
        //临界区
		mutable CRITICAL_SECTION	mCriticalSection; 
	}; 

	/******************************************************************************/
	//CriticalArmor 带保护的互斥锁 
	/******************************************************************************/
	class CriticalArmor
	{
	public:
		CriticalArmor(const Critical& critical);
		~CriticalArmor();
		operator Bool()const;

	public:
		void	lock()const;
		I32		trylock()const;
		void	unlock()const;

	private:
		mutable	U32				mLockNum;
		const	Critical&		mCritical;

	private:
		CriticalArmor(const CriticalArmor& armor);
		CriticalArmor& operator=(const CriticalArmor& armor);

	};

	/******************************************************************************/
	//Semaphore 信号量 核心对象
	/******************************************************************************/
	class Semaphore
	{
	public:
		Semaphore();	
		Semaphore(U32 initialCount,U32 maxCount,Char16* name = NULL);
		~Semaphore();
		operator Bool();

	public:		
		void			destroy();
		I32				create(I32 initialCount,I32 maxCount,Char16* name = NULL);

		U32				lock();
		U32				tryLock();
		U32				timeLock(U32 milliSeconds);
		I32				unlock(U32 relCount = 1);

		
	private:
		Ptr					mHamdle;			
		U32					mMaxCount;			//只是一个标志，没有控制最大计数的功能
	};
	
	/******************************************************************************/
	//InterLocked  I32原子操作
	/******************************************************************************/
	#pragma pack(push)
	#pragma pack(4)
	class InterLocked
	{
	public:
		InterLocked(I32 initVal = 0);
		InterLocked(const InterLocked &val);

		operator I32()const;
		I32 operator=(I32 val);			
		I32 operator=(InterLocked &val);
		I32 operator++();
		I32 operator++(int);
		I32 operator--();					
		I32 operator--(int);				
		I32 operator+=(I32 val);
		I32 operator-=(I32 val);
		I32 operator+=(InterLocked	&val);
		I32 operator-=(InterLocked	&val);

	public:
		I32				setZero();											//return original value
		I32				increment();										//The return value is the resulting incremented value
		I32				decrement();										//The return value is the resulting decremented value
		I32				add(I32 val);										//The return value is the initial value
		I32				assign(I32 newVal);									//The return value is the initial value
		I32				compareAssign(I32 comperand,I32 newVal);			//相等时候才赋值,The return value is the initial value
		static	Bool	suportAcquire();
	private:
		I32 volatile mValue;
		static	Char8		mSuportAcquire;

	};
	#pragma pack(pop)

	/******************************************************************************/
	//RefArmor 
	/******************************************************************************/
	class RefArmor
	{
	public:
		RefArmor(InterLocked &count);
		~RefArmor();

	private:
		InterLocked	&mCount;

	private:
		RefArmor(const RefArmor& );
		RefArmor& operator=(const RefArmor& );

	};
	
    /******************************************************************************/
    //RWLock 读写锁
    //适用于读多写少情况
    //适合小于10个线程的情况
    /******************************************************************************/
    class RWLocker
    {
    public:
        RWLocker();
        ~RWLocker();

        void    writeLock();
        void    writeUnLock();

        void    readLock();
        void    readUnLock();
        
    private:

        struct OneThreadLocker
        {
            DWORD       mThreadID;
            Critical    mThreadCs;
        };

        std::vector<OneThreadLocker*> mThreadLockList;
        Critical mThreadLockListCs;

        OneThreadLocker*    createOrRetrieveOneThreadLocker( UInt threadID );
        OneThreadLocker*    retrieveOneThreadLocker( UInt threadID );
        void			    destroyAllOneThreadLocker();

        void                lockAllThread();
        void                unLockAllThread();
    };



}
#endif