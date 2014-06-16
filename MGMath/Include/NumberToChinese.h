//******************************************************************************************
#ifndef _NUMBERTOCHINESE_H_
#define _NUMBERTOCHINESE_H_
//******************************************************************************************
namespace  MG
{
	class NumberToChinese
	{
	public:
		static Str8 convertNumToChinese(U32 num);
		static Str16 convertNumToChineseW(U32 num);
	private:
		//static const Char8 mUnite[5];
		static const Char8 mNum[10];
		static const Char16 mWnum[10];

	};
}
#endif //_NUMBERTOCHINESE_H_