/******************************************************************************/

#ifndef _MGPACKMANAGER_H_
#define _MGPACKMANAGER_H_

/******************************************************************************/

#include "IMGPackFileSet.h"

/******************************************************************************/

namespace MG
{
	/******************************************************************************/
	// 文件系统
	/******************************************************************************/
	class MGPackManager 
	{
	public:

		MGPackManager();
		virtual ~MGPackManager();

	public:
		//////////////////////////////////////////////////////////////////////////
		// 打包工具使用
		// 打包文件
		bool				packRes(const std::string& src, const std::string& des, const char* password);
		//////////////////////////////////////////////////////////////////////////
		// 更新包使用
		// 解压压缩包到硬盘
		bool				unZip( const std::string& zipname, const std::string& unzipfolder );

		// 合并压缩包
		bool				injectPack(const std::string& unzipfolder, const std::string& tempClienResfolder, 
								StringMap &filemap);
		
		//////////////////////////////////////////////////////////////////////////
		// 加载打包文件集
		IMGPackFileSet* 	loadPackFileSet( const std::string& filename, const char* password );
		// 卸载打包文件集
		void				unloadPackFileSet( IMGPackFileSet* file );

	protected:

	private:
		
	};

}

/******************************************************************************/
#endif

