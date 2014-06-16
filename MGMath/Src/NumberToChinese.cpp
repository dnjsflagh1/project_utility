//******************************************************************************************
#include "stdafx.h"
#include "NumberToChinese.h"
//******************************************************************************************
namespace MG
{
	//const Char8 NumberToChinese::mUnite[5] = {'十','百','千','万','亿'};
	const Char16 NumberToChinese::mWnum[10] = {L'零',L'一',L'二',L'三',L'四',L'五',L'六',L'七',L'八',L'九'};
	const Char8 NumberToChinese::mNum[10] = { (char)('零'),
                                              (char)('一'),
                                              (char)('二'),
                                              (char)('三'),
                                              (char)('四'),
                                              (char)('五'),
                                              (char)('六'),
                                              (char)('七'),
                                              (char)('八'),
                                              (char)('九') };
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