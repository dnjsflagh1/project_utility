
/******************************************************************************/
#ifndef _FILEOP_H_
#define _FILEOP_H_
/******************************************************************************/

namespace MG
{
	/******************************************************************************/
	// �ļ�����
	/******************************************************************************/
	class FileOp
	{
	public:

        // �õ��������ļ�·����
        static Str 	getFullFilePath(Str& pathName, Str& fileName);

        // �ж��Ƿ����Ŀ¼
        static Bool isExistDirectory(Str& pathName);
        // ����Ŀ¼
        static Bool createDirectory(Str& pathName);
        // �Ƴ�Ŀ¼
        static Bool removeDirectory(Str& pathName);
        // ���Ŀ¼
        static Bool cleanDirectory(Str& pathName);


		//����ָ���ļ��Ƿ����
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