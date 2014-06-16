/******************************************************************************/
#include "stdafx.h"
#include "MGStrOp.h"
/******************************************************************************/

namespace MG
{
    //-----------------------------------------------------------------------------
    bool MGStrOp::isNullOrEmpty(CChar8* string)
    {
        if (string != NULL && strcmp(string,"") != 0)
        {
			return true;
        }
        return false;
    }
	//-----------------------------------------------------------------------------
	bool MGStrOp::isNullOrEmpty(CChar16* string)
	{
		if (string != NULL && wcscmp(string,L"") != 0)
		{
			return true;
		}
		return false;
	}
    //-----------------------------------------------------------------------------
	void MGStrOp::makeLower( std::string& str )
	{
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			tolower);
	}
    //-----------------------------------------------------------------------------
	void MGStrOp::makeLower( std::wstring& str )
	{
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			tolower);
	}
    //-----------------------------------------------------------------------------
	void MGStrOp::makeUpper( std::string& str )
	{
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			toupper);
	}
    //-----------------------------------------------------------------------------
	void MGStrOp::makeUpper( std::wstring& str )
	{
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			toupper);
	}
    //-----------------------------------------------------------------------------
	Bool MGStrOp::isDigital( CChar8* digtial )
	{
		DYNAMIC_ASSERT(digtial != NULL);
		if ( !digtial )
		{
			return false;
		}

		size_t length = strlen(digtial);
		for ( U32 i = 0; i < length; i++ )
		{
			char chr = *(digtial + i);
			if ( chr < '0' || chr > '9' )
			{
				return false;
			}
				
		}

		return true;
	}

    //-----------------------------------------------------------------------------
	Bool MGStrOp::isDigital( CChar16* digtial )
	{
		if ( !digtial )
		{
			return false;
		}
		size_t length = wcslen(digtial);
		for ( U32 i = 0; i < length; i++ )
		{
			Char16 chr = *(digtial + i);
			if ( chr < L'0' || chr > L'9' )
			{
				return false;
			}	
		}

		return true;
	}
	//-----------------------------------------------------------------------------
	I32 MGStrOp::sprintf( Char8* dest, int destSize, CChar8* format, ... )
	{
		if (!dest)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		if (!format)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		if (destSize <= 0 )
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		va_list args;
		va_start(args, format);
		dest[destSize-1] = '\0';
		return _vsnprintf_s( dest, destSize, destSize - 1,format, args);
	}
	//-----------------------------------------------------------------------------
	I32 MGStrOp::sprintf( Char16* dest, int destSize, Char16* format, ... )
	{
		if (!dest)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		if (!format)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		if (destSize <= 0)
		{
			DYNAMIC_ASSERT(false);
			return -1;
		}
		va_list args;
		va_start(args, format);
		dest[destSize-1] = L'\0';
		return _vsnwprintf_s( dest,destSize, destSize - 1, format, args);
	}

	//-----------------------------------------------------------------------------
	

    //-----------------------------------------------------------------------------
    Bool MGStrOp::toI32( CChar8* src,I32& dest)
    {
        DYNAMIC_ASSERT(src != 0);
        dest = atoi(src);
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toFlt(CChar8* src, Flt& dest)
    {
        dest = (Flt)atof(src);
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toDbl( CChar8* src,Dbl& dest )
    {
        dest = atof(src);
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toU32( CChar8* src,U32& dest )
    {
        I32 temp = atoi(src);
        dest = temp;
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toU64( CChar8* src,U64& dest )
    {
        I64 temp = _atoi64(src);
        dest = temp;
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toI32( CChar16* src,I32& dest )
    {
        dest = _wtoi(src);
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toFlt(CChar16* src, Flt& dest)
    {
        dest = (Flt)_wtof(src);
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toDbl( CChar16* src,Dbl& dest )
    {
        dest = _wtof(src);
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toU32( CChar16* src,U32& dest )
    {
        dest = _wtoi(src);
        return true;
    }
    //-----------------------------------------------------------------------------
    Bool MGStrOp::toU64( CChar16* src,U64& dest )
    {
        dest = _wtoi64(src);
        return true;
    }
	//-----------------------------------------------------------------------------
	U64 MGStrOp::toU64(CChar16* src)
	{
		I64 temp = _wtoi64(src);
		return temp;
	}
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( CChar16* src, std::string& dest, int size /*= -1*/, unsigned int codePage /*= CP_ACP*/ )
    {
        dest = "";
        if (!src)
            return;

        if (size<=0)
        {
            size = (int)wcslen(src);
            if (size<=0)
                return;
        }

        int maxsize = sizeof(wchar_t) * size;
        char* pSL = (char*)malloc(maxsize + 1);
        if (!pSL)
            return;
        pSL[maxsize] = 0;//it is char, so the size is (wchar size)
        int value = WideCharToMultiByte(codePage, 0, src, size, pSL, maxsize, NULL, NULL);
        if (value>=0 && value<=maxsize)
            pSL[value] = 0;

        dest = pSL;
        free(pSL);
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( I32 src,std::string& dest )
    {
        Char8 buff[11] = {0}; 
        _itoa_s(src,buff,10);
        dest = buff;
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( U32 src,std::string& dest )
    {
        Char8 buff[11] = {0};  
        _itoa_s(src,buff,10);
        dest = buff;
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( U64 src,std::string& dest )
    {
        Char8 buff[21] = {0}; 
        _ui64toa_s(src,buff,20,10);
        dest = buff;
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( Flt src,std::string& dest,I32 format )
    {
        Char8 tempFormat[32] = {0};
        sprintf_s(tempFormat,"%%.%df",format);
        Char8 buff[64] = {0};
        sprintf_s(buff,tempFormat,src);
        dest = buff;
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( CChar8* src, std::wstring& dest, int size /*= -1*/, unsigned int codePage /*= CP_ACP*/ )
    {
        dest = L'\0';
        if (!src)
        {
            return;
        }
        if (size <= 0)
        {
            size = (int)strlen(src);
            if (size <= 0)
                return;
        }

        int maxsize = sizeof(wchar_t) * (size + 1);
        wchar_t* pML = (wchar_t*)malloc(maxsize);
        if (!pML)
        {
            return;
        }

        pML[size] = 0;//it is wide char, so the size is (char size / 2)

        int value = MultiByteToWideChar(codePage, 0, src, size, pML, size);
        if (value>=0 && value<=size)
        {
            pML[value] = 0;
        }
        dest = pML;
        free(pML);
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( I32 src,std::wstring& dest )
    {
        Char16 buff[11] = {0}; 
        _itow_s(src,buff,10);
        dest = buff;
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( U32 src,std::wstring& dest )
    {
        Char16 buff[11] = {0}; 
        _itow_s(src,buff,10);
        dest = buff;
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( U64 src,std::wstring& dest )
    {
        Char16 buff[21] = {0}; 
        _ui64tow_s(src,buff,20,10);
        dest = buff;
    }
    //-----------------------------------------------------------------------------
    void MGStrOp::toString( Flt src,std::wstring& dest,I32 format )
    {
        Char16 tempFormat[32] = {0};
        swprintf_s(tempFormat,L"%%.%df",format);
        Char16 buff[64] = {0}; 
        swprintf_s(buff,tempFormat,src);
        dest = buff;

    }

	void MGStrOp::splitFilename( const std::string& qualifiedName, std::string& outBasename, std::string& outPath )
	{
		std::string path = qualifiedName;
		// Replace \ with / first
		std::replace( path.begin(), path.end(), '\\', '/' );
		// split based on final /
		unsigned int i = path.find_last_of('/');

		if (i == std::string::npos)
		{
			outPath.clear();
			outBasename = qualifiedName;
		}
		else
		{
			outBasename = path.substr(i+1, path.size() - i - 1);
			outPath = path.substr(0, i+1);
		}
	}

	void MGStrOp::splitTypename( const std::string& filename, std::string& outname, std::string& outtypename )
	{
		std::string name = filename;
		unsigned int i = name.find_first_of('.');
		if (i == std::string::npos)
		{
			outtypename.clear();
			outname = filename;
			//outtypename = filename;
		}
		else
		{
			outname = name.substr(0, i);
			outtypename = name.substr(i+1, name.size() - i - 1);
		}
	}

	void MGStrOp::str_replace( std::string& strBig, const std::string& strsrc, const std::string& strdst )
	{
		std::string::size_type pos = 0;
		std::string::size_type srclen = strsrc.size();
		std::string::size_type dstlen = strdst.size();

		while( (pos = strBig.find(strsrc, pos)) != std::string::npos )
		{
			strBig.replace(pos, srclen, strdst);
			pos += dstlen;
		}
	}
}


