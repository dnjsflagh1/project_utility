/******************************************************************************/
#include "stdafx.h"
#include "MySqlDriver.h"

/******************************************************************************/


namespace MG
{

    typedef struct tagMYSQL_FIELD_LEN
    {
        U32	length;
        U32	max_lengh;              // 列中的最长长度
        enum	enum_field_types type;  // 字段类型`
        U32		flags;

    }MYSQL_FIELD_LEN;

    class MysqlDriverStub
    {
    public:
        MYSQL				*mysql;
        MYSQL_RES			*result;	// 查詢結果
        MYSQL_ROW			curr_row;	// 得到的當前行
        I32					FieldNum;	// 字段數
        Char                g_field_name[200][40];
        MYSQL_FIELD_LEN		*mfl;
    };


    //-----------------------------------------------------------------------------
    MySqlDriver::MySqlDriver(void)
        :mData(NULL),
        mIsQueryLock(false)
    {

    }
    //-----------------------------------------------------------------------------
    MySqlDriver::~MySqlDriver(void)
    {
        disconnect();
    }

    //-----------------------------------------------------------------------------
    Bool MySqlDriver::connect( MySqlInitParam& param, Char* loacalCharset, Bool multiThreadMode )
    {
        my_bool reconnect = 1;

        MG_LOG( out_info, "%s: driver initializing...\n", __MG_FUNC__ );

        if ( mData )
        {
            MG_LOG( out_error, "%s error, already initialized\n", __MG_FUNC__ );
            return false;
        }

        mData = new MysqlDriverStub;

        if ( mData == NULL )
        {
            MG_LOG( out_error, "%s error, no enough memory for init\n", __MG_FUNC__ );
            return false;
        }

        // 调用任何 MySQL 语句前都要调用
        // my_init();

        // 初始化 MySQL C API 库，可省略 mysql_init 能自动调用它
        if (mysql_library_init (0, NULL, NULL))  
        {
            MG_LOG( out_error, ("mysql_library_init() failed\n"));
            return false;
        }

        // 初始化 MySQL 连接句柄
        mData->mysql = mysql_init( NULL );
        if ( mData->mysql == NULL )
        {
            MG_LOG( out_error, "%s error, mysql init failed\n", __MG_FUNC__ );
            return false;
        }

        // 配置连接参数
        if ( mysql_options( mData->mysql, MYSQL_OPT_RECONNECT, &reconnect) != 0 )
        {
            MG_LOG( out_error, "%s error, mysql mysql reconnect init option set failed\n", __MG_FUNC__ );
            return false;
        }

        // 配置字符集
        if ( mysql_options( mData->mysql, MYSQL_SET_CHARSET_NAME, loacalCharset ) != 0 )
        {
            MG_LOG( out_error, "%s error, mysql mysql charset option set failed\n", __MG_FUNC__ );
            return false;
        }

        // 与数据库建立连接
        if ( mysql_real_connect(
            mData->mysql, 
            param.ip.c_str(),
            param.account.c_str(),
            param.password.c_str(),
            param.dbName.c_str(),
            param.port,
            NULL,
            CLIENT_FOUND_ROWS ) == NULL )
        {
            MG_LOG( out_error, "%s error, connect to mysql server failed: %s\n", __MG_FUNC__, 
                mysql_error( mData->mysql ) );
            mysql_close (mData->mysql);  // 不是有效连接，但包括传送给错误报告函数的错误信息
            mData->mysql = NULL;
            mysql_library_end();  // 不调用，内存块仍保持分配状态（这不会增加应用程序使用的内存量，但某些内存泄漏检测器将抗议它）
            return false;
        }

        MG_LOG( out_info, "%s: driver initialize done\n", __MG_FUNC__ );

        return true;
    }

    //-----------------------------------------------------------------------------
    Bool MySqlDriver::disconnect()
    {
        if ( mData->mysql != NULL )
        {
            mysql_close( mData->mysql );
            mData->mysql = NULL;
        }

        if ( mData != NULL)
        {
            delete mData;
            mData = NULL;
        }

        mysql_library_end();

        MG_LOG( out_info, "%s: driver closed\n", __MG_FUNC__ );

        return true;
    }

