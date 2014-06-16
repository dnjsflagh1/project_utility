#ifndef __IMGPACK_EXTER_INTERFACE__
#define __IMGPACK_EXTER_INTERFACE__

#include "MGPackDefine.h"

namespace MG
{
	// 外部的资源管理器
	class IMGExternalPackManager
	{
	public:
		// 解压缩文件到内存
		// 文件名是完整路径
		virtual bool		extractPackFile( const std::string& pathfilename, MGExtractedPackFile& file ) = 0;

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual bool		extractPackFile( const std::string& pathfilename, char*& buf, int& size ) = 0;

		// 解压缩文件到内存
		// 文件名是完整路径
		virtual bool		extractPackFile( const std::string& pathfilename, wchar_t*& buf ) = 0;

		// 压缩包里是否有资源文件
		virtual bool		isPackResEmpty() = 0;
	};

} 
#endif