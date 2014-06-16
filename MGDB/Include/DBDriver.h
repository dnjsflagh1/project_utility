/******************************************************************************/
#ifndef _DBDRIVER_H_
#define _DBDRIVER_H_
/******************************************************************************/

#include "Lock.h"
#include "MGLinkList.h"
#include "HeapOp.h"
#include "DBQueryResult.h"

namespace MG
{
	/******************************************************************************/
	//DB字段
	/******************************************************************************/
	struct Colume
	{
		char name[32];
		char type[16];
		char dValue[32];
	};
    /******************************************************************************/
    //DB初始化参数
    /******************************************************************************/
    struct MySqlInitParam
    {
        Str ip;
        Int port;
        Str	account;
        Str	password;
        Str	dbName;
    };

    /******************************************************************************/
    //DB驱动类型
    /******************************************************************************/
    enum DBDriverType
    {
        DDT_MYSQL,
        DDT_SQLSERVER,
        DDT_FILE
    };
    /******************************************************************************/
    //DB查询结果处理类（数据库查询结果集修改）
    /******************************************************************************/
    class  DBQueryResultEx
    {
    public:
        DBQueryResultEx(DBQueryResult* res)
            :mDBQRes(res)
        {
        }
        //-------------------------------------------------
        DBQueryResultEx(const DBQueryResultEx& DBQResEx)
            :mDBQRes(*DBQResEx)
        {
        }
        //-------------------------------------------------
        ~DBQueryResultEx()
        {
            if ( mDBQRes )
                mDBQRes->free();
        }
        //-------------------------------------------------
        DBQueryResult* operator*() const 
        {
            return mDBQRes;
        }
        //-------------------------------------------------
        DBQueryResultEx operator=(const DBQueryResultEx& DBQResEx)
        {
            DBQueryResult* res = *DBQResEx;
            if ( res != mDBQRes )
            {
                if ( mDBQRes )
                    mDBQRes->free();
                mDBQRes = *DBQResEx;
            }
            return *this;
        }
        
    protected:
    private:
        DBQueryResult* mDBQRes;
    };
    //DB服务器操作接口
    /******************************************************************************/
    class DBDriver
    {
        friend class DBQueryResult;
    public:

        DBDriver();
        virtual ~DBDriver();

        virtual Bool                connect( MySqlInitParam& param, Char* loacalCharset, Bool multiThreadMode = false ) = 0;
        virtual Bool                disconnect() = 0;    

        virtual Bool                selectDB( Char* name ) = 0;

        virtual DBQueryResultEx 	query( const Char* queryString, Bool isCheckLock = true ) = 0;
        virtual DBQueryResultEx 	realquery( const Char* queryString, I32 stringLen, Bool isCheckLock = true ) = 0;

		virtual	Bool				addColume(const Char* tableName,Colume * columes,I32 count) = 0;

        virtual Bool				startTransaction() = 0;
        virtual Bool				rollback() = 0;
        virtual Bool				commit() = 0;
        virtual Bool				endTransaction() = 0;

        virtual void				queryLock() = 0;
        virtual void				queryUnLock() = 0;

        virtual I32					getLastErrorNo() = 0;
        virtual const Char*			getLastErrorString() = 0;

        virtual U64                 escapeString(Char *to, const Char *from, U32 length) = 0;

        virtual DBDriverType		getDriverType() = 0;

        void                        free( DBQueryResult* queryRes ); 

    protected:

        Bool                        initialize();
        void                        uninitialize();

        Char*                       allocChar();
        DBQueryResult*              allocQueryResult( I32 row, I32 col );
         
        void					    clearQueryResultList();
        I32					        getQueryResultQueueCount();
        
    private:
        
        BlockAlloc< Char[MG_MAX_QUERY_STRING], 128, 3 >  mStrAllocator;

        MGLinkList<DBQueryResult>   mFreeResults;
        MGLinkList<DBQueryResult>   mResultQueue;
        DBQueryResult*  mResultPool;

        Critical  mCS;
    };

}

/******************************************************************************/
#endif