    //-----------------------------------------------------------------------------
    Bool MySqlDriver::selectDB( Char* name )
    {
        if ( mData && mData->mysql )
        {
            if ( mysql_select_db( mData->mysql, name ) != 0 )
            {
                MG_LOG( out_error, "%s error: select db failed, %s\n", __MG_FUNC__, mysql_error( mData->mysql ) );
                return false;
            }
            return true;
        }
        return false;
    }

    //-----------------------------------------------------------------------------
    void MySqlDriver::queryLock()
    {
        mThreadLock.lock();
        mIsQueryLock = true;
    }

    //-----------------------------------------------------------------------------
    void MySqlDriver::queryUnLock()
    {
        mIsQueryLock = false;
        mThreadLock.unlock();
    }

    //-----------------------------------------------------------------------------
    DBQueryResultEx MySqlDriver::query( const Char* queryString, Bool isCheckLock )
    {
        if ( queryString == NULL )
        {
            return DBQueryResultEx(NULL);
        }

        return DBQueryResultEx( this->performQuery(queryString,0,isCheckLock) );
    }

    //-----------------------------------------------------------------------------
    DBQueryResultEx MySqlDriver::realquery( const Char* queryString, I32 stringLen, Bool isCheckLock )
    {
        if ( queryString == NULL )
        {
            return DBQueryResultEx(NULL);
        }

        return DBQueryResultEx( this->performQuery(queryString, stringLen, isCheckLock) );
    }

    //-----------------------------------------------------------------------------
    Bool MySqlDriver::startTransaction()
    {
        if (mysql_autocommit(mData->mysql, 0) != 0)
        {
            return false;
        }
        return true;
    }

    //-----------------------------------------------------------------------------
    Bool MySqlDriver::rollback()
    {
        if (mysql_rollback(mData->mysql) != 0)
        {
            return false;
        }
        return true;
    }

    //-----------------------------------------------------------------------------
    Bool MySqlDriver::commit()
    {
        if (mysql_commit(mData->mysql) != 0)
        {
            return false;
        }
        return true;
    }

    //-----------------------------------------------------------------------------
    Bool MySqlDriver::endTransaction()
    {
        if (mysql_autocommit(mData->mysql, 1) != 0)
        {
            return false;
        }
        return true;
    }

    //-----------------------------------------------------------------------------
    I32 MySqlDriver::getLastErrorNo()
    {
        if ( mData && mData->mysql )
        {
            I32 num = mysql_errno( mData->mysql );
            return num;
        }
        return 0;
    }

    //-----------------------------------------------------------------------------
    const Char* MySqlDriver::getLastErrorString()
    {
        if ( mData && mData->mysql )
        {
            const Char* p = mysql_error( mData->mysql );
            return p;
        }
        return "";
    }

    //-----------------------------------------------------------------------------
    U64 MySqlDriver::escapeString(Char *to, const Char *from, U32 length)
    {
        return mysql_real_escape_string(mData->mysql, to, from, length);
    }

    //-----------------------------------------------------------------------------
    DBDriverType MySqlDriver::getDriverType()
    {
        return DDT_MYSQL;
    }

