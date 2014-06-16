/******************************************************************************/
#ifndef _DATASETMANAGER_H
#define _DATASETMANAGER_H
/******************************************************************************/

#include "Lock.h"
#include "ObjectPool.h"

/******************************************************************************/
namespace MG
{
    
    #define SMDS_CREATEORRETRIEVE(method_para,new_para) \
        TYPE_CLASS* createOrRetrieve##method_para \
        { \
            TYPE_CLASS * object = NULL; \
            if (isLock) lock(); \
            { \
                object = getData( key, false ); \
                if ( object == NULL ) \
                { \
                    object = MG_POOL_NEW(mObjectPool,TYPE_CLASS, new_para); \
                    mDataList[key] = object; \
                } \
            } \
            if (isLock) unLock(); \
            return object; \
        }; \

    /******************************************************************************/
    // 简单的数据映射管理
    /******************************************************************************/
    template<typename TYPE_KEY, typename TYPE_CLASS>
    class SimpleMapDataSetManager
    {
    public:
        typedef std::map<TYPE_KEY, TYPE_CLASS*> DataSetMap;
		typedef typename std::map<TYPE_KEY, TYPE_CLASS*>::iterator DataSetMapIter;

        void					lock(){mThreadLock.lock();}
        void					unLock(){mThreadLock.unlock();}
        
        SMDS_CREATEORRETRIEVE( (TYPE_KEY key, Bool isLock=true),() )

