/******************************************************************************/
#ifndef _DBQUERYRESULT_H
#define _DBQUERYRESULT_H
/******************************************************************************/

#include "HeapOp.h"
#include "MGLinkList.h"

namespace MG
{

#define MG_MAX_QUERY_STRING				8192

    //
    // 结果数据类型
    //
    enum DBQueryFieldType
    {
        DB_FIELD_TYPE_INVALID,
       
        DB_FIELD_TYPE_Char,
        DB_FIELD_TYPE_Char8,
        DB_FIELD_TYPE_SHORT,
        DB_FIELD_TYPE_USHORT,
        DB_FIELD_TYPE_INT,
        DB_FIELD_TYPE_UINT,
        DB_FIELD_TYPE_LONGLONG,
        DB_FIELD_TYPE_ULONGLONG,

        DB_FIELD_TYPE_Flt,

        DB_FIELD_TYPE_Dlb,

        DB_FIELD_TYPE_STRING,
        DB_FIELD_TYPE_BINARY_STRING,
    };

    //
    // 查询结果错误类型
    //
    enum DBQueryResultType
    {
        DBQuery_Result_VALID,
        DBQuery_Result_INVALID
    };

    // =======================================================================
    //
    // 结果数据
    //
    // =======================================================================
    typedef struct DBQueryResultData_s
    {
        DBQueryFieldType    fieldType;
        union
        {
            I8      byteData;
            U8      ubyteData;

            SChar8  binData;
            Char8   ubinData;

            I16 shortData;
            U16 ushortData;

            I32 intData;
            U32 uintData;

            I64 longlongData;
            U64 ulonglongData;

            Flt FltData;
            Dlb DlbData;
        };

        Char*   stringBuf;
        I32 stringBufLen;

        DBQueryResultData_s()
        {
            fieldType		= DB_FIELD_TYPE_INVALID;
            stringBufLen	= 0;
            stringBuf		= NULL;
        }

    } DBQueryResultData_t;


    // =======================================================================
    //
    // 查寻结果
    //
    // =======================================================================
    class DBQueryResult
    {
        friend class DBDriver;

    public:

        DBQueryResult();
        ~DBQueryResult();

        void                free();

        I32                 getRowCount();
        I32                 getColCount();
        void                setAffectedRows(U32& value);
        U32                 getAffectedRows();
        DBQueryResultType   getResultType();

        DBQueryFieldType    getFieldType( I32 row, I32 col );

        DBQueryResultData_s*getFieldData( I32 row, I32 col );

        Bool                getFieldData( I32 row, I32 col, Char* buf, I32 bufLen );
        Bool                getFieldData( I32 row, I32 col, Str& buf );

        Bool                getFieldData( I32 row, I32 col, SChar8& value );
        Bool                getFieldData( I32 row, I32 col, Char8& value );
        Bool                getFieldData( I32 row, I32 col,  U8& value );
        Bool                getFieldData( I32 row, I32 col, I16& value );
        Bool                getFieldData( I32 row, I32 col, U16& value );
        Bool                getFieldData( I32 row, I32 col, I32& value );
        Bool                getFieldData( I32 row, I32 col, U32& value );
        Bool                getFieldData( I32 row, I32 col, I64& value );
        Bool                getFieldData( I32 row, I32 col, U64& value );
        Bool                getFieldData( I32 row, I32 col, Flt& value );
        Bool                getFieldData( I32 row, I32 col, Dlb& value );

        Bool                getNextFieldData( I32 row, Char* buf, I32 bufLen );

        Bool                getNextFieldData( I32 row, SChar8& value );
        Bool                getNextFieldData( I32 row, Char8& value );
        Bool                getNextFieldData( I32 row, I16& value );
        Bool                getNextFieldData( I32 row, U16& value );
        Bool                getNextFieldData( I32 row, I32& value );
        Bool                getNextFieldData( I32 row, U32& value );
        Bool                getNextFieldData( I32 row, I64& value );
        Bool                getNextFieldData( I32 row, U64& value );
        Bool                getNextFieldData( I32 row, Flt& value );
        Bool                getNextFieldData( I32 row, Dlb& value );

        Bool                setFieldData( I32 row, I32 col, SChar8 value );
        Bool                setFieldData( I32 row, I32 col, Char8 value );
        Bool                setFieldData( I32 row, I32 col, I16 value );
        Bool                setFieldData( I32 row, I32 col, U16 value );
        Bool                setFieldData( I32 row, I32 col, I32 value );
        Bool                setFieldData( I32 row, I32 col, U32 value );
        Bool                setFieldData( I32 row, I32 col, I64 value );
        Bool                setFieldData( I32 row, I32 col, U64 value );
        Bool                setFieldData( I32 row, I32 col, Flt value );
        Bool                setFieldData( I32 row, I32 col, Dlb value );
        Bool                setFieldData( I32 row, I32 col, const Char* buf );
        Bool                setFieldData( I32 row, I32 col, const Char* buf, I32 bufSize );

    protected:

        void                setDriver(DBDriver* driver);
        Char*               allocChar();

    private:

        DBDriver*               mDriver;
        MGLinkList<DBQueryResult>	mResultNode;

        I32                     mRowNum;
        I32                     mColNum;
        U32                     mAffedRow;
        DBQueryResultType       mResultType;

        I32                     mLastCol;

