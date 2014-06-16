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

		// ����Ŀ¼�ַ���
		char configtemp[1024];
		wsprintf(configtemp, "%s\\", destemp);
		// �Է�·��������,����Ŀ¼(·������һ��Ҫ\\)
		if (!MakeSureDirectoryPathExists(configtemp)) 
			return false;

		if (!packFileSet->packRes(srctemp, destemp, password))
			return false;



		wsprintf(srctemp, "%s\\%s", src.c_str(), "Media");
		wsprintf(destemp, "%s\\%s", des.c_str(), "Media");

		// ����Ŀ¼�ַ���
		char mediatemp[1024];
		wsprintf(mediatemp, "%s\\", destemp);
		// �Է�·��������,����Ŀ¼(·������һ��Ҫ\\)
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


