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
    // �ļ�ϵͳ��
    /******************************************************************************/
	class MGPackFileSet : public IMGPackFileSet
    {
	public:

		MGPackFileSet();
		virtual ~MGPackFileSet();
		
	public:
		//////////////////////////////////////////////////////////////////////////
		// ������߽ӿ�
		// ����ļ�
		virtual bool	packRes(const std::string& src, const std::string& des, const char* password);

		//////////////////////////////////////////////////////////////////////////
		// ���°��ϲ��ӿ�
		// ��ѹѹ������Ӳ��
		virtual bool	unZip( const std::string& zipname, const std::string& unzipfolder );

		// �ϲ�ѹ����
		virtual	bool	injectPack(const std::string& unzipfolder, const std::string& tempClienResfolder, 
							StringMap &filemap);

		//////////////////////////////////////////////////////////////////////////
		// �����ļ���Դ
		virtual void	load( const std::string& filename, const char* password );
		// ж���ļ���Դ
		virtual void	unLoad();

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual Bool	extractPackFile( const std::string& pathfilename, MGExtractedPackFile& file );

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual Bool	extractPackFile( const std::string& pathfilename, char*& buf, Int& size );

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual Bool	extractPackFile( const std::string& pathfilename, wchar_t*& buf );

		// �ļ��Ƿ����
		virtual bool	isFileExist(const std::string& filename);

		// �õ�·���б�
		virtual StringMap&				getPathPackTypeMap();
		// �õ�·����Ӧ�ļ��б�map
		virtual ClassifyPathMap*&		getPathFileInfoMap();
		// �õ�·����Ӧ��ͬ���͵��ļ��б�map
		virtual ClassifyPathTypeMap*&	getPathFileInfoTypeMap();
		// �õ��ļ�·��ӳ���б�
		virtual StringMap&				getFilePathMap();
	private:
		// Zip 
		CZipArchive				mArchive;
		// ·����Ӧ�ļ��б�map
		ClassifyPathMap*		mPathFileInfoMap;
		// ·����Ӧ��ͬ���͵��ļ��б�map
		ClassifyPathTypeMap*	mPathFileInfoTypeMap;
		// ���е����ļ����������ļ�����ӳ��
		StringMap				mFilePathMap;
		// ����·��(����Config/Base..�ⲿ�ַ���)��Ӧѹ���������б�
		StringMap				mPathPackTypeMap;
		// �������ļ���(����Config/Base..�����ⲿ�ַ���)
		// ��Ӧ��ѹ������������ļ���(��ȥConfig/Base..��).
		StringMap				mFullNameResName;
    };

}

/******************************************************************************/
#endif

