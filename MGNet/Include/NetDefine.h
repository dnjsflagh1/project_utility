/******************************************************************************/
#ifndef _NETDEFINE_H_
#define _NETDEFINE_H_
/******************************************************************************/


#include "LogSystem.h"
#include "MsgBox.h"
#include "SharedPtr.h"
#include "RamAlloc.h"

/******************************************************************************/
namespace MG
{
	enum NET_EVENT_TYPE
	{
		NET_IDLE,
		NET_CONNECT,
		NET_ACCEPT,
		NET_SEND,
        NET_RECV,
		NET_CLOSE,
		NET_ERR,

		NET_UDP_IDLE,
		NET_UDP_CONNECT,
		NET_UDP_ACCEPT,
		NET_UDP_SEND,
		NET_UDP_RECV,
		NET_UDP_CLOSE,
		NET_UDP_ERR,
	};
    
	#define	MG_NET_MAX_IP_LEN			32
	#define	MG_NET_MAX_POTR_LEN			16

	#define	MG_NET_ADDR_IP_ANY			0
	#define	MG_NET_ADDR_IP_ANY_STR_A	"0.0.0.0"

	#ifndef MG_NET_BYTE_ORDER
	#error Error: please include MGDefine.h before this file.
	#endif

    ///////////////////////////////////////////////////////////////////////
    // 数据包头标签
    #define MG_NET_PACKET_TAG                       222
    

    ///////////////////////////////////////////////////////////////////////
    // 网络超时检测
    #define MG_NET_PACKET_TAG                       222

    // 开始检测心跳间隔
    #define MG_NET_TIMEOUT_CHECK_HEARTBEAT          3000

    // 开始触发检测时间
    #define MG_NET_CLIENT_TIMEOUT_TOUCH_TIME        5000
    #define MG_NET_SERVER_TIMEOUT_TOUCH_TIME        1000*60*5

    // 超时检测时间
    #define MG_NET_CLIENT_TIMEOUT_DURATION          30000
    #define MG_NET_SERVER_TIMEOUT_DURATION          600000
    
    ///////////////////////////////////////////////////////////////////////

    // 默认发送数据缓存大小
    #define MG_NET_DEFAULT_SEND_BUFFER_SIZE	        1024
    // 默认接收数据缓存
	#define MG_NET_DEFAULT_RECV_BUFFER_SIZE	        1024*20

    //IOCP 单连接的数据缓冲区大小 客户端
    #define MG_NET_BUFFER_SIZE_CLIENT	            1024*20 //(20kb)//1000 Client == 20mb
    //IOCP 单连接的数据缓冲区大小 服务端
    #define MG_NET_BUFFER_SIZE_SERVER	            1024*20*1000 // 20mb
    //IOCP 最大的线程数
	#define MG_NET_MAX_THREADS 16
    //网络最大的服务器数量
	#define MG_NET_MAX_SERVERS 32
    //网络最大的客户端数量
	#define MG_NET_MAX_CLIENTS 10000

    ///////////////////////////////////////////////////////////////////////

    //网络最大的合并数据包数量
    #define MG_NET_MAX_COMBINE_PACKET_COUNT         100

	///////////////////////////////////////////////////////////////////////

	//SOCKET缓冲区大小
	#define MG_NET_SOCKET_BUFF_SIZE					1024*32
	//server SOCKET发送缓冲区大小
	#define MG_NET_MAX_SERVER_SOCKET_SENDBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE*10
	//server SOCKET接收缓冲区大小
	#define MG_NET_MAX_SERVER_SOCKET_RECVBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE*10
	//client SOCKET发送缓冲区大小
	#define MG_NET_MAX_CLIENT_SOCKET_SENDBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE
	//client SOCKET接收缓冲区大小
	#define MG_NET_MAX_CLIENT_SOCKET_RECVBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE

    ///////////////////////////////////////////////////////////////////////
	//driver的类型
	#define DRIVER_TYPE_CLIENT  0
	#define DRIVER_TYPE_SERVER  1

	///////////////////////////////////////////////////////////////////////
	#define UDP_LOGIC_PACK_MIN_SIZE				512
	#define UDP_LOGIC_PACK_MAX_SIZE				1024 * 20 * 1000

	#define UDP_SECTION_PACKET_DEFAULT_SIZE		512
	#define UDP_SECTION_PACKET_MAX_SIZE			1024

    /******************************************************************************/
    //网络发送数据缓存
    /******************************************************************************/
    class NetSendDataBuffer
    {
    public:

        NetSendDataBuffer(){};
        virtual ~NetSendDataBuffer(){};

        // 初始化
        virtual Bool        initialize() = NULL;
        // 得到逻辑缓存指针
        virtual Char8*      getLogicData() = NULL;
        // 得到逻辑缓存最大容量
        virtual U32         getLogicDataMaxSize() = NULL;
        // 设置逻辑缓存发送容量
        virtual void        setLogicDataSendSize(U32 size) = NULL;
        // 得到逻辑缓存发送容量
        virtual U32         getLogicDataSendSize() = NULL;

		// 是否激活 
		virtual Bool		getActive() = NULL;
		// 设置激活 
		virtual void		setActive(Bool active) = NULL;
    };  

    /******************************************************************************/
    //网络实际一次发送连接对象配置
    /******************************************************************************/
    class NetOnceSendConfig
    {
    public:

        // 初始化
        virtual Bool    initialize(I32 driveID,I32 netID,I32 sendID){mDriverID=driveID;mNetID=netID;mSendID=sendID;mIsImmeSend=false;return true;};
        
        // 增加发送数据
        virtual void    pushSendData( NetSendDataBuffer* buff ) = 0;

        // 得到网络驱动编号
        I32             getDriverID(){ return mDriverID; };
        // 得到网络编号
        I32             getNetID(){ return mNetID; };
        // 得到发送顺序编号
        I32             getSendID(){ return mSendID; };
        // 是否立刻发送出去了
        Bool            isImmeSend(){ return mIsImmeSend; };
        // 是否是否立刻发送出去了
        void            setImmeSend(Bool isImme){ mIsImmeSend = isImme; };

		// 是否激活 
		virtual Bool	getActive(){ return mIsActive;}
		// 设置激活 
		virtual void	setActive(Bool active){ mIsActive = active; }

    protected:
        
        Bool    mIsImmeSend;
		Bool    mIsActive;
        I32     mDriverID;
        I32     mNetID;
        I32     mSendID;
    };  


    /******************************************************************************/
    //网络接收数据缓存
    /******************************************************************************/
    class NetRecvDataBuffer
    {
    public:
        NetRecvDataBuffer(){};
        virtual ~NetRecvDataBuffer(){};
    }; 
       
}

#endif