
/******************************************************************************/
#ifndef _FILEOP_H_
#define _FILEOP_H_
/******************************************************************************/

namespace MG
{
	/******************************************************************************/
	// 文件操作
	/******************************************************************************/
	class FileOp
	{
	public:

        // 得到完整的文件路径名
        static Str 	getFullFilePath(Str& pathName, Str& fileName);

        // 判断是否存在目录
        static Bool isExistDirectory(Str& pathName);
        // 创建目录
        static Bool createDirectory(Str& pathName);
        // 移除目录
        static Bool removeDirectory(Str& pathName);
        // 清空目录
        static Bool cleanDirectory(Str& pathName);


		//查找指定文件是否存在
        static Bool isExistFile(Str& pathName,Str& fileName);

        ////////////////////////////////////////////////////////////////////////////////////////

		//Bool	exist(Char8* fileName);

		//void*	read(Char8* fileName, I32 * fileSize = NULL);
		//I32		readBuf(Char8* fileName, void * buf, I32 ReadSize);
		//I32		write(Char8* fileName, void * buf, I32 writeSize);
		//I32		size(Char8* fileName);

		//void	init();
		//void	shutDown();
		//void	addPath(Char8* path);
		//void	removePath(Char8* path);

		//void*	openGroup(Char8* groupName, BOOLEAN loadInRam = FALSE);
		//void	closeGroup(Char8* groupName);

		//void*	openGroupFromMemory(Char8* pData, I32 dataSize);
		//void	closeGroupHandle(void* groupHandle);
	};
}

/******************************************************************************/

#endif