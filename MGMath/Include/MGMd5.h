/************************************************************
MD5校验和计算小程序
************************************************************/

#ifndef __MD5_INCLUDED__
#define __MD5_INCLUDED__

namespace MG
{
	//MD5摘要值结构体
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
		//计算字符串的MD5值(若不指定长度则由函数计算)
		static MD5VAL		md5(char * str, unsigned int size=0);

		//MD5文件摘要
		static MD5VAL		md5File(FILE * fpin);

		//计算字符串的MD5值(若不指定长度则由函数计算)
		static std::string	md5Str(char * str, unsigned int size=0);

		//MD5文件摘要
		static std::string	md5FileStr(FILE * fpin);

		static unsigned int	conv(unsigned int a);

	};
	

}

#endif

