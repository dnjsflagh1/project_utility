/******************************************************************************/
#ifndef _WIN32NETIOCPDRIVER_H_
#define _WIN32NETIOCPDRIVER_H_
/******************************************************************************/

#include "NetDriver.h"
#include "NetDefine.h"
#include "NetAddress.h"
#include "Win32NetIocpData.h"
#include "NetSocket.h"
#include "ObjectPool.h"
#include "Thread.h"

/******************************************************************************/
namespace MG
{
    /******************************************************************************/
    //Iocp网络驱动层
    /******************************************************************************/
	class Win32IocpNetDriver : public NetDriver
	{
		
	public:
								        Win32IocpNetDriver();
		virtual					        ~Win32IocpNetDriver();

	public:
		//main
		virtual Bool			        initialize( NetDriverInitParameter& parameter,NetManager* mgr );		
		virtual I32				        unInitialize();
		virtual Bool			        isInitialized();
		virtual I32				        update();

        //////////////////////////////////////////////////////////////////////////////////////

		//get&set
		virtual Bool			        setManager(NetManager* mgr);
		virtual void			        setHandle(I32 handle);
		virtual I32				        getHandle();
		virtual Str				        getName() const;
		virtual CChar*			        getDriverDesc() const;

        //////////////////////////////////////////////////////////////////////////////////////

		//net op

        // 连接网络, 可连接多个远程地址
		virtual I32				        connect(NetAddress* addr);	
        // 关闭网络连接
		virtual I32				        close(I32 id);
        // 异常关闭网络连接
                I32				        exceptClose(I32 id);
        // 关闭所有网络连接
		virtual void			        closeAll();
        // 开始监听网络端口连接 , 只能监听一个网络端口
		virtual I32				        listen(NetAddress* addr);  

        // 发送数据
        virtual Bool                    send(NetOnceSendConfig* config);

        // 设置外部码
        virtual void                    setCode(I32 id, U64 code);

        //////////////////////////////////////////////////////////////////////////////////////
        
        virtual	U64 	                getAddressUin(I32 id);
        virtual	NetAddress              getAddress(I32 id);

		virtual	NetConnectInfo*         getNetInfo();
		virtual	NetConnectInfo*         getConnectInfo(I32 id);
		virtual	I32				        getCurrConnectionCount();

	protected:

        //////////////////////////////////////////////////////////////////////////////////////

		// 网络事件处理线程相关方法
		static void                     handleIOCPStatusThread(Ptr ptr); // IOCP网络事件线程处理方法
		Bool					        createIOCPStatusThreads(Int count);
		void					        destroyIOCPStatusThreads();

        //////////////////////////////////////////////////////////////////////////////////////

        // 设置系统级别超时检测
        void                            setSystemKeepAlive( SOCKET socket );

        //////////////////////////////////////////////////////////////////////////////////////

        // 检查网络连接有效性处理线程相关方法
        static void                     handleCheckSocketValidThread(Ptr ptr); 

        // 线程创建和析构
        Bool					        createCheckSocketValidThread();
        void					        destroyCheckSocketValidThread();

        // 发送检测心跳包
        void                            sendCheckSocketValidHeartBeat( I32 netID );
        // 刷新设置网络连接有效时间
        void                            refreshSocketValidTime( SOCKET socket );

        // 增加一条检测记录
        void                            addCheckSocketValidRecord( SOCKET socket, I32 netID, Bool isAccept );
        // 去除一条检测记录
        void                            removeCheckSocketValidRecord( SOCKET socket );

        //////////////////////////////////////////////////////////////////////////////////////

		// 完全端口相关方法
		Ptr						        createIOCP(); // 创建完全端口
		void					        destroyIOCP();
		Ptr						        getIOCP();
		HANDLE					        associateSocketWithIOCP(SOCKET socket,ULONG_PTR key=NULL);

        //////////////////////////////////////////////////////////////////////////////////////

		// Socket相关方法
		SOCKET					        createTcpSocket();
		void					        destroySocket(SOCKET& socket);
        Bool					        configSocketWhenConnected(SOCKET socket, Bool isAccept);
		Bool					        configAcceptSocket(SOCKET socket);
		
        //////////////////////////////////////////////////////////////////////////////////////

		// 网络连接容器相关方法
        NetIocpConnectPtr               addConnectContainer();
        // 移除网络连接对象 // isException 是否被动
        void                            removeConnectContainer(I32 id,Bool isException,Bool isClearSocketRecord);
        // 移除网络连接对象 // isException 是否被动
        void                            removeConnectContainer(NetIocpConnect* socketObject,  Bool isException,Bool isClearSocketRecord);
        // 移除所有网络连接对象
		void					        removeAllConnectContainers();
        // 得到一个网络连接对象 
        NetIocpConnectPtr               getConnectContainer(I32 id);	

    public:

        //创建连接对象
        NetIocpConnect*                 createConnectContainer( I32 id );
        //销毁连接对象
        void                            destroyConnectContainer(NetIocpConnect* socketObject);

        //创建接收数据缓存对象
        NetIocpRecvDataBuffer*          createNetIocpRecvDataBuffer( NetIocpConnectPtr& netConnect );
        //销毁接收数据缓存对象
        void                            destroyNetIocpRecvDataBuffer(NetIocpRecvDataBuffer* buffObject);
        //销毁所有接收数据缓存对象
        void                            destroyAllNetIocpRecvDataBuffer();

	protected:

        //////////////////////////////////////////////////////////////////////////////////////

