/******************************************************************************/
#ifndef _NETUDPDRIVER_H_
#define _NETUDPDRIVER_H_
/******************************************************************************/

#include "NetDefine.h"

/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//�����ʼ������
	/******************************************************************************/
	struct NetUdpDriverInitParameter
	{
		Str		name ;                          //  ����
		Int		maxConnections;                 //  �����������
		Bool	isSetNetworkTimeout;            //  �Ƿ����糬ʱ���
		Int		waitAcceptSocketCount;          //  ������������
		Int		handleSocketEventThreadCount;   //  ����IOCP�����¼��߳��� ; @ -1 means auto
		UInt    sendNormalBuffSize;             //  ������ͨ���ݻ��泤��
		UInt    sendBigBuffSize;                //  ���ʹ������ݻ��泤��
		//UInt    sectionPacketSize;              //  �������ݻ��泤��
		Bool    isCloseWhenError;               //  �Ƿ�رյ��д����ʱ�� //һ��ͻ���=true,�����=false
		NetUdpDriverInitParameter()
			:maxConnections(1),
			isSetNetworkTimeout(true),
			waitAcceptSocketCount(5),
			handleSocketEventThreadCount(-1),
			sendNormalBuffSize(MG_NET_DEFAULT_SEND_BUFFER_SIZE),
			sendBigBuffSize(MG_NET_DEFAULT_SEND_BUFFER_SIZE),
			//sectionPacketSize(UDP_SECTION_PACKET_DEFAULT_SIZE),
			isCloseWhenError( true )
		{
		}
	};

	/******************************************************************************/
	//��������������
	/******************************************************************************/
	class NetUdpManager;
	class NetAddress;
	struct NetIocpUdpConnect;
	struct NetConnectInfo;
	class LogicSendPacket;
	class NetUdpDriver
	{
	public:
		NetUdpDriver(){}
		virtual	~NetUdpDriver() {}

	public:
		////////////////////////////////////////////////////////////////////////////////
		virtual Bool	                initialize( NetUdpDriverInitParameter& parameter, NetUdpManager* mgr ) = 0;
		virtual I32		                unInitialize() = 0;
		virtual Bool	                isInitialized() = 0;

		////////////////////////////////////////////////////////////////////////////////
		virtual I32		                update() = 0;

		////////////////////////////////////////////////////////////////////////////////
		virtual Bool	                setManager(NetUdpManager* mgr) = 0;
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
		virtual	LogicSendPacket*		getLogicSendPacket(Byte channel,Byte type,U16 sectionPacketSize,U32 logicId,U32 logicDataSize,Char8* data) = 0;

		////////////////////////////////////////////////////////////////////////////////
		// ��������
		virtual Bool					configNetLocalSockets(NetAddress* addr = NULL,I32 count = 0) = 0;
		// ��������, �����Ӷ��Զ�̵�ַ
		virtual void					connect(NetAddress* addr,U16 socketIndex,U32 logicId) = 0;
		// �ر�����
		virtual I32		                close(I32 id) = 0;
		// �ر���������
		virtual void	                closeAll() = 0;
		// �������������ⲿ��
		virtual void                    setCode(I32 id, U64 code) = 0;

	};
}

#endif