/******************************************************************************/
#ifndef __OBJECTPOOL_H__
#define __OBJECTPOOL_H__
/******************************************************************************/

using namespace std;
#include "Lock.h"
#include "MGMemoryDefine.h"

/******************************************************************************/
namespace MG
{
    #pragma warning(push)
    #pragma warning(disable: 4127)


    #define OP_MALLOC_CATEGORY 2004

    /******************************************************************************/
    // 对象池 操作
    /******************************************************************************/
    template<typename T, Bool _NeedGc = true>
    class ObjectPool
    {

    protected:

        // 对象大小
        UInt                m_TypeSize ;
        // 是否调用析构函数在对象内存释放时候
        Bool                m_InvokeDestructionWhenFree;
        // 对象创建数量
        Int                 m_MallocObjectCount;

        // 外部活动对象数量
        Int                 m_ActiveObjectCount;
        // 外部活动对象列表
        std::map<T *,T *>   m_ActiveObjectList;

        // 内部准备中对象列表
        std::deque<T *>     m_IdleObjectList;

        // 是否检测异常
        Bool                mCheckExcept;

        // 线程锁
        Critical*           m_Cs;

    public:

        ObjectPool()
            :m_InvokeDestructionWhenFree(false)
            ,m_MallocObjectCount(0)
            ,m_ActiveObjectCount(0)
            ,mCheckExcept(true)
            ,m_Cs(NULL)
            ,m_TypeSize(sizeof(T))
        {

        }

        virtual ~ObjectPool(void);

        //////////////////////////////////////////////////////////////////////////////////

        inline void        setCheckExceptEnable( Bool enable ){mCheckExcept = enable;}
        inline unsigned    getSizeOfType(){return sizeof(T);}
        inline bool        idleEmpty(){return m_IdleObjectList.empty();}
        inline int         idleCount(){return m_IdleObjectList.size();}
        inline int         activeCount(){return m_ActiveObjectCount;}
        inline int         mallocCount(){return m_MallocObjectCount;}

        inline std::map<T *,T *>&   
                           getActiveObjectList(){ return m_ActiveObjectList; }

        //////////////////////////////////////////////////////////////////////////////////


        // 取回或重新申请对象
        inline T*           retriveOrMallocObject();

        // 取回对象
        inline T*           retriveObject();
        // 申请新的对象内存空间
        inline T*           mallocObjectMemery();

        // 回收对象
        inline void         releaseObject( T* object, Bool isInvokeDestruction );     
        inline void         releaseAllObject( Bool isInvokeDestruction );

        // 移除活动的对象
        inline void         removeActiveObject(T* object);  
        // 增加空闲的对象
        inline void         addIdleObject(T* object);    

    protected:

        // 增加活动的对象
        inline void         addActiveObject(T* object); 

        //回收空闲空间  rate回收比例  如0.5表示回收一半， 1表示全部回收。。返回值为具体回收的个数
        inline int          reclaimIdelMem(double rate);

    public:

        //线程加锁
        inline void threadLock()
        {
            if (m_Cs==NULL)
            {
                m_Cs = MG_NEW Critical();
            }
            m_Cs->lock();
        }

