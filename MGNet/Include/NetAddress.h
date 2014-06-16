/******************************************************************************/
#ifndef _NETADDRESS_H_
#define _NETADDRESS_H_
/******************************************************************************/
#include "NetDefine.h"
/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//INetAddr 	网络连接地址
	/******************************************************************************/
	class NetAddress
	{
	public:
		NetAddress( );
		NetAddress( I32 port );											// 参数为本机字节序
		NetAddress( U32 ip, I32 port );									// 参数为本机字节序
		NetAddress( const Char8* ip, I32 port );                        // port参数为本机字节序
		NetAddress( const NetAddress& addr );

		const NetAddress& operator=( const NetAddress& addr );
		bool			operator==( const NetAddress& addr );

	public:
		CChar8*			getIPStrA() const;
		CChar*			getIPStr() const;
		CChar*			getPortStr() const;
		U32				getIP() const;									//本机字节序
		I32				getPort() const;								//本机字节序

		Bool			isValid();
		void			clear();
		void			setPort( I32 port );							// 参数为本机字节序
		void			setIP( CChar8* ipStr );
        U64             getUin();

		static void		inetNtoA( U32 ip, Char8* ipBuf, I32 bufSize );	//ip为网络字节序
		static U32		inetAddr( CChar8* ip );							//返回网络字节序
		static U32		net2Host( U32 val );
		static U32		host2Net( U32 val );
		static U16		net2Host( U16 val );
		static U16		host2Net( U16 val );

	private:
		Char8			mIPStrA[MG_NET_MAX_IP_LEN];
		Char			mIPStr[MG_NET_MAX_IP_LEN];
		Char			mPortStr[MG_NET_MAX_POTR_LEN];
		U32				mIP;										// 本机字节序
		I32				mPort;										// 本机字节序
	};
}
#endif