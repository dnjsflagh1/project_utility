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
		// ����ļ�(��������õ�)
		virtual bool	packRes(const std::string& src, const std::string& des, const char* password) = 0;

		//////////////////////////////////////////////////////////////////////////
		// ���°��ϲ��ӿ�
		// ��ѹѹ������Ӳ��
		virtual bool	unZip( const std::string& zipname, const std::string& unzipfolder ) = 0;

		// �ϲ�ѹ����
	    virtual	bool	injectPack(const std::string& unzipfolder, const std::string& tempClienResfolder, 
							StringMap &filemap) = 0;

		//////////////////////////////////////////////////////////////////////////
		// �����ļ���Դ
		virtual void	load( const std::string& filename, const char* password ) = 0;
		// ж���ļ���Դ
		virtual void	unLoad() = 0;
		//////////////////////////////////////////////////////////////////////////////////////////
		// ��ȡ��

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual Bool	extractPackFile( const std::string& pathfilename, MGExtractedPackFile& file ) = 0;

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual Bool	extractPackFile( const std::string& pathfilename, char*& buf, Int& size ) = 0;

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual Bool	extractPackFile( const std::string& pathfilename, wchar_t*& buf ) = 0;

		// �ļ��Ƿ����
		virtual bool	isFileExist(const std::string& filename) = 0;

		// �õ�·����Ӧѹ���������б�
		virtual StringMap&				getPathPackTypeMap() = 0;
		// �õ�·����Ӧ�ļ��б�map
		virtual ClassifyPathMap*&		getPathFileInfoMap() = 0;
		// �õ�·����Ӧ��ͬ���͵��ļ��б�map
		virtual ClassifyPathTypeMap*&	getPathFileInfoTypeMap() = 0;
		// �õ��ļ�·��ӳ���б�
		virtual StringMap&				getFilePathMap() = 0;

		//////////////////////////////////////////////////////////////////////////////////////////
		// ���°�����


	};

}

#endif