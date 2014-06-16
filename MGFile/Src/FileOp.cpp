/******************************************************************************/
#include "stdafx.h"
#include "MGStrOp.h"
#include "FileOp.h"
/******************************************************************************/

namespace MG
{

    //-----------------------------------------------------------------------
    Str FileOp::getFullFilePath(Str& pathName,Str& fileName)
    {
        Char c;
        if ( pathName.empty() == false )
        {
            c = pathName[pathName.size()-1];
            if ( c!='/' && c!='\\')
            {
                pathName += '/';
            }
        }
        return pathName + fileName;
    }

    //-----------------------------------------------------------------------
    Bool FileOp::isExistDirectory(Str& pathName)
    {
        WIN32_FIND_DATA wfd;
        Bool result = false;

        HANDLE hFind = ::FindFirstFile( pathName.c_str(), &wfd );

        if ( (hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
            result = true;   
        }

        FindClose(hFind);

        return result;
    }

    //-----------------------------------------------------------------------
    Bool FileOp::createDirectory(Str& pathName)
    {
        if ( isExistDirectory( pathName ) == false )
        {
            return (Bool)( ::CreateDirectory( pathName.c_str(), NULL ) != 0 );
        }
        return true;
    }

    //-----------------------------------------------------------------------
    Bool FileOp::removeDirectory(Str& pathName)
    {
        // 需要递归删除文件夹中所有文件
        //if ( isExistDirectory( pathName ) == true )
        //{
        //    return ::RemoveDirectory( pathName.c_str(), NULL );
        //}
        return true;
    }

    //-----------------------------------------------------------------------
    Bool FileOp::cleanDirectory(Str& pathName)
    {
        return true;
    }

	MG::Bool FileOp::isExistFile( Str& pathName,Str& fileName )
	{
		WIN32_FIND_DATA wfd;
		Bool result = false;
		Str fullFilePath = FileOp::getFullFilePath(pathName,fileName);
		HANDLE hFind = ::FindFirstFile( fullFilePath.c_str(), &wfd );

		if ( (hFind != INVALID_HANDLE_VALUE) && (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) )
		{
			result = true;   
		}

		FindClose(hFind);

		return result;



	}
}


