/******************************************************************************/
#ifndef _WIN32IOCPNETUDPDRIVER_H_
#define _WIN32IOCPNETUDPDRIVER_H_
/******************************************************************************/

#include "NetUdpDriver.h"
#include "NetDefine.h"
#include "NetAddress.h"
#include "Win32IocpNetUdpData.h"
#include "NetSocket.h"
#include "ObjectPool.h"

/******************************************************************************/
namespace MG
{
	
	/******************************************************************************/
	//UDP本地网络套接字配置
	/******************************************************************************/
	struct NetUdpLocalSocket
	{
		NetAddress				addr;
		SOCKET					socket;
		Bool					isBind;
		NetUdpLocalSocket()
		{
			socket			= (SOCKET)SOCKET_ERROR;
			isBind			= true;
		}
		NetUdpLocalSocket& operator =(NetUdpLocalSocket& config)
		{
			this->addr				= config.addr;
			this->socket			= config.socket;
			this->isBind			= config.isBind;
			return *this;
		}
	};
    /******************************************************************************/
    //Iocp网络驱动层
    /******************************************************************************/
	class Win32IocpNetUdpDriver : public NetUdpDriver
	{
		
	public:
								        Win32IocpNetUdpDriver();
		virtual					        ~Win32IocpNetUdpDriver();

	public:
		//main
		virtual Bool			        initialize( NetUdpDriverInitParameter& parameter,NetUdpManager* mgr );		
		virtual I32				        unInitialize();
		virtual Bool			        isInitialized();
		virtual I32				        update();

        //////////////////////////////////////////////////////////////////////////////////////
		//get&set
		virtual Bool			        setManager(NetUdpManager* mgr);
		virtual void			        setHandle(I32 handle);
		virtual I32				        getHandle();
		virtual Str				        getName() const;
		virtual CChar*			        getDriverDesc() const;
		
		//////////////////////////////////////////////////////////////////////////////////////
		//net op
		//配置本地端口信息
		Bool							configNetLocalSockets(NetAddress* addr = NULL,I32 count = 0);

		void							postWaitRecv(U16 socketIndex);
		//连接远程地址
		void							connect(NetAddress* addr,U16 socketIndex,U32 logicId);
		// 关闭网络连接
		virtual I32				        close(I32 id);
        // 被动关闭网络连接
		I32								passivityClose(I32 id);
        // 关闭所有网络连接
		virtual void			        closeAll();

        // 发送数据缓存对象, IocpNetUdpSendDataBuffer调用
        Bool                            send(IocpNetUdpSendDataBuffer* buffObject, I32 id, U16 socketIndex,U64 code = 0);
		Bool                            send(IocpNetUdpSendDataBuffer* buffObject, NetAddress& addr,U16 socketIndex);

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
		// 完全端口相关方法
		Ptr						        createIOCP(); // 创建完全端口
		void					        destroyIOCP();
		Ptr						        getIOCP();
		HANDLE					        associateSocketWithIOCP(SOCKET socket,ULONG_PTR key=NULL);

        //////////////////////////////////////////////////////////////////////////////////////
		// Socket相关方法
		SOCKET					        createUdpSocket();
		void					        destroySocket(SOCKET& socket);
		NetUdpLocalSocket*				getLocalSocket(U16 socketIndex);
		U16								getLocalSocketCount();

        //////////////////////////////////////////////////////////////////////////////////////
		// 网络连接容器相关方法
        IocpNetUdpConnectPtr			addConnectContainer(NetAddress& addr,U16 socketIndex);
        // 移除网络连接对象//返回智能指针引用计数
        void							removeConnectContainer(I32 id,Bool isException);
        // 移除网络连接对象
        void							removeConnectContainer(IocpNetUdpConnect* socketObject,  Bool isException);
        // 移除所有网络连接对象
		void							removeAllConnectContainers();
        // 得到一个网络连接对象 
        IocpNetUdpConnectPtr			getConnectContainer(I32 id);
		IocpNetUdpConnectPtr			getConnectContainerByUin(U16 socketIndex,U64 uin);	

    public:
        //创建连接对象
        IocpNetUdpConnect*				createUdpConnectContainer(I32 id );
        //销毁连接对象
        void							destroyUdpConnectContainer(IocpNetUdpConnect* socketObject);
        //创建发送数据缓存对象
        IocpNetUdpSendDataBuffer*		createIocpNetUdpSendDataBuffer( U32 buffSize  );
        //销毁发送数据缓存对象
        void							destroyIocpNetUdpSendDataBuffer(IocpNetUdpSendDataBuffer* buffObject);
        //销毁所有发送数据缓存对象
        void							destroyAllIocpNetUdpSendDataBuffer();
        //创建接收数据缓存对象
        IocpNetUdpRecvDataBuffer*		createIocpNetUdpRecvDataBuffer(U16 sectionPacketSize);
        //销毁接收数据缓存对象
        void							destroyIocpNetUdpRecvDataBuffer(IocpNetUdpRecvDataBuffer* buffObject);
        //销毁所有接收数据缓存对象
        void							destroyAllIocpNetUdpRecvDataBuffer();

