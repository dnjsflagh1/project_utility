/******************************************************************************/
#include "stdafx.h"
#include "DBQueryResult.h"
#include "DBDriver.h"
/******************************************************************************/

namespace MG
{

    #define MG_MAX_RESULT_NUM   4096

    //-----------------------------------------------------------------------------
    DBDriver::DBDriver()
        :mResultPool(NULL)
    {
        initialize();
    }

    //-----------------------------------------------------------------------------
    DBDriver::~DBDriver()
    {
        uninitialize();
    }

    //-----------------------------------------------------------------------------
    Bool DBDriver::initialize()
    {
        I32 i;

        if ( mResultPool != NULL )
        {
            MG_LOG( out_error, ("%s error: StoreQueryResult data already initialized\n"), __MG_FUNC__ );
            return false;
        }

        mResultPool = MG_NEW DBQueryResult[MG_MAX_RESULT_NUM];
        if ( mResultPool == NULL )
        {
            MG_LOG( out_error, ("%s error: alloc StoreQueryResult data error\n"), __MG_FUNC__ );
            return false;
        }

        for ( i = 0; i < MG_MAX_RESULT_NUM; i++ )
        {
            mResultPool[i].mResults = NULL;
            mResultPool[i].setDriver(this);
            mResultPool[i].free();
        }

        return true;
    }
    //-----------------------------------------------------------------------------
    void DBDriver::uninitialize()
    {
        MG_SAFE_DELETE_ARRAY( mResultPool );
    }

    //-----------------------------------------------------------------------------
    Char* DBDriver::allocChar()
    {
        Char* result = (Char*)mStrAllocator.Alloc();

        memset( result, 0, MG_MAX_QUERY_STRING );

        return result;
    }
    
    //-----------------------------------------------------------------------------
    DBQueryResult* DBDriver::allocQueryResult( I32 row, I32 col )
    {
        DBQueryResult* result = NULL;
        I32 count = row * col;

        mCS.lock();
        {
            if ( mFreeResults.IsListEmpty() )
            {
                DYNAMIC_ASSERT( false );
                MG_LOG( out_error, ("%s error, free list empty\n"), __MG_FUNC__ );
				mCS.unlock();
                return NULL;
            }

            result = mFreeResults.Next();
            result->mResultNode.Remove();

            if ( result->mDataNum < count )
            {
                if ( result->mResults )
                {
                    for ( I32 i = 0; i < result->mDataNum; i++ )
                    {
                        if ( result->mResults[i].stringBuf )
                        {
                            mStrAllocator.Free( (char (*)[MG_MAX_QUERY_STRING])result->mResults[i].stringBuf );
                            result->mResults[i].stringBuf = NULL;
                        }
                    }

                    delete[] result->mResults;
                }

                result->mResults = MG_NEW DBQueryResultData_t[count];
                if ( result == NULL )
                {
                    MG_LOG( out_error, ("%s error: alloc result data failed\n"), __MG_FUNC__ );
					mCS.unlock();
                    return NULL;
                }

                result->mDataNum = count;
            }
            result->mColNum		= col;
            result->mRowNum		= row;
            result->mLastCol	= 0;
            result->mResultNode.AddToEnd( mResultQueue );
        }
        mCS.unlock();

        return result;
    }

    //-----------------------------------------------------------------------------
    void DBDriver::free( DBQueryResult* queryRes )
    {
        mCS.lock();
        {
            queryRes->mRowNum = 0;
            queryRes->mRowNum = 0;

            queryRes->mResultNode.SetOwner( queryRes );
            queryRes->mResultNode.AddToFront( mFreeResults );
        }
        mCS.unlock();
    }

    //-----------------------------------------------------------------------------
    void DBDriver::clearQueryResultList()
    {
        mCS.lock();
        {
            mFreeResults.Clear();
            mResultQueue.Clear();

            if ( mResultPool )
            {
                I32 i;

                for ( i = 0; i < MG_MAX_RESULT_NUM; i++ )
                {
                    mResultPool[i].free();
                }
            }
        }
        mCS.unlock();
    }

    //-----------------------------------------------------------------------------
    I32 DBDriver::getQueryResultQueueCount()
    {
        I32 count = 0;
        mCS.lock();
        {
            count = mResultQueue.Count();
        }
        mCS.unlock();
        return count;
    }



}