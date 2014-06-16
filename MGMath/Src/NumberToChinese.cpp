//******************************************************************************************
#include "stdafx.h"
#include "NumberToChinese.h"
//******************************************************************************************
namespace MG
{
	//const Char8 NumberToChinese::mUnite[5] = {'ʮ','��','ǧ','��','��'};
	const Char16 NumberToChinese::mWnum[10] = {L'��',L'һ',L'��',L'��',L'��',L'��',L'��',L'��',L'��',L'��'};
	const Char8 NumberToChinese::mNum[10] = { (char)('��'),
                                              (char)('һ'),
                                              (char)('��'),
                                              (char)('��'),
                                              (char)('��'),
                                              (char)('��'),
                                              (char)('��'),
                                              (char)('��'),
                                              (char)('��'),
                                              (char)('��') };
	//-----------------------------------------------------------------------------
	Str8 NumberToChinese::convertNumToChinese( U32 num)
	{
		Str8 ret = "";
		U32 temp = 0;
		/*I32 unit1 = 0;
		I32 unit2 = 0;
		I32 value[4] = {0};*/
		while(num != 0)
		{			
			//if (unit1 == 3)
			//{
			//	unit2++;
			//	unit1 = 0;
			//}
			
			temp = num % 10;
			ret = mNum[temp] + ret; 
			num /= 10;
			
			//unit1++;

		}
		return ret;
	}
	//-----------------------------------------------------------------------------
	Str16 NumberToChinese::convertNumToChineseW( U32 num)
	{
		Str16 ret = L"";
		U32 temp = 0;
		while(num != 0)
		{
			temp = num % 10;
			ret = mWnum[temp] + ret; 
			num /= 10;	
		}
		return ret;
	}
	//-----------------------------------------------------------------------------
}