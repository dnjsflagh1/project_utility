/******************************************************************************/
#ifndef _NETDRIVER_H_
#define _NETDRIVER_H_
/******************************************************************************/

#include "NetDefine.h"

/******************************************************************************/
namespace MG
{

    /******************************************************************************/
    //网络初始化参数
    /******************************************************************************/
	struct NetDriverInitParameter
	{
		Str			name ;                          //  名字
		Int			maxConnections;                 //  最大连接数量
		Bool		isSetNetworkTimeout;            //  是否网络超时检测
        Int		    networkTimeoutTouchTime;        //  网络超时开始检测时间
        Int		    networkTimeoutDuration;         //  网络超时检测时间
		Int			waitAcceptSocketCount;          //  监听连接数量
		Int			handleSocketEventThreadCount;   //  处理IOCP网络事件线程数 ; @ -1 means auto
        UInt		sendNormalBuffSize;             //  发送普通数据缓存长度
        UInt		sendBigBuffSize;                //  发送大型数据缓存长度
        UInt		recvBuffSize;                   //  接收数据缓存长度
        Bool		isCloseWhenError;               //  是否关闭当有错误的时候 //一般客户端=true,服务端=false
		Int			socketSendBuffSize;				//	socket发送缓冲区大小
		Int			socketRecvBuffSize;				//	socket接收缓冲区大小
		Int			driverType;						//	driver类型

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
    //网络驱动管理者
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

        // 连接网络
		virtual I32		                connect(NetAddress* addr) = 0;
        // 关闭网络
		virtual I32		                close(I32 id) = 0;
        // 关闭所有网络
		virtual void	                closeAll() = 0;
        // 监听网络连接
		virtual I32		                listen( NetAddress* addr) = 0;
        // 发送数据
        virtual Bool                    send(NetOnceSendConfig* config) = 0;
        // 设置网络连接外部码
        virtual void                    setCode(I32 id, U64 code) = 0;

	};
}

#endif