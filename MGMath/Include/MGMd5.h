/************************************************************
MD5У��ͼ���С����
************************************************************/

#ifndef __MD5_INCLUDED__
#define __MD5_INCLUDED__

namespace MG
{
	//MD5ժҪֵ�ṹ��
	typedef struct MD5VAL_STRUCT
	{
		unsigned int a;
		unsigned int b;
		unsigned int c;
		unsigned int d;
	} MD5VAL;

	class  MGMd5
	{
	public:
		//�����ַ�����MD5ֵ(����ָ���������ɺ�������)
		static MD5VAL		md5(char * str, unsigned int size=0);

		//MD5�ļ�ժҪ
		static MD5VAL		md5File(FILE * fpin);

		//�����ַ�����MD5ֵ(����ָ���������ɺ�������)
		static std::string	md5Str(char * str, unsigned int size=0);

		//MD5�ļ�ժҪ
		static std::string	md5FileStr(FILE * fpin);

		static unsigned int	conv(unsigned int a);

	};
	

}

#endif