        template<typename CONSTRUCTION_1>
        SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, Bool isLock=true),(c1) )

        template<typename CONSTRUCTION_1,typename CONSTRUCTION_2>
        SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, Bool isLock=true),(c1,c2) )

        template<typename CONSTRUCTION_1,typename CONSTRUCTION_2,typename CONSTRUCTION_3>
        SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, CONSTRUCTION_3 c3, Bool isLock=true),(c1,c2,c3) )

        template<typename CONSTRUCTION_1,typename CONSTRUCTION_2,typename CONSTRUCTION_3,typename CONSTRUCTION_4>
        SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, CONSTRUCTION_3 c3, CONSTRUCTION_4 c4, Bool isLock=true),(c1,c2,c3,c4) )

		template<typename CONSTRUCTION_1,typename CONSTRUCTION_2,typename CONSTRUCTION_3,typename CONSTRUCTION_4,typename CONSTRUCTION_5>
		SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, CONSTRUCTION_3 c3, CONSTRUCTION_4 c4, CONSTRUCTION_5 c5, Bool isLock=true),(c1,c2,c3,c4,c5) )
		
		template<typename CONSTRUCTION_1,typename CONSTRUCTION_2,typename CONSTRUCTION_3,typename CONSTRUCTION_4,typename CONSTRUCTION_5, typename CONSTRUCTION_6>
		SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, CONSTRUCTION_3 c3, CONSTRUCTION_4 c4, CONSTRUCTION_5 c5, CONSTRUCTION_6 c6,Bool isLock=true),(c1,c2,c3,c4,c5,c6) )

		template<typename CONSTRUCTION_1,typename CONSTRUCTION_2,typename CONSTRUCTION_3,typename CONSTRUCTION_4,typename CONSTRUCTION_5, typename CONSTRUCTION_6, typename CONSTRUCTION_7>
		SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, CONSTRUCTION_3 c3, CONSTRUCTION_4 c4, CONSTRUCTION_5 c5, typename CONSTRUCTION_6 c6, CONSTRUCTION_7 c7,Bool isLock=true),(c1,c2,c3,c4,c5,c6,c7) )

		template<typename CONSTRUCTION_1,typename CONSTRUCTION_2,typename CONSTRUCTION_3,typename CONSTRUCTION_4,typename CONSTRUCTION_5, typename CONSTRUCTION_6, typename CONSTRUCTION_7, typename CONSTRUCTION_8>
		SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, CONSTRUCTION_3 c3, CONSTRUCTION_4 c4, CONSTRUCTION_5 c5, CONSTRUCTION_6 c6, CONSTRUCTION_7 c7, CONSTRUCTION_8 c8,Bool isLock=true),(c1,c2,c3,c4,c5,c6,c7,c8) )

		template<typename CONSTRUCTION_1,typename CONSTRUCTION_2,typename CONSTRUCTION_3,typename CONSTRUCTION_4,typename CONSTRUCTION_5, typename CONSTRUCTION_6, typename CONSTRUCTION_7, typename CONSTRUCTION_8, typename CONSTRUCTION_9>
		SMDS_CREATEORRETRIEVE( (TYPE_KEY key, CONSTRUCTION_1 c1, CONSTRUCTION_2 c2, CONSTRUCTION_3 c3, CONSTRUCTION_4 c4, CONSTRUCTION_5 c5, CONSTRUCTION_6 c6, CONSTRUCTION_7 c7, CONSTRUCTION_8 c8, CONSTRUCTION_9 c9, Bool isLock=true),(c1,c2,c3,c4,c5,c6,c7,c8,c9) )
        void					addOrUpdate(TYPE_KEY key, TYPE_CLASS* object, Bool isLock=true) ; 
		Bool					empty();
        Bool					hasData(TYPE_KEY key, Bool isLock=true) ; 
        TYPE_CLASS*				getData(TYPE_KEY key, Bool isLock=true) ; 
		TYPE_CLASS*				popData(TYPE_KEY key, Bool isLeavePool=false, Bool isLock=true) ; 
		
		typename std::map<TYPE_KEY, TYPE_CLASS*>::iterator	
								destroy(TYPE_KEY key, Bool isLock=true);
							
		//void					destroy(TYPE_CLASS* object, Bool isLock=true);
		
		void					destroy(DataSetMapIter it, Bool isLock=true);

        void					destroyAll(Bool isLock=true) ;
        DataSetMap*				getDataSet(){ return &mDataList; } ;

    protected:
        Critical                    mThreadLock;
        ObjectPool<TYPE_CLASS>      mObjectPool;
        DataSetMap                  mDataList;
    };



    /******************************************************************************/
    // 多数据映射管理
    /******************************************************************************/
    template<typename TYPE_KEY0, typename TYPE_KEY1, typename TYPE_CLASS>
    class MultiMapDataSetManager
    {
    public:
        
    };


    /******************************************************************************/
    // SimpleMapDataSetManager Definition
    /******************************************************************************/
    template<typename TYPE_KEY, typename TYPE_CLASS>
    void SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::addOrUpdate(TYPE_KEY key, TYPE_CLASS* object, Bool isLock)
    {
        if (isLock) lock();
        {
			//if ( getData(key) != object )
			{
				destroy(key, false);
				mDataList[key] = object;
				mObjectPool.addObject( object );
			}
        }
        if (isLock) unLock();
    }

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	Bool SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::empty()
	{
		return mObjectPool.getActiveObjectList().empty();
	}

    //-----------------------------------------------------------------------------
    template<typename TYPE_KEY, typename TYPE_CLASS>
    Bool SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::hasData(TYPE_KEY key, Bool isLock)
    {
        Bool result = false;
        if (isLock) lock();
        {
            SimpleMapDataSetManager::DataSetMap::iterator it = mDataList.find(key);
            if ( it != mDataList.end() )
            {
                result = true;
            }
        }
        if (isLock) unLock();
        return result;
    }

    //-----------------------------------------------------------------------------
    template<typename TYPE_KEY, typename TYPE_CLASS>
    TYPE_CLASS* SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::getData(TYPE_KEY key, Bool isLock)
    {
        TYPE_CLASS* result = NULL;
        if (isLock) lock();
        {
            SimpleMapDataSetManager::DataSetMap::iterator it = mDataList.find(key);
            if ( it != mDataList.end() )
            {
                result = it->second;
            }
        }
        if (isLock) unLock();
        return result;
    }

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	TYPE_CLASS* SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::popData(TYPE_KEY key, Bool isLeavePool, Bool isLock)
	{
		TYPE_CLASS* result = NULL;
		if (isLock) lock();
		{
			SimpleMapDataSetManager::DataSetMap::iterator it = mDataList.find(key);
			if ( it != mDataList.end() )
			{
				result = it->second;
				mDataList.erase(it);
                if ( isLeavePool )
                {
                    mObjectPool.removeActiveObject( result );
                }
			}
		}
		if (isLock) unLock();
		return result;
	}

    //-----------------------------------------------------------------------------
    template<typename TYPE_KEY, typename TYPE_CLASS>
	typename std::map<TYPE_KEY, TYPE_CLASS*>::iterator 
		SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::destroy(TYPE_KEY key, Bool isLock)
    {
		std::map<TYPE_KEY, TYPE_CLASS*>::iterator it = mDataList.find(key);
        if (isLock) lock();
        {
            
            if ( it != mDataList.end() )
            {
                MG_POOL_DELETE(mObjectPool,it->second);
                it = mDataList.erase(it);
            }  
        }
        if (isLock) unLock();
		return it;
    }

	////-----------------------------------------------------------------------------
	//template<typename TYPE_KEY, typename TYPE_CLASS>
	//void SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::destroy(TYPE_CLASS* object, Bool isLock)
	//{
	//	if (isLock) lock();
	//	{
	//		MG_POOL_DELETE(mObjectPool,object);
	//	}
	//	if (isLock) unLock();
	//}

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	void SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::destroy(DataSetMapIter it, Bool isLock)
	{
		if (isLock) lock();
		{
			if ( it != mDataList.end() )
			{
				MG_POOL_DELETE(mObjectPool,it->second);
				mDataList.erase(it);
			}  
		}
		if (isLock) unLock();
	}

    //-----------------------------------------------------------------------------
    template<typename TYPE_KEY, typename TYPE_CLASS>
    void SimpleMapDataSetManager<TYPE_KEY,TYPE_CLASS>::destroyAll(Bool isLock)
    {
        if (isLock) lock();
        {
            mDataList.clear();
            mObjectPool.releaseAllObject( true );
        }
        if (isLock) unLock();
    }

}
/******************************************************************************/
#endif // _DATASETMANAGER_H