        I32                     mDataNum;
        DBQueryResultData_t*    mResults;

        DBQueryResultData_t*    GetResult( I32 row, I32 col )
        {
            if ( mResults == NULL
                || row < 0
                || row >= mRowNum
                || col < 0
                || col >= mColNum )
            {
                return NULL;
            }

            return &(mResults[row*mColNum + col]);
        }
    };

    // =======================================================================
    //
    // 结果集操作
    //
    // =======================================================================

    MG_INLINE I32 DBQueryResult::getRowCount()
    {
        return mRowNum;
    }

    MG_INLINE I32 DBQueryResult::getColCount()
    {
        return mColNum;
    }

    MG_INLINE void DBQueryResult::setAffectedRows(U32& value)
    {
        mAffedRow = value;
    }

    MG_INLINE U32 DBQueryResult::getAffectedRows()
    {
        return mAffedRow;
    }

    MG_INLINE DBQueryResultType DBQueryResult::getResultType()
    {
        return mResultType;
    }

    // ============
    //
    // 获得指定字段的类型
    //
    // ============

    MG_INLINE DBQueryFieldType DBQueryResult::getFieldType( I32 row, I32 col )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return DB_FIELD_TYPE_INVALID;
        }

        return result->fieldType;
    }

    // ============
    //
    // 获得指定记录，指定字段的数据
    //
    // ============

    MG_INLINE DBQueryResultData_s* DBQueryResult::getFieldData( I32 row, I32 col )
    {
        return GetResult( row, col );
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, Char* buf, I32 bufLen )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        if ( result->stringBuf == NULL )
        {
            return false;
        }

        I32 copySize = min( result->stringBufLen, bufLen );

        memcpy( buf, result->stringBuf, copySize );

        if ( result->fieldType == DB_FIELD_TYPE_STRING )
        {
            // 写入字符串 \0 结束符
            if ( copySize < bufLen )
            {
                buf[copySize] = '\0';
            }
            else
            {
                buf[bufLen-1] = '\0';
            }
        }

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, Str& buf )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        if ( result->stringBuf == NULL )
        {
            return false;
        }

        buf = result->stringBuf;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col,  SChar8& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->binData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col,  Char8& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->ubinData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col,  U8& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->ubyteData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, U16& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->ushortData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, I32& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->intData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, U32& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->uintData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, I64& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->longlongData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, U64& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->ulonglongData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, Flt& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->FltData;

        mLastCol	= col;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getFieldData( I32 row, I32 col, Dlb& value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        value = result->DlbData;

        mLastCol	= col;

        return true;
    }

    // ============
    //
    // 获得下一个字段的数据，由getFieldData来指定第一个字段
    //
    // ============
    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, Char* buf, I32 bufLen )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        StrnCpyS( buf, bufLen, result->stringBuf, bufLen );

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row,  SChar8& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->binData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row,  Char8& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->ubinData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, I16& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->shortData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, U16& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->ushortData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, I32& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->intData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, U32& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->uintData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, I64& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->longlongData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, U64& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->ulonglongData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, Flt& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->FltData;

        return true;
    }

    MG_INLINE Bool DBQueryResult::getNextFieldData( I32 row, Dlb& value )
    {
        if ( mLastCol < mColNum )
        {
            mLastCol++;
        }

        DBQueryResultData_t* result = GetResult( row, mLastCol );
        if ( !result )
        {
            return false;
        }

        value = result->DlbData;

        return true;
    }

    // ============
    //
    // 设置指定记录，指定字段的数据
    //
    // ============

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col,  SChar8 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_Char;
        result->binData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col,  Char8 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_Char8;
        result->ubinData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, I16 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_SHORT;
        result->shortData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, U16 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_USHORT;
        result->ushortData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, I32 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_INT;
        result->intData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, U32 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_UINT;
        result->uintData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, I64 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_LONGLONG;
        result->longlongData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, U64 value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_ULONGLONG;
        result->ulonglongData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, Flt value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_Flt;
        result->FltData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, Dlb value )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        result->fieldType = DB_FIELD_TYPE_Dlb;
        result->DlbData = value;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, const Char* buf )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        I32 len = StrLen( buf );
        if ( len >= MG_MAX_QUERY_STRING )
        {
            return false;
        }

        if ( result->stringBuf == NULL )
        {
            result->stringBuf = allocChar();
            if ( result->stringBuf == NULL )
            {
                return false;
            }
        }

        result->fieldType = DB_FIELD_TYPE_STRING;
        strcpy_s(result->stringBuf, len + 1, buf);

        result->stringBufLen = len;

        return true;
    }

    MG_INLINE Bool DBQueryResult::setFieldData( I32 row, I32 col, const Char* buf, I32 bufSize )
    {
        DBQueryResultData_t* result = GetResult( row, col );
        if ( !result )
        {
            return false;
        }

        if ( bufSize > MG_MAX_QUERY_STRING )
        {
            return false;
        }

        if ( result->stringBuf == NULL )
        {
            result->stringBuf = allocChar();
            if ( result->stringBuf == NULL )
            {
                return false;
            }
        }

        result->fieldType = DB_FIELD_TYPE_BINARY_STRING;
        memcpy( result->stringBuf, buf, bufSize );

        result->stringBufLen = bufSize;

        return true;
    }
}

#endif // DBQUERYRESULT_H
