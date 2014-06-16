/******************************************************************************/
#include "stdafx.h"
#include "MsgBox.h"
/******************************************************************************/

namespace MG
{
    //-----------------------------------------------------------------------------
    void MsgBox::show(CChar8* text, CChar8* title)
    {
		::MessageBoxExA(NULL,text,title,MB_OK,0);
    }

	void MsgBox::show( CChar16* text, CChar16* title/*=NULL*/ )
	{
		::MessageBoxExW(NULL,text,title,MB_OK,0);
	}
}


