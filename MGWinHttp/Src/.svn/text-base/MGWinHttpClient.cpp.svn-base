/******************************************************************************/
#include "stdafx.h"
#include "MGWinHttpClient.h"
#include "WinHttpClient.h"
/******************************************************************************/

namespace MG
{
	//-------------------------------------------------------------------------------
	MGWinHttpClient::MGWinHttpClient()
	{

	}
	//-------------------------------------------------------------------------------
	MGWinHttpClient::~MGWinHttpClient()
	{

	}
	//-------------------------------------------------------------------------------
	Bool MGWinHttpClient::sendHttpQuest( Str16 url, Str16& response )
	{
		//Bool ret = false;
		WinHttpClient httpClient(url);
		Bool ret = httpClient.SendHttpRequest();
		if (ret)
		{
			response = httpClient.GetResponseContent();
		}
		return ret;
	}

	//-------------------------------------------------------------------------------
}