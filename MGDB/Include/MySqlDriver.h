/******************************************************************************/
#ifndef _MYSQLDRIVER_H_
#define _MYSQLDRIVER_H_
/******************************************************************************/

#include "DBDriver.h"

namespace MG
{

    class MysqlDriverStub;
    /******************************************************************************/
    //MySql服务器操作管理器
    /******************************************************************************/
    class MySqlDriver : public DBDriver
    {
    public:
        MySqlDriver();
        virtual ~MySqlDriver();

    public:

        virtual Bool                connect( MySqlInitParam& param, Char* loacalCharset, Bool multiThreadMode = false );
        virtual Bool                disconnect();    

        virtual Bool                selectDB( Char* name );


        // +-------------------------------------------+------------+
        // |  MYSQL_TYPE_TINY         TINYINT          |            |
        // |  MYSQL_TYPE_SHORT        SMALLINT         |            |
        // |  MYSQL_TYPE_INT24        MEDIUMINT        |            |
        // |  MYSQL_TYPE_LONG         INT              |  NUM_FLAG  |
        // |  MYSQL_TYPE_LONGLONG     BIGINT           |  NUM_FLAG  |
        // +-------------------------------------------+------------+
        // |  MYSQL_TYPE_DECIMAL      DECIMAL, NUMERIC |            |
        // |  MYSQL_TYPE_NEWDECIMAL   DECIMAL, NUMERIC |            |
        // |  MYSQL_TYPE_DOUBLE       DOUBLE, REAL     |            |
        // |  MYSQL_TYPE_FLOAT        FLOAT            |            |
        // +-------------------------------------------+------------+
        // |  MYSQL_TYPE_STRING       CHAR             |    ELSE    |
        // |  MYSQL_TYPE_VAR_STRING   VARCHAR          |    ELSE    |
        // |  MYSQL_TYPE_BLOB         BLOB, TEXT       |            |
        // |  MYSQL_TYPE_ENUM         ENUM             |            |
        // |  MYSQL_TYPE_SET          SET              |            |
        // +-------------------------------------------+------------+
        // |  MYSQL_TYPE_DATE         DATE             |            |
        // |  MYSQL_TYPE_DATETIME     DATETIME         |    ELSE    |
        // |  MYSQL_TYPE_TIME         TIME             |            |
        // |  MYSQL_TYPE_TIMESTAMP    TIMESTAMP        |  NUM_FLAG  |
        // |  MYSQL_TYPE_YEAR         YEAR             |  NUM_FLAG  |
        // +-------------------------------------------+------------+
        // |  MYSQL_TYPE_BIT          BIT              |            |
        // |  MYSQL_TYPE_NULL         NULL             |            |
        // +-------------------------------------------+------------+
        virtual DBQueryResultEx 	query( const Char* queryString, Bool isCheckLock = true );
        virtual DBQueryResultEx 	realquery( const Char* queryString, I32 stringLen, Bool isCheckLock = true );

        virtual Bool				addColume(const Char* tableName,Colume * columes,I32 count);

        //事务是线程不安全的
        virtual Bool				startTransaction();
        virtual Bool				rollback();
        virtual Bool				commit();
        virtual Bool				endTransaction();

        //线程锁定
        virtual void				queryLock();
        virtual void				queryUnLock();

        virtual I32					getLastErrorNo();
        virtual const Char*			getLastErrorString();

        virtual U64                 escapeString(Char *to, const Char *from, U32 length);

        virtual DBDriverType		getDriverType();

    private:
        // 对 Query 和 RealQuery 的实现
        // stringLen == 0 执行 mysql_query， stringLen != 0 执行 mysql_real_query
        DBQueryResult*              performQuery( const Char* queryString, I32 stringLen = 0, Bool isCheckLock = true );
        MysqlDriverStub*	        mData;

        Critical					mThreadLock;
        Bool                        mIsQueryLock;
    };

}

/******************************************************************************/
#endif

