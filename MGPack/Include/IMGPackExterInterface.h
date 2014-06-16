#ifndef __IMGPACK_EXTER_INTERFACE__
#define __IMGPACK_EXTER_INTERFACE__

#include "MGPackDefine.h"

namespace MG
{
	// �ⲿ����Դ������
	class IMGExternalPackManager
	{
	public:
		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual bool		extractPackFile( const std::string& pathfilename, MGExtractedPackFile& file ) = 0;

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual bool		extractPackFile( const std::string& pathfilename, char*& buf, int& size ) = 0;

		// ��ѹ���ļ����ڴ�
		// �ļ���������·��
		virtual bool		extractPackFile( const std::string& pathfilename, wchar_t*& buf ) = 0;

		// ѹ�������Ƿ�����Դ�ļ�
		virtual bool		isPackResEmpty() = 0;
	};

} 
#endif