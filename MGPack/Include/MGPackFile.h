/******************************************************************************/

#ifndef _MGPACKFILE_H_
#define _MGPACKFILE_H_

/******************************************************************************/

#include "IMGPackFileSet.h"
#include "ZipArchive.h"


/******************************************************************************/
namespace MG
{
    /******************************************************************************/
    // 文件系统集
    /******************************************************************************/
	class MGPackFileSet : public IMGPackFileSet
    {
	public:

		MGPackFileSet();
		virtual ~MGPackFileSet();
		
	public:
		//////////////////////////////////////////////////////////////////////////
		// 打包工具接口
		// 打包文件
		virtual bool	packRes(const std::string& src, const std::string& des, const char* password);

		//////////////////////////////////////////////////////////////////////////
		// 更新包合并接口
		// 解压压缩包到硬盘
		virtual bool	unZip( const std::string& zipname, const std::string& unzipfolder );

		// 合并压缩包
		virtual	bool	injectPack(const std::string& unzipfolder, const std::string& tempClienResfolder, 
							StringMap &filemap);

		//////////////////////////////////////////////////////////////////////////
		// 加载文件资源
		virtual void	load( const std::string& filename, const char* password );
		// 卸载文件资源
		virtual void	unLoad();

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual Bool	extractPackFile( const std::string& pathfilename, MGExtractedPackFile& file );

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual Bool	extractPackFile( const std::string& pathfilename, char*& buf, Int& size );

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual Bool	extractPackFile( const std::string& pathfilename, wchar_t*& buf );

		// 文件是否存在
		virtual bool	isFileExist(const std::string& filename);

		// 得到路径列表
		virtual StringMap&				getPathPackTypeMap();
		// 得到路径对应文件列表map
		virtual ClassifyPathMap*&		getPathFileInfoMap();
		// 得到路径对应不同类型的文件列表map
		virtual ClassifyPathTypeMap*&	getPathFileInfoTypeMap();
		// 得到文件路径映射列表
		virtual StringMap&				getFilePathMap();
	private:
		// Zip 
		CZipArchive				mArchive;
		// 路径对应文件列表map
		ClassifyPathMap*		mPathFileInfoMap;
		// 路径对应不同类型的文件列表map
		ClassifyPathTypeMap*	mPathFileInfoTypeMap;
		// 所有单独文件名和完整文件名的映射
		StringMap				mFilePathMap;
		// 完整路径(包括Config/Base..外部字符串)对应压缩包类型列表
		StringMap				mPathPackTypeMap;
		// 完整的文件名(包括Config/Base..这类外部字符串)
		// 对应在压缩包里的完整文件名(除去Config/Base..等).
		StringMap				mFullNameResName;
    };

}

/******************************************************************************/
#endif

