/******************************************************************************/
#include "stdafx.h"
#include "DllLoader.h"
#include "MGStrOp.h"
/******************************************************************************/


namespace MG
{
	//-----------------------------------------------------------------------------
	DllLoader::DllLoader()
		:mHinst(NULL)
	{
	}
	//-----------------------------------------------------------------------------
	DllLoader::~DllLoader()
	{
		unload();
	}
	//-----------------------------------------------------------------------------
	bool DllLoader::load(const char* filePath, bool isFirst)
	{
        mHinst = NULL;

	#ifdef UNICODE
        std::wstring wChar;
        MGStrOp::toString(filePath,wChar);
		if (isFirst==false)
            mHinst = GetModuleHandle(wChar.c_str());
		if (mHinst==NULL)
		{
			mHinst = LoadLibrary(wChar.c_str());
		}
	#else
		if (isFirst==false)
			mHinst = GetModuleHandle(filePath);
		if (mHinst==NULL)
		{
			mHinst = LoadLibrary(filePath);
		}
	#endif // !UNICODE

		return mHinst != NULL;
	}
    //-----------------------------------------------------------------------------
    bool DllLoader::load(const wchar_t* filePath, bool isFirst)
    {
        std::string chars;
        MGStrOp::toString(filePath,chars);
        return load(chars.c_str());
    }
	//-----------------------------------------------------------------------------
	bool DllLoader::unload()
	{
		bool result = true;
		if (mHinst!=NULL)
		{
			result = FreeLibrary(mHinst) != 0;
		}
		mHinst = NULL;

		return result;
	}
	//-----------------------------------------------------------------------------
	FARPROC DllLoader::getProcAddress(const char* name)
	{
		if (mHinst == NULL) 
			return NULL;
	    
		FARPROC farProc =  GetProcAddress(mHinst, name); 

		return farProc;
	}
    //-----------------------------------------------------------------------------
    FARPROC DllLoader::getProcAddress(const wchar_t* name)
    {
        std::string chars;
        MGStrOp::toString(name,chars);
        return getProcAddress(chars.c_str());
    }


}