    //-----------------------------------------------------------------------------
    DBQueryResult* MySqlDriver::performQuery( const Char* queryString, I32 stringLen, Bool isCheckLock )
    {
        DBQueryResult *result = NULL;
        MYSQL_RES		*res = NULL;
        MYSQL_FIELD		*fd = NULL;
        MYSQL_ROW		row = 0;
        I32				state = 0;
        U32				row_num = 0;
        U32				col_num = 0;
        U32				aff_num = 0;
        U32				i = 0;
        U32				j = 0;

        MYSQL_FIELD_LEN *fd_len = NULL;

        if ( !mData || !mData->mysql )
        {
            MG_LOG( out_error, "%s: need initialize\n", __MG_FUNC__ );
            return NULL;
        }

#ifdef _DEBUG
        static U64 totalQueryNum = 0;
        static U64 totalQueryTime = 0;
        static U32 maxQueryTime = 0;
        static Dbl avgQueryTime = 0;
        static Dbl avgQueryNumPerSecond = 0;

        static Char queryBuf[10240*4];

        totalQueryNum++;

        size_t queryBufLen = strlen( queryString );
        assert( sizeof(queryBuf) > queryBufLen );

        if ( sizeof(queryBuf) > queryBufLen )
        {
            strcpy_s( queryBuf, queryString );

            Char* queryCh = queryBuf;

            while ( *queryCh )
            {
                if ( *queryCh == '\t' )
                {
                    *queryCh = ' ';
                }

                queryCh++;
            }

            MG_LOG( out_debug, ("\n<<Query Debug>>:\n" ) );
            MG_LOG( out_debug, ("%s\n"), queryBuf );
        }
        else
        {
            MG_LOG( out_debug, ("%s(%d) : buf size is not enough\n"), __MG_FUNC__, __MG_LINE__ );
        }

        U32 startTime = MGTimeOp::getCurrTick();
#endif

        MYSQL *mysql = mData->mysql;

        //*****************************************************************************
        if ( isCheckLock )
            if ( mIsQueryLock )
                mThreadLock.lock();
        //*****************************************************************************
        {
            // 判断执行哪种 mysql_query
            if ( stringLen == 0 ) {
                state = mysql_query( mysql, queryString );
            }
            else {
                state = mysql_real_query( mysql, queryString, stringLen );
            }

            if ( state != 0 )  // 判断操作是否正常执行
            {
                MG_LOG( out_error, "%s: mysql_query() error: %s\n", __MG_FUNC__, mysql_error( mysql ) );
                MG_LOG( out_error, "  %.510s \n", queryString );
                return NULL;
            }

            res = mysql_store_result( mysql ); // create table 这样的操作无返回结果集
            if ( res != NULL )
            {
                // 获得，行，列
                row_num = (U32)mysql_num_rows( res );
                col_num	= (U32)mysql_num_fields( res );
            }

            result = this->allocQueryResult( row_num, col_num );  // 获得结果集
            if(result == NULL)
            {
                MG_LOG( out_error, "%s: alloc StoreResult failed\n", __MG_FUNC__ );
                mysql_free_result( res );
                return NULL;
            }

            // 获得 影响行
            aff_num = (U32)mysql_affected_rows(mysql);
            result->setAffectedRows(aff_num);

            // SELECT 无匹配, 或 INSERT, REPLACE, UPDATE, DELETE 会越过下面的操作
            // 因为他们有返回结果集，但是为 0 行
            if ( row_num > 0 )
            {
                // 创建存储结果集列信息组
                fd_len = new MYSQL_FIELD_LEN[col_num];
                if ( fd_len == NULL )
                {
                    MG_LOG( out_error, "%s: new MYSQL_FIELD_LEN[] array failed\n", __MG_FUNC__ );
                    mysql_free_result( res );
                    result->free();
                    return NULL;
                }

                // 获得列信息
                for ( i = 0; i < col_num; i++ )
                {
                    fd = mysql_fetch_field( res );
                    fd_len[i].length = fd->length;
                    fd_len[i].max_lengh = fd->max_length;
                    fd_len[i].type = fd->type;
                    fd_len[i].flags = fd->flags;
                }

                for ( i = 0; row = mysql_fetch_row( res ); i++ )
                {
                    for( j = 0; j < col_num; j++ )
                    {
                        if ( row[j] == NULL )  // 字段值 为 NULL 类型
                        {
                            result->setFieldData( i, j, (double)0 );
                            result->setFieldData( i, j, "\0", 1 );
                        }
                        else if ( IS_NUM(fd_len[j].type) )
                        {
                            switch( fd_len[j].type )
                            {   // 整形，有符号和无符号
                            case MYSQL_TYPE_TINY:
                                if ( fd_len[j].flags & UNSIGNED_FLAG ) {
                                    result->setFieldData( i, j, (Char)StrtoUL(row[j], NULL, 10) );
                                }
                                else {
                                    result->setFieldData( i, j, (SChar8)AtoI(row[j]) );
                                }
                                break;
                            case MYSQL_TYPE_SHORT:
                                if ( fd_len[j].flags & UNSIGNED_FLAG ) {
                                    result->setFieldData( i, j, (U16)StrtoUL(row[j], NULL, 10) );
                                }
                                else {
                                    result->setFieldData( i, j, (I16)AtoI(row[j]) );
                                }
                                break;
                            case MYSQL_TYPE_INT24:
                            case MYSQL_TYPE_LONG:  // 
                                if ( fd_len[j].flags & UNSIGNED_FLAG ) {
                                    result->setFieldData( i, j, (U32)StrtoUL(row[j], NULL, 10) );
                                }
                                else {
                                    result->setFieldData( i, j, (I32)AtoI(row[j]) );
                                }
                                break;
                            case MYSQL_TYPE_LONGLONG:
                                if ( fd_len[j].flags & UNSIGNED_FLAG ) {
                                    result->setFieldData( i, j, (U64)StrtoULL(row[j], NULL, 10) );
                                }
                                else {
                                    result->setFieldData( i, j, (I64)AtoLL(row[j]) );
                                }
                                break;
                                // 浮点型
                            case MYSQL_TYPE_FLOAT:
                                result->setFieldData( i, j, (Flt)AtoF(row[j]) );
                                break;
                            case MYSQL_TYPE_DECIMAL:
                            case MYSQL_TYPE_NEWDECIMAL:
                            case MYSQL_TYPE_DOUBLE:
                                result->setFieldData( i, j, (Dbl)AtoF(row[j]) );
                                break;
                            case MYSQL_TYPE_YEAR:
                            case MYSQL_TYPE_TIMESTAMP:  // 标准 time 的 tm 格式要 36 个字节，按字符串返回只要 19 个字节
                                result->setFieldData( i, j, row[j], fd_len[j].max_lengh );
                                break;
                            default:
                                assert(0);  // 未知类型
                                break;
                            } // end switch
                        }  // end else if
                        else
                        {
                            switch( fd_len[j].type )
                            {  // 字符串
                            case MYSQL_TYPE_ENUM:
                            case MYSQL_TYPE_SET:
                            case MYSQL_TYPE_BIT:
                            case MYSQL_TYPE_DATE:
                            case MYSQL_TYPE_DATETIME:
                            case MYSQL_TYPE_TIME:
                            case MYSQL_TYPE_STRING:
                            case MYSQL_TYPE_VAR_STRING:
                                result->setFieldData( i, j, row[j] );
                                break;
                            case MYSQL_TYPE_BLOB:
                                result->setFieldData( i, j, row[j], fd_len[j].max_lengh);	// 末尾有'\0'
                                break;
                            default:
                                assert(0);  // 未知类型
                                break;
                            }
                        }
                    }  // end for
                }  // end for
                delete[] fd_len;
            }

            // create table, alter 操作是无结果集返回的
            if (res != NULL)
            {	// 我不确定当 res 返回为 NULL 的时候是否需要 mysql_free_result
                mysql_free_result( res );
            }
        }
        //*****************************************************************************
        if ( isCheckLock )
            if ( mIsQueryLock )
                mThreadLock.unlock();
        //*****************************************************************************


#ifdef _DEBUG
        U32 endTime = MGTimeOp::getCurrTick();
        if ( ( endTime - startTime ) > maxQueryTime )
        {
            maxQueryTime = endTime - startTime;
        }

        static U32 firstQueryTime = MGTimeOp::getCurrTick();

        if ( endTime != firstQueryTime )
        {
            avgQueryNumPerSecond = (Dbl)totalQueryNum/(Dbl)(endTime - firstQueryTime);
        }

        totalQueryTime += (endTime - startTime );

        avgQueryTime = (Dbl)totalQueryTime/(Dbl)totalQueryNum;

        MG_LOG( out_debug, ("<<Query cost %u (ms), totalQueryNum %llu, maxQueryTime %u, avgTime %.4f, QPS %.2f>>\n\n"), 
            endTime - startTime, totalQueryNum, maxQueryTime, avgQueryTime, avgQueryNumPerSecond );
#endif
        return result;
    }

    //-----------------------------------------------------------------------
    Bool MySqlDriver::addColume( const Char* tableName,Colume * columes,I32 count )
    {
        
        DBQueryResult* res;
        static Char temp[256] = {0};
        for (int i=0;i<count;i++)
        {
            MG::MGStrOp::sprintf(temp,256,MG_STR("ALTER TABLE %s ADD %s %s NOT NULL DEFAULT %s"),tableName,columes[i].name,columes[i].type,columes[i].dValue);
            DBQueryResultEx resEx = query(temp);
            res = *resEx;
            if (res && res->getResultType() == DBQuery_Result_INVALID)
            {
                return false;
            }
        }
        return true;	
    }

}