		//////////////////////////////////////////////////////////////////////////////////////
		//创建逻辑缓存对象
		LogicSendPacket*				getLogicSendPacket(Byte channel,Byte type,U16 sectionPacketSize,U32 logicId,U32 logicDataSize,Char8* data);

	protected:
        //////////////////////////////////////////////////////////////////////////////////////
        //投递发送数据请求
		Bool                            postSendRequest(IocpNetUdpSendDataBuffer* buffObject,U16 socketIndex );
        //投递接收数据请求
		Bool                            postRecvRequest(IocpNetUdpRecvDataBuffer* buffObject,U16 socketIndex );
		
        //处理发送数据回应
		void					        processSendResponse(IocpNetUdpSendDataBuffer* buffObject, I32 bytes, I32 error);
        //处理接收数据回应
        ///收到0字节包时，默认认为需要断开连接
		void					        processRecvResponse(IocpNetUdpRecvDataBuffer* buffObject, I32 bytes, I32 error);
        
        //////////////////////////////////////////////////////////////////////////////////////
        //记录发送日志
        void                            recordSendLog( IocpNetUdpConnect* netSocket, I32 bytes );
        //记录接受日志
        void                            recordRecvLog( IocpNetUdpConnect* netSocket, I32 bytes );

	private:

        //////////////////////////////////////////////////////////////////////////////////////
		Str										mName;                                      // 描述名字
		I32										mHandle;									// Driver唯一序列号
        I32										mErrorNo;                                   // 错误编号

		Bool									mNetDriverInited;                           // 初始化状态
		Bool									mNetDriverIniting;                          // 初始化中状态
		Bool									mNetDriverListened;                         // 监听状态

        Bool									mIsCloseWhenError;                          // 是否关闭当错误异常时
	
        //////////////////////////////////////////////////////////////////////////////////////

		I32										mCpuCount;									// 当前机器中的CPU个数								
		Ptr										mCompletionPort;							// 完成端口句柄

        //////////////////////////////////////////////////////////////////////////////////////

		OneThreadHandle							mThreadHandles[MG_NET_MAX_THREADS];     	// 监控线程句柄
		I32										mStatuses[MG_NET_MAX_THREADS];			    // 监控线程状态
		I32										mThreadCount;								// 监控线程计数
		
    
        //////////////////////////////////////////////////////////////////////////////////////

        ObjectPool<IocpNetUdpConnect,false>     mIocpNetUdpConnectPool;                        // 连接对象池
		
		//key = netId value = connectObj
        std::map<I32,IocpNetUdpConnectPtr>		mIocpNetUdpConnectList;                        // 连接对象列表
	
		//connectObjs <key = uin,value = connectObj>
		//key = socketIndex value = connectObjs								                   // 连接对象列表
		std::map< U16, std::map< U64,IocpNetUdpConnectPtr > >
												mIocpNetUdpConnectListsbyUin;

		Critical								mIocpNetUdpConnectListCs;                      // 连接对象列表临界区
        I32										mIocpNetUdpConnectIndexGenerator;              // 连接对象分配ID

        std::map<U32, ObjectPool<IocpNetUdpSendDataBuffer, false>>
												mIocpNetUdpSendBuffPoolMap;						// 发送数据缓存结构池列表
        Critical								mIocpNetSendBuffPoolMapCs;						// 连接对象列表临界区

        ObjectPool<IocpNetUdpRecvDataBuffer, false>	
                                                mIocpNetUdpRecvBuffPool;                        // 接收数据缓存结构池
        Critical								mIocpNetRecvBuffPoolCs;							// 连接对象列表临界区

        UInt									mSendNormalBufferSize;							// 接收数据缓存大小
        UInt									mSendBigBufferSize;								// 接收数据缓存大小

        UInt									mRecvBufferSize;								// 接收数据缓存大小

		I32										mMaxConnections;								// 最大连接数量
		I32										mCurrConnections;								// 现在的连接数量

        //////////////////////////////////////////////////////////////////////////////////////							 
		NetUdpLocalSocket*						mLocalSockets;									// 本机网络配置
		Int										mLocalSocketsCount;

		NetUdpManager*							mNetUdpManager;									// 管理者

		Bool									mIsNetworkTimeoutValid;							// 是否设置了网络超时处理
        Int										mWaitAcceptSocketCount;							// 等待网络处理数量 
        
		//////////////////////////////////////////////////////////////////////////////////////
		NetConnectInfo							mNetConnectInfo;								// 连接信息

	};
}

#endif