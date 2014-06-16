/******************************************************************************/

#ifndef _IMGPACKFILE_H_
#define _IMGPACKFILE_H_

/******************************************************************************/

#include "MGPackDefine.h"

namespace MG
{

	class IMGPackFileSet
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// 打包文件(打包工具用到)
		virtual bool	packRes(const std::string& src, const std::string& des, const char* password) = 0;

		//////////////////////////////////////////////////////////////////////////
		// 更新包合并接口
		// 解压压缩包到硬盘
		virtual bool	unZip( const std::string& zipname, const std::string& unzipfolder ) = 0;

		// 合并压缩包
	    virtual	bool	injectPack(const std::string& unzipfolder, const std::string& tempClienResfolder, 
							StringMap &filemap) = 0;

		//////////////////////////////////////////////////////////////////////////
		// 加载文件资源
		virtual void	load( const std::string& filename, const char* password ) = 0;
		// 卸载文件资源
		virtual void	unLoad() = 0;
		//////////////////////////////////////////////////////////////////////////////////////////
		// 读取包

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual Bool	extractPackFile( const std::string& pathfilename, MGExtractedPackFile& file ) = 0;

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual Bool	extractPackFile( const std::string& pathfilename, char*& buf, Int& size ) = 0;

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual Bool	extractPackFile( const std::string& pathfilename, wchar_t*& buf ) = 0;

		// 文件是否存在
		virtual bool	isFileExist(const std::string& filename) = 0;

		// 得到路径对应压缩包类型列表
		virtual StringMap&				getPathPackTypeMap() = 0;
		// 得到路径对应文件列表map
		virtual ClassifyPathMap*&		getPathFileInfoMap() = 0;
		// 得到路径对应不同类型的文件列表map
		virtual ClassifyPathTypeMap*&	getPathFileInfoTypeMap() = 0;
		// 得到文件路径映射列表
		virtual StringMap&				getFilePathMap() = 0;

		//////////////////////////////////////////////////////////////////////////////////////////
		// 更新包方法


	};

}

#endif