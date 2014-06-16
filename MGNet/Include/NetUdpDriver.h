/******************************************************************************/
#ifndef _NETUDPDRIVER_H_
#define _NETUDPDRIVER_H_
/******************************************************************************/

#include "NetDefine.h"

/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//网络初始化参数
	/******************************************************************************/
	struct NetUdpDriverInitParameter
	{
		Str		name ;                          //  名字
		Int		maxConnections;                 //  最大连接数量
		Bool	isSetNetworkTimeout;            //  是否网络超时检测
		Int		waitAcceptSocketCount;          //  监听连接数量
		Int		handleSocketEventThreadCount;   //  处理IOCP网络事件线程数 ; @ -1 means auto
		UInt    sendNormalBuffSize;             //  发送普通数据缓存长度
		UInt    sendBigBuffSize;                //  发送大型数据缓存长度
		//UInt    sectionPacketSize;              //  接收数据缓存长度
		Bool    isCloseWhenError;               //  是否关闭当有错误的时候 //一般客户端=true,服务端=false
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
	//网络驱动管理者
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
		// 连接网络
		virtual Bool					configNetLocalSockets(NetAddress* addr = NULL,I32 count = 0) = 0;
		// 连接网络, 可连接多个远程地址
		virtual void					connect(NetAddress* addr,U16 socketIndex,U32 logicId) = 0;
		// 关闭网络
		virtual I32		                close(I32 id) = 0;
		// 关闭所有网络
		virtual void	                closeAll() = 0;
		// 设置网络连接外部码
		virtual void                    setCode(I32 id, U64 code) = 0;

	};
}

#endif