/******************************************************************************/

#include "stdafx.h"
#include "MGPackManager.h"
#include <imagehlp.h>

/******************************************************************************/

namespace MG
{
	//-------------------------------------------------------------------
	MGPackManager::MGPackManager()
	{

	}

	//-------------------------------------------------------------------
	MGPackManager::~MGPackManager()
	{

	}

	//-------------------------------------------------------------------
	bool MGPackManager::packRes( const std::string& src, const std::string& des, const char* password )
	{
		IMGPackFileSet* packFileSet = MG_NEW MGPackFileSet();

		char srctemp[1024], destemp[1024];
		wsprintf(srctemp, "%s\\%s", src.c_str(), "Config");
		wsprintf(destemp, "%s\\%s", des.c_str(), "Config");

		// 创建目录字符串
		char configtemp[1024];
		wsprintf(configtemp, "%s\\", destemp);
		// 以防路径不存在,创建目录(路径后面一定要\\)
		if (!MakeSureDirectoryPathExists(configtemp)) 
			return false;

		if (!packFileSet->packRes(srctemp, destemp, password))
			return false;



		wsprintf(srctemp, "%s\\%s", src.c_str(), "Media");
		wsprintf(destemp, "%s\\%s", des.c_str(), "Media");

		// 创建目录字符串
		char mediatemp[1024];
		wsprintf(mediatemp, "%s\\", destemp);
		// 以防路径不存在,创建目录(路径后面一定要\\)
		if (!MakeSureDirectoryPathExists(mediatemp)) 
			return false;

		if (!packFileSet->packRes(srctemp, destemp, password))
			return false;

		MG_SAFE_DELETE(packFileSet);

		return true;
	}

	//-------------------------------------------------------------------
	bool MGPackManager::unZip( const std::string& zipname, const std::string& unzipfolder )
	{
		IMGPackFileSet* packFileSet = MG_NEW MGPackFileSet();
		if (!packFileSet->unZip(zipname, unzipfolder))
		{
			MG_SAFE_DELETE(packFileSet);
			return false;
		}
		return true;
	}

	//-------------------------------------------------------------------
	bool MGPackManager::injectPack( const std::string& unzipfolder, const std::string& tempClienResfolder, 
							StringMap &filemap )
	{
		IMGPackFileSet* packFileSet = MG_NEW MGPackFileSet();
		if (!packFileSet->injectPack(unzipfolder, tempClienResfolder, filemap))
		{
			MG_SAFE_DELETE(packFileSet);
			return false;
		}
		return true;
	}

	//-------------------------------------------------------------------
	IMGPackFileSet* MGPackManager::loadPackFileSet( const std::string& filename, const char* password )
	{
		IMGPackFileSet* packFileSet = MG_NEW MGPackFileSet();
		packFileSet->load( filename, password );
		return packFileSet;
	}

	//-------------------------------------------------------------------
	void MGPackManager::unloadPackFileSet( IMGPackFileSet* file )
	{
		MG_SAFE_DELETE(file);
	}


}

/******************************************************************************/


