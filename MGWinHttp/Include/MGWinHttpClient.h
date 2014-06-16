/******************************************************************************/
#ifndef _MGWINHTTPCLIENT_H_
#define _MGWINHTTPCLIENT_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	class MGWinHttpClient
	{
	public:
		MGWinHttpClient();
		~MGWinHttpClient();
	public:
		Bool sendHttpQuest( Str16 url, Str16& response );
	};

}


#endif
/******************************************************************************/