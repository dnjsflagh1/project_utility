/******************************************************************************/
#ifndef _NETADDRESS_H_
#define _NETADDRESS_H_
/******************************************************************************/
#include "NetDefine.h"
/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//INetAddr 	�������ӵ�ַ
	/******************************************************************************/
	class NetAddress
	{
	public:
		NetAddress( );
		NetAddress( I32 port );											// ����Ϊ�����ֽ���
		NetAddress( U32 ip, I32 port );									// ����Ϊ�����ֽ���
		NetAddress( const Char8* ip, I32 port );                        // port����Ϊ�����ֽ���
		NetAddress( const NetAddress& addr );

		const NetAddress& operator=( const NetAddress& addr );
		bool			operator==( const NetAddress& addr );

	public:
		CChar8*			getIPStrA() const;
		CChar*			getIPStr() const;
		CChar*			getPortStr() const;
		U32				getIP() const;									//�����ֽ���
		I32				getPort() const;								//�����ֽ���

		Bool			isValid();
		void			clear();
		void			setPort( I32 port );							// ����Ϊ�����ֽ���
		void			setIP( CChar8* ipStr );
        U64             getUin();

		static void		inetNtoA( U32 ip, Char8* ipBuf, I32 bufSize );	//ipΪ�����ֽ���
		static U32		inetAddr( CChar8* ip );							//���������ֽ���
		static U32		net2Host( U32 val );
		static U32		host2Net( U32 val );
		static U16		net2Host( U16 val );
		static U16		host2Net( U16 val );

	private:
		Char8			mIPStrA[MG_NET_MAX_IP_LEN];
		Char			mIPStr[MG_NET_MAX_IP_LEN];
		Char			mPortStr[MG_NET_MAX_POTR_LEN];
		U32				mIP;										// �����ֽ���
		I32				mPort;										// �����ֽ���
	};
}
#endif