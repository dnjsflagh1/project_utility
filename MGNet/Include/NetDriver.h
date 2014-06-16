/******************************************************************************/
#ifndef _NETDRIVER_H_
#define _NETDRIVER_H_
/******************************************************************************/

#include "NetDefine.h"

/******************************************************************************/
namespace MG
{

    /******************************************************************************/
    //�����ʼ������
    /******************************************************************************/
	struct NetDriverInitParameter
	{
		Str			name ;                          //  ����
		Int			maxConnections;                 //  �����������
		Bool		isSetNetworkTimeout;            //  �Ƿ����糬ʱ���
        Int		    networkTimeoutTouchTime;        //  ���糬ʱ��ʼ���ʱ��
        Int		    networkTimeoutDuration;         //  ���糬ʱ���ʱ��
		Int			waitAcceptSocketCount;          //  ������������
		Int			handleSocketEventThreadCount;   //  ����IOCP�����¼��߳��� ; @ -1 means auto
        UInt		sendNormalBuffSize;             //  ������ͨ���ݻ��泤��
        UInt		sendBigBuffSize;                //  ���ʹ������ݻ��泤��
        UInt		recvBuffSize;                   //  �������ݻ��泤��
        Bool		isCloseWhenError;               //  �Ƿ�رյ��д����ʱ�� //һ��ͻ���=true,�����=false
		Int			socketSendBuffSize;				//	socket���ͻ�������С
		Int			socketRecvBuffSize;				//	socket���ջ�������С
		Int			driverType;						//	driver����

		NetDriverInitParameter()
			:maxConnections(1),
			isSetNetworkTimeout(true),
            networkTimeoutTouchTime(MG_NET_CLIENT_TIMEOUT_TOUCH_TIME),
            networkTimeoutDuration(MG_NET_CLIENT_TIMEOUT_DURATION),
			waitAcceptSocketCount(5),
			handleSocketEventThreadCount(-1),
            sendNormalBuffSize(MG_NET_DEFAULT_SEND_BUFFER_SIZE),
            sendBigBuffSize(MG_NET_DEFAULT_SEND_BUFFER_SIZE),
            recvBuffSize(MG_NET_DEFAULT_RECV_BUFFER_SIZE),
			isCloseWhenError( true ),
			socketSendBuffSize(MG_NET_MAX_CLIENT_SOCKET_SENDBUFF_SIZE),
			socketRecvBuffSize(MG_NET_MAX_CLIENT_SOCKET_RECVBUFF_SIZE),
			driverType(0)
		{
		}
	};


    /******************************************************************************/
    //��������������
    /******************************************************************************/
	class NetManager;
	class NetAddress;
	struct NetIocpConnect;
	struct NetConnectInfo;
	class NetDriver
	{
	public:
		NetDriver(){}
		virtual						~NetDriver() {}
	public:

        ////////////////////////////////////////////////////////////////////////////////

		virtual Bool	                initialize( NetDriverInitParameter& parameter, NetManager* mgr ) = 0;
		virtual I32		                unInitialize() = 0;
		virtual Bool	                isInitialized() = 0;

        ////////////////////////////////////////////////////////////////////////////////

		virtual I32		                update() = 0;

        ////////////////////////////////////////////////////////////////////////////////

		virtual Bool	                setManager(NetManager* mgr) = 0;
		virtual void	                setHandle(I32 handle) = 0;
		virtual I32		                getHandle() = 0;
		virtual Str		                getName() const = 0;
		virtual CChar*	                getDriverDesc() const = 0;

        ////////////////////////////////////////////////////////////////////////////////


		virtual	U64 	                getAddressUin(I32 id) = 0;
        virtual	NetAddress              getAddress(I32 id) = 0;

		virtual	NetConnectInfo*         getNetInfo() = 0;
		virtual	NetConnectInfo*         getConnectInfo(I32 id) = 0;
		virtual	I32				        getCurrConnectionCount() = 0;
		

        ////////////////////////////////////////////////////////////////////////////////

        // ��������
		virtual I32		                connect(NetAddress* addr) = 0;
        // �ر�����
		virtual I32		                close(I32 id) = 0;
        // �ر���������
		virtual void	                closeAll() = 0;
        // ������������
		virtual I32		                listen( NetAddress* addr) = 0;
        // ��������
        virtual Bool                    send(NetOnceSendConfig* config) = 0;
        // �������������ⲿ��
        virtual void                    setCode(I32 id, U64 code) = 0;

	};
}

#endif