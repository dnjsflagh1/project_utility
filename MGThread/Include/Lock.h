/******************************************************************************/
#ifndef _LOCK_H_
#define _LOCK_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{

	/******************************************************************************/
	//Critical ������ 
	/******************************************************************************/
	class Critical 
	{ 
	public: 

		Critical(U32 spinCount = 4000); 
		virtual ~Critical();
		operator Bool()const;

    protected:

        //�����ٽ���
		void	create();
        //�����ٽ���
		void	destroy();
        
	public: 

        //����
		void	lock()const; 
        //���Լ���
		Bool    tryLock()const;
        //����
		void	unlock()const; 
	
	private: 

        //������ʾλ
		Bool						mCreate;
        //�����ʱ��Ƭ�γ���
		U32							mSpinCount;
        //�ٽ���
		mutable CRITICAL_SECTION	mCriticalSection; 
	}; 

	/******************************************************************************/
	//CriticalArmor �������Ļ����� 
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
	//Semaphore �ź��� ���Ķ���
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
		U32					mMaxCount;			//ֻ��һ����־��û�п����������Ĺ���
	};
	
	/******************************************************************************/
	//InterLocked  I32ԭ�Ӳ���
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
		I32				compareAssign(I32 comperand,I32 newVal);			//���ʱ��Ÿ�ֵ,The return value is the initial value
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
    //RWLock ��д��
    //�����ڶ���д�����
    //�ʺ�С��10���̵߳����
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