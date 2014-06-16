/******************************************************************************/
#include "stdafx.h"
#include "DBQueryResult.h"
#include "DBDriver.h"
/******************************************************************************/

namespace MG
{
    //-----------------------------------------------------------------------------
    DBQueryResult::DBQueryResult()
        :mResults(NULL),
        mRowNum(0),
        mColNum(0),
        mDataNum(0),
        mDriver(NULL)
    {
    }

    //-----------------------------------------------------------------------------
    DBQueryResult::~DBQueryResult()
    {
        if ( mResults )
        {
            delete[] mResults;
            mResults = NULL;
        }
    }

    //-----------------------------------------------------------------------------
    void DBQueryResult::setDriver(DBDriver* driver)
    {
        mDriver = driver;
    }

    //-----------------------------------------------------------------------------
    void DBQueryResult::free()
    {
        DYNAMIC_ASSERT(mDriver!=NULL);
        mDriver->free(this);
    }

    //-----------------------------------------------------------------------------
    Char* DBQueryResult::allocChar()
    {
        DYNAMIC_ASSERT(mDriver!=NULL);
        return mDriver->allocChar();
    }
}
