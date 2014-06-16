/******************************************************************************/
#ifndef _WIN32NETIOCPDATA_H_
#define _WIN32NETIOCPDATA_H_
/******************************************************************************/
#include "NetDefine.h"
#include "NetAddress.h"
#include "Lock.h"
#include "SharedPtr.h"
#include "NetSocket.h"
/******************************************************************************/
namespace MG
{

    /******************************************************************************/
    //Socket连接容器对象
    //用于描述Socket连接相关属性
    /******************************************************************************/
    class Win32IocpNetDriver;
    struct NetIocpConnect
    {
    public:

        //唯一编码
        I32					mId;	
        //外部锁定密码
        U64				    mUserCode ; 

        /////////////////////////////////////////////////

        //网络Socket
        SOCKET				mSocket;
        //网络地址
        NetAddress			mAddr;

        /////////////////////////////////////////////////

        //错误编号
        I32					mErrorNo;	
        //错误信息
        Char				mErrorStr[256];	

        /////////////////////////////////////////////////

        //网络连接信息
        NetConnectInfo		mConnectInfo;

        /////////////////////////////////////////////////

        //创造者
        Win32IocpNetDriver* mOwner;

    protected:

        //线程锁
        Critical			mCs;

    public:

        NetIocpConnect(Win32IocpNetDriver* owner);
        virtual ~NetIocpConnect();

        //清空属性
        void            clear();

        //线程锁定
        void            lock();
        //线程解锁
        void            unlock();

        //是否有网络连接
        Bool			isConnected();
        //关闭网络连接
        void			diconnect();

        //设置编号
        void            setID(I32 id);
        //得到编号
        I32             getID();
        //得到网络地址
        NetAddress&     getAddress(){return mAddr;};

        //设置外部锁定编号
        void            setCode(U64 code);
        //检测外部锁定编号
        Bool            checkCode(U64 code);

    };

    /******************************************************************************/
    //Socket连接容器智能指针
    //和NetIocpConnect的引用计数合用，组成双重引用计数
    /******************************************************************************/
    class NetIocpConnectPtr : public SharedPtr<NetIocpConnect> 
    {
    public:
        virtual ~NetIocpConnectPtr() { release(); }
        NetIocpConnectPtr() : SharedPtr<NetIocpConnect>() {}
        explicit NetIocpConnectPtr(NetIocpConnect* rep) : SharedPtr<NetIocpConnect>(rep) {}
        NetIocpConnectPtr(const NetIocpConnectPtr& r) : SharedPtr<NetIocpConnect>(r) {} 
        virtual void destroy(void);
        NetIocpConnectPtr& operator=(const NetIocpConnectPtr& r) 
        {
            if (pRep == r.pRep)
                return *this;
            NetIocpConnectPtr tmp(r);
            swap(tmp);
            return *this;
        }
    };

    
    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////


    /******************************************************************************/
    //Iocp网络包最底层数据结构
    /******************************************************************************/
#pragma pack (push)
#pragma pack (1)

    struct NetIocpPacketData
    {
        U8      tag;        //标签
        U16     dataLen;    //数据长度
        Char8   data[1];    //数据
    };

	struct NetIocpPacketData_INFO
	{
		static const Int headSize = sizeof (NetIocpPacketData) - sizeof(Char8);
	};

#pragma pack (pop)


    /******************************************************************************/
    //自定义WSAOVERLAPPED结构
    /******************************************************************************/

    struct NetIocpOverlapped
    {
    public:
        //网络缓存类型
        NET_EVENT_TYPE	    mType;
        //重叠结构
        WSAOVERLAPPED	    mWSAOverlapped;

        //清空重叠数据结构
        void                clear();
    };

    /******************************************************************************/
    //网络接受和发送缓存数据结构定义
    //自定义WSAOVERLAPPED结构
    /******************************************************************************/
    struct NetIocpBufBlock 
    {
    public:

        //缓存指针
        Char8*			    mBuf;
        //缓存大小
        UInt                mBufferSize;

        //缓存已用长度
        Int				    mUsedBufLen;
        //缓存当前可用位置
        Int				    mBufValidPos;

    public:

        NetIocpBufBlock();
        virtual ~NetIocpBufBlock();

        // 分配缓存
        void        mallocBuff(UInt bufferSize);
        // 释放缓存
        void        releaseBuff();

        //清空属性
        void        clear();

    };


    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////

    /******************************************************************************/
    //网络发送数据缓存管理对象
    /******************************************************************************/
    class NetIocpSendDataBuffer : public NetSendDataBuffer 
    {
    public:

        // IOCP缓存对象
        NetIocpBufBlock     mNetIocpBufBlock;

		Bool				mActive;

    public:

        NetIocpSendDataBuffer( Int bufferSize );
        virtual ~NetIocpSendDataBuffer();

        // 初始化
        virtual Bool            initialize();
        // 清空属性
        void                    clear();

        // 检测
        Bool                    check();

        // 得到实际数据包
        NetIocpPacketData*      getNetIocpPacketData();
        // 得到实际发送缓存大小
        U32                     getNetSendBuffSize();

        // 得到逻辑缓存指针
        virtual Char8*          getLogicData();
        // 得到逻辑缓存最大容量
        virtual U32             getLogicDataMaxSize();
        // 设置逻辑缓存发送容量
        virtual void            setLogicDataSendSize(U32 size);
        // 得到逻辑缓存发送容量
        virtual U32             getLogicDataSendSize();

		// 是否激活 
		virtual Bool			getActive();
		// 设置激活 
		virtual void			setActive(Bool active) ;
    };

    /******************************************************************************/
    //网络实际发送连接数据缓存管理对象
    /******************************************************************************/
    class NetIocpOnceSendConfig  : public NetOnceSendConfig 
    {
        friend class Win32IocpNetDriver;
    public:

        NetIocpOnceSendConfig();

        // IOCP Overlapped
        NetIocpOverlapped   mOverlapped;

        // 初始化
        virtual Bool    initialize(I32 driveID,I32 netID,I32 sendID);
        // 检测
        Bool            check();

        // 增加发送数据
        virtual void    pushSendData( NetSendDataBuffer* buff );

        // 填充WSABUF
        WSABUF*         getAndFillWsabuf();
        // 填充WSABUF
        DWORD           getWsabufCount();
        // 发送WSABUF
        Int             getAndSubWsabufBuffSize(Int size);

    protected:

        WSABUF	                                mWbuf[MG_NET_MAX_COMBINE_PACKET_COUNT];
        std::vector<NetIocpSendDataBuffer*>     mNetSendDataBufferList;
        Int                                     mAllWbufSize;

    };


    /******************************************************************************/
    //网络接收数据缓存管理对象
    /******************************************************************************/
    class NetIocpRecvDataBuffer : public NetRecvDataBuffer 
    {
    public:

        // IOCP Overlapped
        NetIocpOverlapped   mOverlapped;

        //IOCP缓存对象
        NetIocpBufBlock     mNetIocpBufBlock;

        //创造者
        Win32IocpNetDriver* mOwner;

        //网络连接智能指针
        NetIocpConnectPtr   mNetIocpConnectPtr;

    public:
        NetIocpRecvDataBuffer(  Win32IocpNetDriver* owner, UInt bufferSize );
        virtual ~NetIocpRecvDataBuffer();

        //清空属性
        void                clear();
        // 销毁
        virtual Bool        destroy(void);
    };
}

#endif