        //线程解锁
        inline void threadUnLock()
        {
            m_Cs->unlock();
        }
    };

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    ObjectPool<T,_NeedGc>::~ObjectPool()
    {
        // 如果需要回收则自动回收外部对象
        if ( _NeedGc )
        {
            std::map<T *,T *>::iterator ite;
            for(ite=m_ActiveObjectList.begin(); ite!=m_ActiveObjectList.end(); ++ite)
            {
                ite->second->~T();
                MG_POOL_FREE(ite->second,OP_MALLOC_CATEGORY);
                m_MallocObjectCount--;
                m_ActiveObjectCount--;
            }
            m_ActiveObjectList.clear();
        }

        // 回收空闲的对象列表
        std::deque<T *>::iterator ite;
        for(ite=m_IdleObjectList.begin(); ite!=m_IdleObjectList.end(); ++ite)
        {
            if ( m_InvokeDestructionWhenFree )
            {
                (*ite)->~T();
            }
            MG_POOL_FREE((*ite),OP_MALLOC_CATEGORY);
            m_MallocObjectCount--;
        }
        m_IdleObjectList.clear();

        // 释放线程锁
        if (m_Cs)
        {
            MG_SAFE_DELETE( m_Cs ) ;
            m_Cs = NULL;
        }

        if (mCheckExcept)
        {
			if (m_MallocObjectCount != 0)
			{
				MessageBoxA(NULL,"m_MallocObjectCount != 0","error",MB_OK);
			}
			
			DYNAMIC_ASSERT_LOG( m_MallocObjectCount == 0, "error : ObjectPool<T,_NeedGc>::~ObjectPool(), m_MallocObjectCount == 0" );
        }
    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    T* ObjectPool<T, _NeedGc>::retriveOrMallocObject()
    {
        T* pObject = NULL;

		if ( idleEmpty() )
		{
            pObject = mallocObjectMemery();
		}else
		{
			pObject = retriveObject();
		}

        return pObject;
    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    T* ObjectPool<T, _NeedGc>::retriveObject()
    {
        T* pObject = NULL;

		DYNAMIC_ASSERT_LOG( m_IdleObjectList.empty() == false , "error : T* ObjectPool<T, _NeedGc>::retriveObject(), m_IdleObjectList.empty() == false" );

		pObject = m_IdleObjectList.back();
		m_IdleObjectList.pop_back();
        addActiveObject( pObject );

        return pObject;
    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    T* ObjectPool<T, _NeedGc>::mallocObjectMemery()
    {
        T* pObject = (T*)MG_POOL_MALLOC( m_TypeSize, OP_MALLOC_CATEGORY );
        m_MallocObjectCount++;

        addActiveObject( pObject );

        return pObject;
    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    void ObjectPool<T, _NeedGc>::releaseObject( T* object, Bool isInvokeDestruction )    
    {
        m_InvokeDestructionWhenFree = !isInvokeDestruction;
        
        if (isInvokeDestruction)
            object->~T();

        removeActiveObject( object );

        addIdleObject( object );
    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    void ObjectPool<T, _NeedGc>::releaseAllObject(  Bool isInvokeDestruction )    
    {
		DYNAMIC_ASSERT_LOG( _NeedGc, "error : void ObjectPool<T, _NeedGc>::releaseAllObject(  Bool isInvokeDestruction ),_NeedGc" );

        if ( _NeedGc )
        {
            std::map<T *,T *>::iterator ite;
            T * pObject = NULL;
            for(ite=m_ActiveObjectList.begin(); ite!=m_ActiveObjectList.end(); ++ite)
            {
                pObject = ite->second;

                if (!m_InvokeDestructionWhenFree)
                    pObject->~T();

                m_ActiveObjectCount--;
                m_MallocObjectCount--;

                addIdleObject( pObject );
            }
            m_ActiveObjectList.clear();
        }
    }
    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    void ObjectPool<T, _NeedGc>::addActiveObject(T* object)    
    {
        if ( _NeedGc )
        {
			DYNAMIC_ASSERT_LOG( object != NULL, "error : void ObjectPool<T, _NeedGc>::addActiveObject(T* object)   ,object != NULL" );
            m_ActiveObjectList[object] = object;
        }
        m_ActiveObjectCount++;
    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    void ObjectPool<T, _NeedGc>::removeActiveObject( T* object )    
    {
        if ( _NeedGc )
        {
            std::map<T *,T *>::iterator ite;
            ite=m_ActiveObjectList.find( object );
            if ( ite != m_ActiveObjectList.end() )
            {
                m_ActiveObjectList.erase(ite);
            }
        }
        m_ActiveObjectCount--;
        m_MallocObjectCount--;

#ifdef _DEBUG
        if ( mCheckExcept )
        {

			if (m_MallocObjectCount < 0)
			{
				MessageBoxA(NULL,"m_ActiveObjectCount < 0","error",MB_OK);
			}
			DYNAMIC_ASSERT_LOG( m_ActiveObjectCount >= 0, "error : void ObjectPool<T, _NeedGc>::removeActiveObject( T* object )  ,m_ActiveObjectCount >= 0" );
        }
#endif

    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    void ObjectPool<T, _NeedGc>::addIdleObject( T* object )    
    {

		DYNAMIC_ASSERT_LOG( object != NULL, "error : void ObjectPool<T, _NeedGc>::addIdleObject( T* object )  ,object != NULL" );

#ifdef _DEBUG
        if ( mCheckExcept )
        {
            std::deque<T *>::iterator ite;
            for(ite=m_IdleObjectList.begin(); ite!=m_IdleObjectList.end(); ++ite)
            {
                if ( (*ite) == object )
                {
					DYNAMIC_ASSERT_LOG( false, "error : void ObjectPool<T, _NeedGc>::addIdleObject( T* object ) ,(*ite) == object  false" );
                }
            }
        }
#endif

        m_IdleObjectList.push_back( object );
        m_MallocObjectCount++;
    }

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    int ObjectPool<T, _NeedGc>::reclaimIdelMem(double rate)
    {
		DYNAMIC_ASSERT_LOG( false, "error : int ObjectPool<T, _NeedGc>::reclaimIdelMem(double rate) , false" );
        //if(0<rate && rate<=1)
        //{
        //    int size = m_IdleObjectList.size();

        //    size = (int)(size * rate);

        //    for(int idx=0; idx<size; ++idx)
        //    {
        //        //TODO:

        //    }

        //    return size;
        //}

        return 0;
    }

    #pragma warning(pop)

}
/******************************************************************************/
#endif

