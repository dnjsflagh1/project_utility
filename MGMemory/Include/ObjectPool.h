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
    // ����� ����
    /******************************************************************************/
    template<typename T, Bool _NeedGc = true>
    class ObjectPool
    {

    protected:

        // �����С
        UInt                m_TypeSize ;
        // �Ƿ�������������ڶ����ڴ��ͷ�ʱ��
        Bool                m_InvokeDestructionWhenFree;
        // ���󴴽�����
        Int                 m_MallocObjectCount;

        // �ⲿ���������
        Int                 m_ActiveObjectCount;
        // �ⲿ������б�
        std::map<T *,T *>   m_ActiveObjectList;

        // �ڲ�׼���ж����б�
        std::deque<T *>     m_IdleObjectList;

        // �Ƿ����쳣
        Bool                mCheckExcept;

        // �߳���
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


        // ȡ�ػ������������
        inline T*           retriveOrMallocObject();

        // ȡ�ض���
        inline T*           retriveObject();
        // �����µĶ����ڴ�ռ�
        inline T*           mallocObjectMemery();

        // ���ն���
        inline void         releaseObject( T* object, Bool isInvokeDestruction );     
        inline void         releaseAllObject( Bool isInvokeDestruction );

        // �Ƴ���Ķ���
        inline void         removeActiveObject(T* object);  
        // ���ӿ��еĶ���
        inline void         addIdleObject(T* object);    

    protected:

        // ���ӻ�Ķ���
        inline void         addActiveObject(T* object); 

        //���տ��пռ�  rate���ձ���  ��0.5��ʾ����һ�룬 1��ʾȫ�����ա�������ֵΪ������յĸ���
        inline int          reclaimIdelMem(double rate);

    public:

        //�̼߳���
        inline void threadLock()
        {
            if (m_Cs==NULL)
            {
                m_Cs = MG_NEW Critical();
            }
            m_Cs->lock();
        }

        //�߳̽���
        inline void threadUnLock()
        {
            m_Cs->unlock();
        }
    };

    //-----------------------------------------------------------------------------
    template<typename T, Bool _NeedGc>
    ObjectPool<T,_NeedGc>::~ObjectPool()
    {
        // �����Ҫ�������Զ������ⲿ����
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

        // ���տ��еĶ����б�
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

        // �ͷ��߳���
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

