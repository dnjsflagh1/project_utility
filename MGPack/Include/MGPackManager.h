/******************************************************************************/

#ifndef _MGPACKMANAGER_H_
#define _MGPACKMANAGER_H_

/******************************************************************************/

#include "IMGPackFileSet.h"

/******************************************************************************/

namespace MG
{
	/******************************************************************************/
	// �ļ�ϵͳ
	/******************************************************************************/
	class MGPackManager 
	{
	public:

		MGPackManager();
		virtual ~MGPackManager();

	public:
		//////////////////////////////////////////////////////////////////////////
		// �������ʹ��
		// ����ļ�
		bool				packRes(const std::string& src, const std::string& des, const char* password);
		//////////////////////////////////////////////////////////////////////////
		// ���°�ʹ��
		// ��ѹѹ������Ӳ��
		bool				unZip( const std::string& zipname, const std::string& unzipfolder );

		// �ϲ�ѹ����
		bool				injectPack(const std::string& unzipfolder, const std::string& tempClienResfolder, 
								StringMap &filemap);
		
		//////////////////////////////////////////////////////////////////////////
		// ���ش���ļ���
		IMGPackFileSet* 	loadPackFileSet( const std::string& filename, const char* password );
		// ж�ش���ļ���
		void				unloadPackFileSet( IMGPackFileSet* file );

	protected:

	private:
		
	};

}

/******************************************************************************/
#endif