		//投递接收连接请求
		Bool					        postAcceptRequest();
        //投递发送数据请求
		Bool                            postSendRequest(NetIocpOnceSendConfig* iocpSendConfig);
        //投递接收数据请求
		Bool                            postRecvRequest(NetIocpRecvDataBuffer* buffObject);
		
        //处理接收连接回应
		void					        processAcceptResponse(NetIocpRecvDataBuffer* buffObject, I32 bytes, I32 error);
        //处理发送数据回应
		void					        processSendResponse(NetIocpOnceSendConfig* configObject, I32 bytes, I32 error);
        //处理接收数据回应
        ///收到0字节包时，默认认为需要断开连接
		void					        processRecvResponse(NetIocpRecvDataBuffer* buffObject, I32 bytes, I32 error);
        
        //////////////////////////////////////////////////////////////////////////////////////

        //记录发送日志
        void                            recordSendLog( NetIocpConnect* netSocket, I32 bytes );
        //记录接受日志
        void                            recordRecvLog( NetIocpConnect* netSocket, I32 bytes );

	private:

        //////////////////////////////////////////////////////////////////////////////////////

		Str         mName;                                      // 描述名字
		I32         mHandle;									// Driver唯一序列号
        I32         mErrorNo;                                   // 错误编号

		Bool        mNetDriverInited;                           // 初始化状态
		Bool        mNetDriverIniting;                          // 初始化中状态
		Bool        mNetDriverListened;                         // 监听状态

        Bool        mIsCloseWhenError;                          // 是否关闭当错误异常时
		
		Int			mDriverType;								// driver类型

        //////////////////////////////////////////////////////////////////////////////////////

		I32							        mCpuCount;									// 当前机器中的CPU个数								
		Ptr							        mCompletionPort;							// 完成端口句柄
		LPFN_ACCEPTEX				        mAcceptEx;									// AcceptEx 指针
		LPFN_GETACCEPTEXSOCKADDRS	        mGetAcceptExSockaddrs;						// GetAcceptExSockaddrs 指针


        //////////////////////////////////////////////////////////////////////////////////////

		OneThreadHandle                     mThreadHandles[MG_NET_MAX_THREADS];     	// 监控线程句柄
		I32							        mStatuses[MG_NET_MAX_THREADS];			    // 监控线程状态
		I32							        mThreadCount;								// 监控线程计数
		
        //////////////////////////////////////////////////////////////////////////////////////

        ObjectPool<NetIocpConnect,false>    mNetIocpConnectPool;                        // 连接对象池
        std::map<I32,NetIocpConnectPtr>     mNetIocpConnectList;                        // 连接对象列表
        RWLocker					        mNetIocpConnectListCs;                      // 连接对象列表临界区
        I32                                 mNetIocpConnectIndexGenerator;              // 连接对象分配ID

        //////////////////////////////////////////////////////////////////////////////////////

        struct NetIocpConnectSocket
        {
            I32     netID;
            Bool    isAccept;
            U32     lastValidTime;
            U32     lastSendHeartBeatTime;

            NetIocpConnectSocket()
            {
                lastValidTime = 0;
                lastSendHeartBeatTime = 0;
            }

            NetIocpConnectSocket( I32 id, Bool accepted )
                :netID(id),isAccept(accepted)
            {
                lastValidTime = 0;
                lastSendHeartBeatTime = 0;
            }
        };

        std::map<SOCKET,NetIocpConnectSocket>   mNetIocpConnectSocketMap;               // 连接对象Socket列表
        Critical					        mNetIocpConnectSocketMapCs;                 // 连接对象列表临界区
        OneThreadHandle                     mCheckNetIocpConnectSocket;                 // 检测监控网络连接句柄
        NetIocpSendDataBuffer*              mCheckNetIocpConnectSendHeartBeatPacket;    // 心跳发送包

        //////////////////////////////////////////////////////////////////////////////////////

        ObjectPool<NetIocpRecvDataBuffer,false>   
                                            mNetIocpRecvBuffPool;                        // 接收数据缓存结构池
        Critical					        mNetIocpRecvBuffPoolCs;                      // 连接对象列表临界区
        
        UInt                                mSendNormalBufferSize;                       // 发送数据缓存大小
        UInt                                mSendBigBufferSize;                          // 发送数据缓存大小

		UInt								mSocketSendBufferSize;                       // 接收数据缓存大小
		UInt								mSocketRecvBufferSize;                       // 接收数据缓存大小

        UInt                                mRecvBufferSize;                             // 接收数据缓存大小

		I32							        mMaxConnections;                             // 最大连接数量
		InterLocked					        mCurrConnections;                            // 现在的连接数量

        //////////////////////////////////////////////////////////////////////////////////////

		SOCKET						    mListenSocket;                               // 监听连接端口Socket
		Int							    mWaitAcceptSocketCount;                      // 等待连接端口数量   
		NetAddress					    mAddrs;	                                     // 本机网络地址
		NetManager					    *mNetManager;                                // 管理者


        //////////////////////////////////////////////////////////////////////////////////////

		Bool						    mIsNetworkTimeoutValid;						 // 是否设置了网络超时处理
        Int						        mNetworkTimeoutTouchTime;                     // 网络超时触发时间
        Int						        mNetworkTimeoutDuration;                     // 网络超时持续时间

        //////////////////////////////////////////////////////////////////////////////////////

        Critical                        mNetConnectInfoCs;                           // 连接信息线程锁
		NetConnectInfo				    mNetConnectInfo;                             // 连接信息
	};
}

#endif