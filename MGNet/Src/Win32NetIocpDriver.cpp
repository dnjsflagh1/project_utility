/******************************************************************************/
#include "stdafx.h"
#include "Win32NetIocpDriver.h"
#include "MGStrOp.h"
#include "NetErrorDefine.h"
#include "NetAddress.h"
#include "MGStrOp.h"
#include "NetManager.h"
#include "NetDefine.h"
#include <mstcpip.h>
/******************************************************************************/
namespace MG
{
    //-----------------------------------------------------------------------------
    Win32IocpNetDriver::Win32IocpNetDriver()
        :mNetDriverInited(false),
		mNetDriverIniting(false),
        mNetDriverListened(false),
        mIsNetworkTimeoutValid(true),
        mNetworkTimeoutTouchTime(5000),
        mNetworkTimeoutDuration(50000),
		mMaxConnections(1),
        mCompletionPort(NULL),
        mNetManager(NULL),
		mListenSocket(NULL),
		mWaitAcceptSocketCount(5),
        mNetIocpConnectIndexGenerator(0),
		mCurrConnections(0),
        mSendNormalBufferSize(0),
        mSendBigBufferSize(0),
        mRecvBufferSize(0),
		mSocketSendBufferSize(0),
		mSocketRecvBufferSize(0),
		mDriverType(0)

    {
        mNetIocpRecvBuffPool.setCheckExceptEnable( false );
        mNetIocpConnectPool.setCheckExceptEnable( false );
    }

    //-----------------------------------------------------------------------------
    Win32IocpNetDriver::~Win32IocpNetDriver()
    {
        unInitialize();
    }

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::initialize( NetDriverInitParameter& parameter,NetManager* mgr )
	{
        //判断是否已经初始化或正在初始化
		if (mNetDriverInited || mNetDriverIniting)
		{
			return false;
		}
		///放在顶部为了不支持重新初始化
		mNetDriverIniting = false;

        //设置参数
		{
			mName                       = parameter.name;
			mIsNetworkTimeoutValid      = parameter.isSetNetworkTimeout;
            mNetworkTimeoutTouchTime    = parameter.networkTimeoutTouchTime;
            mNetworkTimeoutDuration     = parameter.networkTimeoutDuration;
			mMaxConnections             = parameter.maxConnections;
			mWaitAcceptSocketCount      = (mMaxConnections / 2 + 1) > parameter.waitAcceptSocketCount ? parameter.waitAcceptSocketCount : (mMaxConnections/2);
		    
            mSendNormalBufferSize       = parameter.sendNormalBuffSize;
            mSendBigBufferSize          = parameter.sendBigBuffSize;
            mRecvBufferSize             = parameter.recvBuffSize;

            mIsCloseWhenError           = parameter.isCloseWhenError;
			mSocketSendBufferSize	    = parameter.socketSendBuffSize;
			mSocketRecvBufferSize	    = parameter.socketRecvBuffSize;
			mDriverType				    = parameter.driverType;
        }

        //Winsock服务的初始化
		{
			WSADATA   wsd;
			if ( WSAStartup(MAKEWORD(2,2), &wsd) != 0 )
			{
				return false;
			}
		}

        //设置管理者
		{
			if ( setManager(mgr) == false )
			{
				WSACleanup();
				return false;
			}
		}

		//创建一个网络套接字用于监听
		{
			mListenSocket = createTcpSocket();
			if (mListenSocket == INVALID_SOCKET)
			{
				return false;
			}
		}

        // 配置 ListenSocket
		{
			if ( configAcceptSocket(mListenSocket) == false )
			{
				return false;
			}
		}

		// 创建完全端口
		{
			if ( createIOCP() == NULL )
			{
				return false;
			}
		}


        // 创建处理网络事件线程
		{
			if ( createIOCPStatusThreads(parameter.handleSocketEventThreadCount) == false )
			{
				return false;
			}
		}


        // 创建检测网络连接有效性线程
        {
            if ( createCheckSocketValidThread() == false )
            {
                return false;
            }
        }
        
		mNetDriverListened = false;
		mNetDriverInited = true;
		mNetDriverIniting = false;
		mCurrConnections = 0;

		return true;
	}

    //-----------------------------------------------------------------------------
	I32 Win32IocpNetDriver::update()
	{
		//检查链接 是否有非法链接
		return 1;
	}

    //-----------------------------------------------------------------------------
	I32 Win32IocpNetDriver::unInitialize()
	{
        if ( mNetDriverInited == true )
        {
            mNetDriverInited = false;
			
            // 关闭检测连接有效性处理线程
            destroyIOCPStatusThreads();

			//关闭所有连接
			closeAll();

            //关闭网络事件处理线程
			destroyIOCPStatusThreads();

			Sleep(10);

            //关闭完全端口
			destroyIOCP();
			
			//关闭监听端口
			destroySocket(mListenSocket);

            WSACleanup();
			
			mCurrConnections = 0;
            //销毁网络连接对象池
			//destroyAllConnectContainers();
        }
		return 0;
	}

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::setManager( NetManager* mgr )
	{
		DYNAMIC_ASSERT(mgr);
		if (!mgr)
		{
			return false;
		}
		mNetManager = mgr;
		return true;
	}

    //-----------------------------------------------------------------------------
	I32 Win32IocpNetDriver::connect( NetAddress* addr )
	{
		if ( !mNetDriverInited )
		{
			return MG_NET_ERR;
		}

		// 流程是否继续
		Bool isContinue = true;

		// 设置网络连接对象
		NetIocpConnectPtr netConnect;
		if ( isContinue )
		{
            //获得一个连接对象
			netConnect = addConnectContainer();
			if ( netConnect.isNull() )
			{
				isContinue = false;
			}else
			{
                SOCKET socket = createTcpSocket();

				if ( socket == INVALID_SOCKET )
				{
                    DYNAMIC_ASSERT( false );
					isContinue = false;
				}else
                {
                    // 检测是否有无效的连接使用新的Socket
                    addCheckSocketValidRecord( socket, netConnect->getID(), false );
                }

                // 绑定Socket到连接对象
                netConnect->mSocket = socket;
			}
		}

		// 关联Scoket到完成端口
		if ( isContinue )
		{
			HANDLE hrc = associateSocketWithIOCP( netConnect->mSocket, (ULONG_PTR)netConnect.getPointer() );
			if ( hrc == NULL )
			{
				isContinue = false;
			}
		}

		//连接网络地址
		if ( isContinue )
		{
			sockaddr_in inAddr;
			inAddr.sin_family = AF_INET;
			inAddr.sin_addr.s_addr = NetAddress::host2Net((U32)addr->getIP());
			inAddr.sin_port = NetAddress::host2Net((U16)addr->getPort());
			netConnect->mAddr = *addr;

			Int rc;
			rc = ::connect( netConnect->mSocket, (struct sockaddr*)&inAddr, sizeof(inAddr) );
			if ( rc == SOCKET_ERROR )
			{
				mErrorNo = WSAGetLastError();
				isContinue = false;
			}else
            {
                isContinue = configSocketWhenConnected( netConnect->mSocket, false );
            }
		}

		if ( isContinue )
		{


            // 得到一个接受缓存数据
            NetIocpRecvDataBuffer* buffObject = createNetIocpRecvDataBuffer( netConnect );

			//////////////////////////////////////////////////////////////
		
			//增加一个连接事件
			mNetManager->addConnectEvent(mHandle,netConnect->mId,netConnect->mAddr);

            //////////////////////////////////////////////////////////////

            // 投递一个接受请求
            postRecvRequest( buffObject );	

		}else
		{
            //失败退出
            if ( netConnect.isNull() == false )
            {
                exceptClose( netConnect->getID() );
            }
			return MG_NET_ERR;
		}

		return netConnect->mId;
	}

    //-----------------------------------------------------------------------------
	I32 Win32IocpNetDriver::listen( NetAddress* addr )
	{
		if ( !mNetDriverInited )
		{
			return MG_NET_ERR;
		}

		if (mNetDriverListened)
		{
			return MG_NET_ERR;
		}

		// 流程是否继续
		Bool isContinue = true;

		// 关联Scoket到完成端口
		if ( isContinue )
		{
			HANDLE hrc = associateSocketWithIOCP( mListenSocket, (ULONG_PTR)this );
			if ( hrc == NULL )
			{
				mErrorNo = GetLastError();
				isContinue = false;
			}
		}

		// 监听Scoket
		if ( isContinue )
		{
			sockaddr_in addrIn;
			addrIn.sin_family = AF_INET;
			addrIn.sin_addr.s_addr = NetAddress::host2Net(addr->getIP());
			addrIn.sin_port = NetAddress::host2Net((U16)addr->getPort());

			int rc;
			rc = bind( mListenSocket, (struct sockaddr*)&addrIn, sizeof(addrIn) );
			if ( rc == SOCKET_ERROR )
			{
				mErrorNo = GetLastError();
				return MG_NET_ERR;
			}

			rc = ::listen( mListenSocket, 200 );
			if ( rc == SOCKET_ERROR )
			{
				mErrorNo = GetLastError();
				return MG_NET_ERR;
			}

			mNetDriverListened = true;
		}
		
		//创建等待监听Socket
		if ( isContinue )
		{
			for (int i = 0; i < mWaitAcceptSocketCount; i++)
			{
                //提交接受连接请求
                postAcceptRequest();
			}
		}else
		{
			return mErrorNo;
		}

		mNetConnectInfo.mStartTime = MGTimeOp::getCurrTick();

		return MG_NET_SUCCESS;
	}

    //-----------------------------------------------------------------------------
    Bool Win32IocpNetDriver::send(NetOnceSendConfig* config)
    {
        Bool result = false;

        NetIocpOnceSendConfig* iocpSendConfig   =  (NetIocpOnceSendConfig*)config;

        //if ( netConnect->checkCode(code)==true )
        if ( iocpSendConfig && iocpSendConfig->check() )
        {
            result = postSendRequest( iocpSendConfig );
        }

        return result;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::setCode(I32 id, U64 code)
    {
        NetIocpConnectPtr netConnect = getConnectContainer(id);
        if ( netConnect.isNull() == false )
        {
            netConnect->setCode(code);
        }
    }

    //-----------------------------------------------------------------------------
	I32 Win32IocpNetDriver::close( I32 id )
	{
		removeConnectContainer(id,false,true);
		return 1;
	}

    //-----------------------------------------------------------------------------
    I32 Win32IocpNetDriver::exceptClose(I32 id)
    {
        removeConnectContainer(id,true,true);
        return 1;
    }

    //-----------------------------------------------------------------------------
	void Win32IocpNetDriver::closeAll()
	{
		removeAllConnectContainers();
	}

    //-----------------------------------------------------------------------------
	Str Win32IocpNetDriver::getName() const
	{
		return mName;
	}

    //-----------------------------------------------------------------------------
	CChar* Win32IocpNetDriver::getDriverDesc() const
	{
		return NULL;
	}

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::setSystemKeepAlive( SOCKET socket )
    {
        // 设置Socket超时
        struct tcp_keepalive keepAlive = {1, mNetworkTimeoutTouchTime, MG_NET_TIMEOUT_CHECK_HEARTBEAT};
        DWORD dwBytesReturned;
        WSAIoctl( socket, SIO_KEEPALIVE_VALS,       // dwIoControlCode
            (LPVOID)&keepAlive,		                // pointer to tcp_keepalive struct 
            sizeof(keepAlive),                      // length of input buffer 
            NULL,                                   // output buffer
            0,                                      // size of output buffer
            &dwBytesReturned,                       // number of bytes returned
            NULL,                                   // OVERLAPPED structure
            NULL                                    // completion routine
            );

        // 屏蔽Nagle算法
        Int nDisable = 1;
        setsockopt(socket,  IPPROTO_TCP, TCP_NODELAY, (const char*)&nDisable, sizeof(nDisable));
    }
    
    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::handleCheckSocketValidThread(Ptr ptr)
    {
        OneThreadHandle* thd = (OneThreadHandle*)ptr;	
        if (thd)
        {
            if (thd->getParam())
            {
                Win32IocpNetDriver*dev  = (Win32IocpNetDriver *)(thd->getParam());
                SOCKET  socket;
                I32     netID = 0;
                UInt    checkIndex = 0;
                UInt    mapCount = 0;
                Bool    isDisconned = false;
                U32     currTime = MG::MGTimeOp::getCurrTick();
                Int     interval = 0;
                Int     networkMaxTimeout = dev->mNetworkTimeoutTouchTime;
                Int     networkTimeoutDuration = dev->mNetworkTimeoutDuration;

                std::map<SOCKET,NetIocpConnectSocket>::iterator iter;

                while ( thd->isRun())
                {	
                    isDisconned = false;

                    dev->mNetIocpConnectSocketMapCs.lock();
                    {
                        mapCount = dev->mNetIocpConnectSocketMap.size();

                        if ( mapCount > 0 )
                        {
                            if ( checkIndex >= mapCount )
                            {
                                checkIndex = 0;
                            }

                            iter = dev->mNetIocpConnectSocketMap.begin();

                            std::advance( iter, checkIndex );
                            socket  = iter->first;
                            netID   = iter->second.netID;

                            // 检测Socket是否超时
                            {
                                currTime = MG::MGTimeOp::getCurrTick();

                                interval = currTime - iter->second.lastValidTime;

                                // 如果超时就断开连接
                                if ( interval > networkMaxTimeout )
                                {
                                    isDisconned = true;
                                }else
                                // 如果一定时间没有网络通讯则开始启动心跳
                                if ( interval > networkTimeoutDuration )
                                {
                                    interval = currTime - iter->second.lastSendHeartBeatTime;

                                    // 开始启动间隔心跳
                                    if ( interval > MG_NET_TIMEOUT_CHECK_HEARTBEAT )
                                    {
                                        dev->sendCheckSocketValidHeartBeat(netID);
                                    }
                                }
                            }
                        }
                    }
                    dev->mNetIocpConnectSocketMapCs.unlock();

                    if ( isDisconned )
                    {
                        dev->exceptClose( netID );
                    }

                    checkIndex++;

                    Sleep(5);
                }
            }
        }
    }

    //-----------------------------------------------------------------------------
    Bool Win32IocpNetDriver::createCheckSocketValidThread()
    {
        //if ( mCheckNetIocpConnectSocket.create( destroyCheckSocketValidThread,this ) == false )
        //{
        //    return false;
        //}

        //mCheckNetIocpConnectSendHeartBeatPacket = MG_NEW NetIocpSendDataBuffer( this, MG_NET_DEFAULT_SEND_BUFFER_SIZE );

        return true;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::destroyCheckSocketValidThread()
    {
        //mCheckNetIocpConnectSocket.destory();

        //MG_SAFE_DELETE( mCheckNetIocpConnectSendHeartBeatPacket );
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::sendCheckSocketValidHeartBeat( I32 netID )
    {
       // mCheckNetIocpConnectSendHeartBeatPacket->send();
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::refreshSocketValidTime( SOCKET socket )
    {
        //mNetIocpConnectSocketMapCs.lock();
        //{
        //    std::map<SOCKET,NetIocpConnectSocket>::iterator iter = mNetIocpConnectSocketMap.find(socket);
        //    if ( iter != mNetIocpConnectSocketMap.end() )
        //    {
        //        iter->second.lastValidTime = MG::MGTimeOp::getCurrTick();
        //    }
        //}
        //mNetIocpConnectSocketMapCs.unlock();
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::addCheckSocketValidRecord( SOCKET socket, I32 netID, Bool isAccept )
    {
        I32 duplicateNetID = -1;
        mNetIocpConnectSocketMapCs.lock();
        {
            std::map<SOCKET,NetIocpConnectSocket>::iterator iter = mNetIocpConnectSocketMap.find(socket);
            if ( iter != mNetIocpConnectSocketMap.end() )
            {
                duplicateNetID = iter->second.netID;
                //DYNAMIC_ASSERT_LOG( false, "Win32IocpNetDriver::addCheckSocketValidRecord : Duplicate Scoket" );
            }

            // 如果有重复的Socket, 就说明有旧的连接用了新的Socket, 需要清空原来和该Socket绑定的连接信息
            if ( duplicateNetID != -1 )
            {
                NetIocpConnectPtr netIocpConnectPtr = getConnectContainer(duplicateNetID);  

                if ( netIocpConnectPtr.isNull() == false )
                {
                    // 先清空无效连接的Socket,防止下面释放连接时释放掉新的Socket
                    netIocpConnectPtr->mSocket = INVALID_SOCKET;

                    removeConnectContainer( duplicateNetID, true, false );
                }
            }

            mNetIocpConnectSocketMap[ socket ] = NetIocpConnectSocket(netID,isAccept);
        }
        mNetIocpConnectSocketMapCs.unlock();
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::removeCheckSocketValidRecord( SOCKET socket )
    {
        if ( socket != INVALID_SOCKET )
        {
            mNetIocpConnectSocketMapCs.lock();
            {
                std::map<SOCKET,NetIocpConnectSocket>::iterator iter = mNetIocpConnectSocketMap.find(socket);
                if ( iter != mNetIocpConnectSocketMap.end() )
                {
                    mNetIocpConnectSocketMap.erase(iter);
                }
            }
            mNetIocpConnectSocketMapCs.unlock();
        }
    }

	//-----------------------------------------------------------------------------
	void Win32IocpNetDriver::handleIOCPStatusThread(Ptr ptr)
	{
		OneThreadHandle* thd = (OneThreadHandle*)ptr;	
		if (thd)
		{
			if (thd->getParam())
			{
				Win32IocpNetDriver*dev  = (Win32IocpNetDriver *)(thd->getParam());
				NetIocpOverlapped*block = NULL;
				OVERLAPPED				*pol = NULL;
				DWORD					bytes = 0;
				DWORD					flags = 0;
				ULONG_PTR				key = 0;
				Int						rc;
				Int						error;
				Int						pass = 1;
				SOCKET					socket;

                NetIocpOnceSendConfig*  sendConfigObject;
                NetIocpRecvDataBuffer*  recvBuffObject;

				while ( pass && thd->isRun())
				{	
                    sendConfigObject = NULL;
                    recvBuffObject = NULL;

					error = NO_ERROR;

					// 阻塞等待I/O事件
					rc = GetQueuedCompletionStatus( dev->mCompletionPort, &bytes, &key, &pol, INFINITE );

					// 收到关闭消息
					if ( key == NET_CLOSE )
					{
						pass = 0; //退出
						continue;
					}

					if (rc == FALSE)
					{
						I32 err = GetLastError();
						if (err == ERROR_INVALID_HANDLE)
						{
							break;
						}
					}

					if ( rc == FALSE && pol == NULL )
					{
                        DYNAMIC_EEXCEPT_LOG("Win32IocpNetDriver::handleIOCPStatusThread : rc == FALSE && pol == NULL !");
						continue;
					}


					// 通过指针偏移得到自定义OVERLAPPED结构
					block = CONTAINING_RECORD( pol, NetIocpOverlapped, mWSAOverlapped );
					if ( block->mType == NET_SEND )
					{
						sendConfigObject = CONTAINING_RECORD( block, NetIocpOnceSendConfig, mOverlapped );
					}
                    else
					if ( block->mType == NET_ACCEPT || block->mType == NET_RECV )
					{	
						recvBuffObject = CONTAINING_RECORD( block, NetIocpRecvDataBuffer, mOverlapped );
					}else
					{
                        DYNAMIC_EEXCEPT_LOG("Win32IocpNetDriver::handleIOCPStatusThread : block->mType not define!");
						continue;
					}


					// 检查完成端口状态失败後调用 WSAGetOverlappedResult 获得出错信息
					if ( rc == FALSE )
					{
						if ( block->mType == NET_ACCEPT )
						{
							socket = ((NetIocpConnect*)key)->mSocket;
						}
						else
						{
							//socket = ((Win32IocpNetDriver*)key)->mListenSocket;
                            socket = ((NetIocpConnect*)key)->mSocket;
						}

						// Show error
						if (ERROR_OPERATION_ABORTED == GetLastError())
						{
							continue;
						}

						rc = WSAGetOverlappedResult( socket, &block->mWSAOverlapped, &bytes, FALSE, &flags );

						if ( rc == SOCKET_ERROR )
						{
							error = WSAGetLastError();

							if ( socket == dev->mListenSocket )
							{
								pass = 0;
								continue;
							}
						}
					}

					if ( block->mType == NET_ACCEPT )
					{
                        if ( recvBuffObject )
						    dev->processAcceptResponse( recvBuffObject, bytes, error );
					}
                    else
                    if ( block->mType == NET_RECV )
                    {
                        if ( recvBuffObject )
                            dev->processRecvResponse( recvBuffObject, bytes, error );
                    }
                    else
					if ( block->mType == NET_SEND )
					{
                        if ( sendConfigObject )
						    dev->processSendResponse( sendConfigObject, bytes, error );
					}
				}
			}
		}
	}

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::postAcceptRequest()
	{
        Bool isOk = false;

        NetIocpRecvDataBuffer* buffObject = NULL;
        //获得一个新的连接对象
        NetIocpConnectPtr netConnect = addConnectContainer();
        if (netConnect.isNull() == false )
        {
			int rc = 0;
            
            // 得到接受数据缓存对象
            buffObject = createNetIocpRecvDataBuffer( netConnect );

			buffObject->mOverlapped.mType = NET_ACCEPT;

            SOCKET socket = createTcpSocket();
            // 检测是否有无效的连接对象使用了新的Socket
            addCheckSocketValidRecord( socket, netConnect->getID(), true );
            // 绑定Socket到新的连接对象
			netConnect->mSocket = socket;

			DWORD bytes = 0;
			rc = mAcceptEx(
				mListenSocket,
				netConnect->mSocket,
				buffObject->mNetIocpBufBlock.mBuf,
				0,
				sizeof(sockaddr_in) + 16,
				sizeof(sockaddr_in) + 16,
				&bytes,
				&buffObject->mOverlapped.mWSAOverlapped);

            if(rc == TRUE)
            {
                isOk = true;
            }else
            {
                I32 err = WSAGetLastError();
                if(err!=ERROR_IO_PENDING)
                {
                    isOk = false;
                }else
                {
                    isOk = true;
                }
            }
		}

        //失败的话关闭连接
        if(isOk == false)
        {
            if ( netConnect.isNull() == false )
                exceptClose( netConnect->getID() );
            if ( buffObject )
                destroyNetIocpRecvDataBuffer( buffObject );

            DYNAMIC_ASSERT_LOG( false, "postAcceptRequest Failed" );

            // 等待一段时间重新投递接受数据请求
            Sleep(10);
            postAcceptRequest();
        }

        return isOk;
	}

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::postSendRequest( NetIocpOnceSendConfig* iocpSendConfig )
	{
		Bool	result      = false;
		WSABUF*	wbuf        = NULL;
        DWORD	wbufCount   = 0;
		DWORD	bytes       = 0;
		Int		ret         = NO_ERROR;
		Int		err         = SOCKET_ERROR;

        NetIocpConnectPtr netConnect = getConnectContainer(iocpSendConfig->getNetID());

        if ( netConnect.isNull()==false )
        {
            // 填充缓冲区描述结构
            wbuf        = iocpSendConfig->getAndFillWsabuf();
            wbufCount   = iocpSendConfig->getWsabufCount();
            DYNAMIC_ASSERT_LOG( wbufCount > 0, " postSendRequest : no buff! "  );

            // 设置发送标志
            iocpSendConfig->mOverlapped.mType = NET_SEND;

            //投递一个发送请求
            ret = WSASend( netConnect->mSocket,
                           wbuf, wbufCount, 
                           &bytes, 0, 
                           &iocpSendConfig->mOverlapped.mWSAOverlapped, 
                           NULL );

            // 当没有立即发送成功时
            if ( ret == SOCKET_ERROR )
            {
                // 发送错误，应该释放该SOCKET对应的所有资源
                if ( ( err = WSAGetLastError() ) != WSA_IO_PENDING )
                {
                    ret = SOCKET_ERROR;
                    result = false;
                }else
                    //表示异步投递已经成功，但是稍后发送才会完成
                    //此时投递的缓冲区将会被锁定，等到TCP/IP层缓冲区有空余时才会被发送，然后会发送给IOCP一个完成消息
                {
                    result = true;
                }
            }else
                // 投递立即成功，TCP/IP将数据从程序缓冲区中拷贝到TCP/IP层缓冲区中，然后不锁定该程序缓冲区。
            {
                iocpSendConfig->setImmeSend(true); 
                result = true;
                //if ( wbuf.len != bytes )
                //{
                //    // 虽然 发送数据返回值不等于发送值,但是TCP/IP之后还是会发送该数据
                //    MG_LOG( out_info, "%s: wbuf.len != bytes ...\n", __MG_FUNC__ );
                //}
            }
        }

        //失败后判断移除连接对象
        if ( result == false )
        {
            if ( mIsCloseWhenError )
            {
                if ( netConnect.isNull() == false )
                {
                    exceptClose( netConnect->getID() );
                }
            }
        }
		
#ifdef _DEBUG
		if ( err == WSAENOBUFS )
		{
			MG_LOG( out_error, "postSendRequest WSAENOBUFS error!!\n", __MG_FUNC__ );
		}else
		if ( err == WSAECONNRESET )
		{
			MG_LOG( out_error, "postSendRequest WSAECONNRESET error!!\n", __MG_FUNC__ );
		}else
        if ( err == WSAENOTCONN )
        {
            MG_LOG( out_error, "postSendRequest WSAENOTCONN error!!\n", __MG_FUNC__ );
        }

        mNetConnectInfoCs.lock();
        {
            mNetConnectInfo.mWaitSendPacketCount++;
            if ( mNetConnectInfo.mWaitSendPacketCount > mNetConnectInfo.mMaxWaitSendPacketCount )
                mNetConnectInfo.mMaxWaitSendPacketCount = mNetConnectInfo.mWaitSendPacketCount;
        }
        mNetConnectInfoCs.unlock();
#endif	

		return result;
	}

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::postRecvRequest(NetIocpRecvDataBuffer* buffObject)
	{
		WSABUF	wbuf;
		DWORD	bytes = 0;
		DWORD	flat = 0;
		Int		ret = NO_ERROR;
		Int		err = SOCKET_ERROR;
		Bool    isOk = false;

        NetIocpConnectPtr& netConnect = buffObject->mNetIocpConnectPtr;
        if ( buffObject && (netConnect.isNull()==false) )
        {
            //填充缓冲区描述结构
            wbuf.buf = buffObject->mNetIocpBufBlock.mBuf + buffObject->mNetIocpBufBlock.mBufValidPos;
            wbuf.len = buffObject->mNetIocpBufBlock.mBufferSize - buffObject->mNetIocpBufBlock.mBufValidPos;
            DYNAMIC_ASSERT( wbuf.len > 0 );

            //清空OVERLAPPED
            buffObject->mOverlapped.clear();

            //设置接受标志
            buffObject->mOverlapped.mType = NET_RECV;

            //投递一个接受请求
            ret = WSARecv( netConnect->mSocket, &wbuf, 1, &bytes, &flat, &buffObject->mOverlapped.mWSAOverlapped, NULL );

            // 当没有立即接受到TCP/IP中数据
            if ( ret == SOCKET_ERROR )
            {
                // 调用WSARecv错误，应该释放该SOCKET对应的所有资源
                if ( ( err = WSAGetLastError() ) != WSA_IO_PENDING )
                {
                    ret = SOCKET_ERROR;
                    isOk = false ;
                }else
                    //TCP/IP层缓冲区中没有数据可取，IOCP将锁定投递的WSARecv的buffer，直到TCP/IP层缓冲区中有新的数据到来通知IOCP
                {
                    isOk = true;
                }
            }else
            {
                isOk = true;
            }

            //成功后返回投递长度。
            if ( isOk )
            {
                ret = (Int)wbuf.len;
            }
        }

#ifdef _DEBUG

		if ( err == WSAENOBUFS )
		{
			MG_LOG( out_error, "postRecvRequest WSAENOBUFS error!!\n", __MG_FUNC__ );
		}else
		if ( err == WSAECONNRESET )
		{
			MG_LOG( out_error, "postRecvRequest WSAECONNRESET error!!\n", __MG_FUNC__ );
        }else
        if ( err == WSAENOTCONN )
        {
            MG_LOG( out_error, "postRecvRequest WSAENOTCONN error!!\n", __MG_FUNC__ );
        }

        mNetConnectInfoCs.lock();
        {
            mNetConnectInfo.mWaitRecvPacketCount++;
            if ( mNetConnectInfo.mWaitRecvPacketCount > mNetConnectInfo.mMaxWaitRecvPacketCount )
                mNetConnectInfo.mMaxWaitRecvPacketCount = mNetConnectInfo.mWaitRecvPacketCount;
        }
        mNetConnectInfoCs.unlock();

#endif	

        //失败后移除连接对象
        if ( isOk == false )
        {
            if ( netConnect.isNull() == false )
                exceptClose( netConnect->getID() );
            if ( buffObject )
                destroyNetIocpRecvDataBuffer( buffObject );
        }

		return isOk;
	}
	
    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::processAcceptResponse(NetIocpRecvDataBuffer* buffObject, I32 bytes, I32 error)
    {
        //bytes;

        HANDLE          hrc;
        SOCKADDR_IN     *local_addr = NULL;
        SOCKADDR_IN     *remote_addr = NULL;
        int             local_addr_len;
        int             remote_addr_len;
        I32             clean_socket = FALSE;
        Bool            isOk = false;
        clean_socket = FALSE;
        if ( error != NO_ERROR )
        {
        }

        NetIocpConnectPtr& netConnect = buffObject->mNetIocpConnectPtr;
        if ( buffObject && (netConnect.isNull()==false) )
        {
            mGetAcceptExSockaddrs(
                buffObject->mNetIocpBufBlock.mBuf, 
                0,
                sizeof(SOCKADDR) + 16,
                sizeof(SOCKADDR) + 16,
                (struct sockaddr **)&local_addr,
                &local_addr_len,
                (struct sockaddr **)&remote_addr,
                &remote_addr_len
                );

            netConnect->mAddr = NetAddress(
                NetAddress::net2Host((U32)remote_addr->sin_addr.s_addr),
                NetAddress::net2Host((U16)remote_addr->sin_port));

            // 配置Socket参数
            isOk = configSocketWhenConnected( netConnect->mSocket, true );

            // 设置系统级别超时检测
            setSystemKeepAlive( netConnect->mSocket );    

            // 绑定Socket到IOCP
            hrc = associateSocketWithIOCP(netConnect->mSocket, (ULONG_PTR)netConnect.getPointer());
            if ( hrc == NULL )
            {
                mNetManager->addErrEvent(mHandle, netConnect->getID());
                isOk = false;
            }
            else
            {
				if (mCurrConnections >= mMaxConnections)
				{
					isOk = false;
				}
				else
				{
					isOk = true;

                    {
						//netSocket链接信息
						netConnect->mConnectInfo.mStartTime = MGTimeOp::getCurrTimestamp();
						netConnect->mConnectInfo.mLastComingTime = 0;
						netConnect->mConnectInfo.mLastGoingTime = 0;

						// Accept所有流程 成功
						// 调用回调函数，将回调函数的返回值做为连接的 id
						if ( mNetManager)
						{
							mCurrConnections++;
							mNetManager->addAcceptEvent(mHandle, netConnect->getID(), netConnect->mAddr);
						}
					}
                    
                    // 投递一个接受数据的请求
                    if ( postRecvRequest( buffObject ) == false )
                    {
                        //DYNAMIC_ASSERT( false );
                    }
				}
            }
        }

        // 失败的话则关闭连接
        if ( isOk == false )
        {
			error = WSAGetLastError();
            
			if ( netConnect.isNull() == false )
                exceptClose( netConnect->getID() );
            if ( buffObject )
                destroyNetIocpRecvDataBuffer( buffObject );
        }

        // 无论成功失败都需要重新投递AccerptConnet对象
		//if (mCurrConnections < mMaxConnections)
        {
            postAcceptRequest();
        }
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::processSendResponse(NetIocpOnceSendConfig* configObject, I32 bytes, I32 error)
    {
        //dev;
        //error:

        DYNAMIC_ASSERT( error == NO_ERROR );

#ifdef _DEBUG

        if ( error == WSAENOBUFS )
        {
            MG_LOG( out_error, "postRecvRequest WSAENOBUFS error!!\n", __MG_FUNC__ );
        }else
        if ( error == WSAECONNRESET )
        {
            MG_LOG( out_error, "postRecvRequest WSAECONNRESET error!!\n", __MG_FUNC__ );
        }else
        if ( error == WSAENOTCONN )
        {
            MG_LOG( out_error, "postRecvRequest WSAENOTCONN error!!\n", __MG_FUNC__ );
        }

        if (bytes > 0)
        {
			// 由于投递后缓存对象没有锁定,则下面的实现暂时屏蔽
            //NetIocpConnectPtr netConnect = buffObject->getNetIocpConnectPtr();
            //if ( buffObject && (netConnect.isNull()==false) )
            //{
            //    recordSendLog( netConnect.getPointer(), bytes );
            //}

            //if ( buffObject->mNetIocpBufBlock.mUsedBufLen != bytes )
            //{
            //    //需要投递剩余部分
            //    DYNAMIC_ASSERT(0);
            //}
        }

        mNetConnectInfoCs.lock();
        {
            mNetConnectInfo.mWaitSendPacketCount--;
            if ( mNetConnectInfo.mWaitSendPacketCount < 0 )
            {
                mNetConnectInfo.mWaitSendPacketCount = 0;
            }
        }
        mNetConnectInfoCs.unlock();

#endif	

        //需要测试当WSA_IO_PENDING时，IOCP是否会先将数据缓冲复制到分页内存中，还是锁定数据内存？ 等待下次空闲时复制到TCP/IP缓冲中发送
        //如果是这样则需要处理多次发送等待。不能一次清空 mWrite Buff
        //netSocket->lock();
        //{
        //	DYNAMIC_ASSERT(bytes == netSocket->mWrite.mBufLen);
        //}
        //netSocket->unlock();

        //释放发送数据缓存对象
        if ( configObject )
        {
			if (bytes != 0) 
			{
				if ( configObject->getAndSubWsabufBuffSize(bytes) == 0 )
				{
					mNetManager->addSendEvent(configObject);
				}else
				{
					DYNAMIC_EEXCEPT_LOG("Win32IocpNetDriver::processSendResponse : not send all buff size!");
				}
			}
        }
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::processRecvResponse(NetIocpRecvDataBuffer* buffObject, I32 bytes, I32 error)
    {
        //error;

        Bool isOk = false;

        NetIocpConnectPtr& netConnect = buffObject->mNetIocpConnectPtr;
        if ( buffObject && (netConnect.isNull()==false) )
        {
            //bytes为此次iocp投递缓冲获得的数据长度
            if ( bytes > 0 )
            {	
                // Iocp网络包最底层数据结构
                NetIocpPacketData* iocpPacketData = NULL;
                // 根据发包协议解包
                Char8*	packetHeader = buffObject->mNetIocpBufBlock.mBuf;
                // 包头长度
                Int		packetHeaderLen = NetIocpPacketData_INFO::headSize;
                Char8*	dataHeader = NULL;
                U16		dataLen = 0;
                // 剩余有效缓冲长度＝以前未处理的缓冲长度+此次投递获得的新缓冲长度
                Int		spareBuffLen = buffObject->mNetIocpBufBlock.mBufValidPos + bytes; 

                // 临时设置标志位
                isOk = true;

                if ( spareBuffLen > packetHeaderLen )
                {
                    // 循环从底层缓冲区里面取出数据包//根据包头
                    while ( true )
                    {	
                        // 转换为网络包数据结构
                        iocpPacketData       = (NetIocpPacketData*)(packetHeader);

                        // 判断标志头
                        if ( iocpPacketData->tag != MG_NET_PACKET_TAG )
                        {
                            DYNAMIC_ASSERT(0);
                            // 标志错误
                            isOk = false;
                            break;
                        }else
                        {
                            // 获得实际数据包头
                            dataHeader  = iocpPacketData->data;
                            // 数据包长度
                            dataLen     = iocpPacketData->dataLen;

                            //如果数据包长度 大于 底层缓冲区大小，则判断错误
                            if ( dataLen > (buffObject->mNetIocpBufBlock.mBufferSize - packetHeaderLen) )
                            {
                                isOk = false;
                                break;
                            }else
                            if ( dataLen <= 0 )
                            {
                                isOk = false;
                                break;
                            }

                            //剩余有效缓冲长度 大于 数据包长度则有效
                            if ( spareBuffLen >= (packetHeaderLen + dataLen)  )
                            {
                                isOk = true;
                                mNetManager->addRecvEvent(mHandle, netConnect->mId, dataHeader, dataLen);
                                spareBuffLen -= packetHeaderLen;
                                spareBuffLen -= dataLen;
                                packetHeader = dataHeader + dataLen;
                            }else
                            {
                                break;
                            }

                            if ( spareBuffLen <=  packetHeaderLen )
                            {
                                break;
                            }
                        }
                    }

                    //剩余有效缓冲长度 小于 数据包长度则推出循环
                    //保留未处理数据给下次处理
                    if (spareBuffLen>0)
                    {
                        // 监听下该事件
                        memmove(buffObject->mNetIocpBufBlock.mBuf,packetHeader,spareBuffLen);
                        buffObject->mNetIocpBufBlock.mBufValidPos = spareBuffLen;
                    }else
                    {
                        buffObject->mNetIocpBufBlock.mBufValidPos = 0;
                    }
                }

#ifdef _DEBUG
                recordRecvLog( netConnect.getPointer(), bytes );

                mNetConnectInfoCs.lock();
                {
                    mNetConnectInfo.mWaitRecvPacketCount--;
                    if ( mNetConnectInfo.mWaitRecvPacketCount < 0 )
                    {
                        mNetConnectInfo.mWaitRecvPacketCount = 0;
                    }
                }
                mNetConnectInfoCs.unlock();
#endif	
            }
        }
   
        if ( isOk )
        {
            //重新发送一个请求
            postRecvRequest( buffObject );
        }else
        {   
			error = WSAGetLastError();
            if ( netConnect.isNull() == false )
                exceptClose( netConnect->getID() );
            if ( buffObject )
                destroyNetIocpRecvDataBuffer( buffObject );
        }

    }

	//-----------------------------------------------------------------------------
	Ptr Win32IocpNetDriver::createIOCP()
	{
		if ( mCompletionPort == NULL )
		{
			mCompletionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0 );
			if ( mCompletionPort == NULL )
			{
				mErrorNo = GetLastError();
			}
		}
		return mCompletionPort;
	}

	//-----------------------------------------------------------------------------
	void Win32IocpNetDriver::destroyIOCP()
	{
		if ( mCompletionPort != NULL )
		{
			CloseHandle( mCompletionPort );
			mCompletionPort = NULL;
		}
	}

	//-----------------------------------------------------------------------------
	Ptr Win32IocpNetDriver::getIOCP()
	{
		return mCompletionPort;
	}
	
	//-----------------------------------------------------------------------------
	HANDLE Win32IocpNetDriver::associateSocketWithIOCP(SOCKET socket,ULONG_PTR key)
	{
		HANDLE	hrc=0;
		
		if ( mCompletionPort != NULL )
		{
			hrc = CreateIoCompletionPort( (HANDLE)socket, mCompletionPort, (ULONG_PTR)key, 0 );
		}

		if ( (Int)hrc==0 )
		{
			mErrorNo = GetLastError();
		}
		return hrc;
	}

	//-----------------------------------------------------------------------------
	SOCKET Win32IocpNetDriver::createTcpSocket()
	{
		SOCKET res = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if (res == INVALID_SOCKET)
		{
			mErrorNo = WSAGetLastError();
		}
		return res;
	}

	//-----------------------------------------------------------------------------
	void Win32IocpNetDriver::destroySocket(SOCKET& socket)
	{
		if ( socket )
		{
			closesocket(socket);
			socket = NULL;
		}
	}

    //-----------------------------------------------------------------------------
    Bool Win32IocpNetDriver::configSocketWhenConnected(SOCKET socket, Bool isAccept)
    {
        Bool isOK = true ;

        /////////////////////////////////////////////////////////////////////////////////////////

        // 设置linger属性 为关闭连接时不用等待 
        LINGER ling; 
        ling.l_onoff    = 1; 
        ling.l_linger   = 5; 
        if (setsockopt(socket, SOL_SOCKET, SO_LINGER, (const char *) &ling, sizeof(ling)))
        {
            isOK = false;
        }

        // TODO: 需要修改
        // 设置socket缓冲区大小
        if (setsockopt( socket, SOL_SOCKET, SO_SNDBUF, (const char*)&mSocketSendBufferSize, sizeof(U32)))
        {
            isOK = false;
        }

        if (setsockopt( socket, SOL_SOCKET, SO_RCVBUF, (const char*)&mSocketRecvBufferSize, sizeof(U32)))
        {
            isOK = false;
        } 

        /////////////////////////////////////////////////////////////////////////////////////////

        // isAccept == true
        if ( isAccept )
        {
            // 拷贝listenSocket属性到连接socket避免s关闭连接时候出现10057错误
            if (setsockopt( socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&mListenSocket, sizeof(mListenSocket)))
            {
                isOK = false;
            }
        }

        return isOK;
    }

	//-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::configAcceptSocket(SOCKET socket)
	{
		// 获得 AcceptEx 的地址
		GUID	guidAcceptEx			   = WSAID_ACCEPTEX;
		GUID	guidGetAcceptExSockaddrs   = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD   bytes = 0;
		int rc = 0;
		rc = WSAIoctl( socket, SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&guidAcceptEx, sizeof(guidAcceptEx), 
			&mAcceptEx, sizeof(mAcceptEx), 
			&bytes, NULL, NULL );
		if ( rc == SOCKET_ERROR )
		{
			mErrorNo = WSAGetLastError();
			return false;
		}

		// 获得 GetAcceptExSockaddrs 的地址
		rc = WSAIoctl( socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidGetAcceptExSockaddrs, sizeof(guidGetAcceptExSockaddrs),
			&mGetAcceptExSockaddrs, sizeof(mGetAcceptExSockaddrs),
			&bytes, NULL, NULL );
		if ( rc == SOCKET_ERROR )
		{
			mErrorNo = WSAGetLastError();
			return false;
		}

		return true;
	}
    
	//-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::createIOCPStatusThreads(Int count)
	{
		if ( count > 0 )
		{
			mThreadCount = count;
		}else
		{
			//根据CPU记录网络事件处理线程
			SYSTEM_INFO		sysinfo;
			GetSystemInfo( &sysinfo );
			mCpuCount = sysinfo.dwNumberOfProcessors;
			mThreadCount = mCpuCount * 2;
		}
		if ( mThreadCount > MG_NET_MAX_THREADS )
		{
			mThreadCount = MG_NET_MAX_THREADS;
		}

		for (int i = 0; i < mThreadCount; i++ )
		{
			if ( mThreadHandles[i].create( handleIOCPStatusThread,this,"handleIOCPStatusThread" ) == false )
			{
				mErrorNo = GetLastError();
				return false;
			}
		}
		return true;
	}

	//-----------------------------------------------------------------------------
	void Win32IocpNetDriver::destroyIOCPStatusThreads()
	{
        if ( mCompletionPort != NULL )
        {
            for (int i = 0; i < mThreadCount; i++ )
            {
                PostQueuedCompletionStatus( mCompletionPort, 0, DWORD(NET_CLOSE), 0 );
            }
        }

		for (int i = 0; i < mThreadCount; i++ )
		{
			mThreadHandles[i].destory();
		}
	}

    //-----------------------------------------------------------------------------
    NetIocpConnectPtr Win32IocpNetDriver::addConnectContainer()
    {
        NetIocpConnectPtr   netConnect;
        NetIocpConnect*     netConnectPoint = NULL;
        I32 id = 0;

        //连接容器加锁
        mNetIocpConnectListCs.writeLock();
        {   
            //得到当前连接数量
            UInt checkCount = mNetIocpConnectList.size();
            //遍历一定数量
            for( UInt i=0; i <= checkCount; i++ )
            {
                //得到当前递增编号
                //最大数量为 2 147 483 647，一般够用
                id = mNetIocpConnectIndexGenerator++;
                //如果小于零则归零
                if ( mNetIocpConnectIndexGenerator < 0 )
                {
                    mNetIocpConnectIndexGenerator = 0;
                }

                //查找是否有重编号的连接，如果没有则创建新连接
                //TODO:有极小机率在递增编号归零后出错
                if ( mNetIocpConnectList.find(id) == mNetIocpConnectList.end() )
                {
                    //创建新的连接
                    netConnectPoint = createConnectContainer( id );
                    //退出遍历检测
                    break;
                }
            }

            //保存连接容器
            if ( netConnectPoint )
            {
                netConnectPoint->setID( id );
                netConnect.bind(netConnectPoint);
                mNetIocpConnectList[id] = netConnect;
            }
        }
        //连接容器解锁
        mNetIocpConnectListCs.writeUnLock();
        
        return netConnect;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::removeConnectContainer(NetIocpConnect* socketObject, Bool isException, Bool isClearSocketRecord)
    {
        if ( socketObject )
        {
            removeConnectContainer( socketObject->getID(), isException, isClearSocketRecord );
        }
    }
    
    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::removeConnectContainer(I32 id,Bool isException, Bool isClearSocketRecord)
    {
		//递减连接数量
        SOCKET socket = INVALID_SOCKET;

        mNetIocpConnectListCs.writeLock();
        {
            std::map<I32,NetIocpConnectPtr>::iterator iter = mNetIocpConnectList.find(id);
            if ( iter != mNetIocpConnectList.end() )
            {
                NetIocpConnectPtr& netConnect = iter->second;
                if (  netConnect.isNull() == false )
                {
                    //判断是否有连接
                    //if ( netConnect->isConnected() )
                    {
                        // 获得当前socket
                        socket = netConnect->mSocket;
                        //增加一个关闭事件
                        mNetManager->addCloseEvent(mHandle,id,isException,netConnect->mAddr);
                        // 关闭连接
                        // iocp会接受到一个0字节的关闭消息
                        netConnect->diconnect();
                    }
                }
                //移除连接
				mNetIocpConnectList.erase(iter);
            }
        }
        mNetIocpConnectListCs.writeUnLock();

        if ( isClearSocketRecord )
            removeCheckSocketValidRecord( socket );
    }

    //-----------------------------------------------------------------------------
    NetIocpConnect* Win32IocpNetDriver::createConnectContainer( I32 id )
    {
        NetIocpConnect* netConnect = NULL;
        mNetIocpConnectPool.threadLock();
        {
            netConnect = MG_POOL_NEW_ONCE_CONSTRUCTION(mNetIocpConnectPool,NetIocpConnect,(this));
            netConnect->clear();
            netConnect->setID( id );
        }
        mNetIocpConnectPool.threadUnLock();

        //递增连接数量
        //mCurrConnections++;

        return netConnect;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::destroyConnectContainer(NetIocpConnect* socketObject)
    {
		//递增连接数量
		mCurrConnections--;
        mNetIocpConnectPool.threadLock();
        {
            MG_POOL_DELETE_ONCE_DESTRUCTOR(mNetIocpConnectPool,socketObject);
        }
        mNetIocpConnectPool.threadUnLock();

       
    }

    //-----------------------------------------------------------------------------
    NetIocpRecvDataBuffer* Win32IocpNetDriver::createNetIocpRecvDataBuffer( NetIocpConnectPtr& netConnect )
    {
        NetIocpRecvDataBuffer* buffObject = NULL;
        mNetIocpRecvBuffPoolCs.lock();
        {
            buffObject = MG_POOL_NEW_ONCE_CONSTRUCTION(mNetIocpRecvBuffPool,NetIocpRecvDataBuffer,(this, mRecvBufferSize));
        }
        mNetIocpRecvBuffPoolCs.unlock();

        // 清空属性
        buffObject->clear();
        // 绑定网络连接智能指针到缓存对象上
        buffObject->mNetIocpConnectPtr = netConnect;
        return buffObject;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::destroyNetIocpRecvDataBuffer(NetIocpRecvDataBuffer* buffObject)
    {
        buffObject->mNetIocpConnectPtr.setNull();
        mNetIocpRecvBuffPoolCs.lock();
        {
            MG_POOL_DELETE_ONCE_DESTRUCTOR(mNetIocpRecvBuffPool,buffObject);
        }
        mNetIocpRecvBuffPoolCs.unlock();
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::destroyAllNetIocpRecvDataBuffer()
    {
        //监听也用到,所以暂时不能删除
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::removeAllConnectContainers()
    {
        //加锁,遍历连接对象
        mNetIocpConnectListCs.writeLock();
        {
            std::map<I32,NetIocpConnectPtr>::iterator iter = mNetIocpConnectList.begin();
            std::map<I32,NetIocpConnectPtr>::iterator end_iter = mNetIocpConnectList.end();
            for ( ;iter != end_iter; iter++ )
            {
                NetIocpConnectPtr& netConnect = iter->second;
                if (  netConnect.isNull() == false )
                {
                    //判断是否有连接
                    if ( netConnect->isConnected() )
                    {
						// 递减连接数
						//mCurrConnections--;
                        mNetManager->addCloseEvent(mHandle, netConnect->getID(), true, netConnect->getAddress());
                        // 关闭连接
                        // iocp会接受到一个0字节的关闭消息
                        netConnect->diconnect();
                    }
                }
            }

            mNetIocpConnectList.clear();
            mNetIocpConnectSocketMap.clear();
        }
        mNetIocpConnectListCs.writeUnLock();

        //////////////////////////////////////////////////////////////////////

        // 释放缓存对象
        destroyAllNetIocpRecvDataBuffer();
    }
    //-----------------------------------------------------------------------------
    NetIocpConnectPtr	Win32IocpNetDriver::getConnectContainer(I32 id)
    {
        NetIocpConnectPtr netConnect;
        mNetIocpConnectListCs.readLock();
        {
            std::map<I32,NetIocpConnectPtr>::iterator iter = mNetIocpConnectList.find(id);
            if ( iter != mNetIocpConnectList.end() )
            {
                netConnect = iter->second;
            }
        }
        mNetIocpConnectListCs.readUnLock();
        return netConnect;
    }

    //-----------------------------------------------------------------------------
    U64 Win32IocpNetDriver::getAddressUin(I32 id)
    {
        NetIocpConnectPtr netConnect = getConnectContainer(id);
        if (netConnect.isNull()==false)
        {
			U64 uin = netConnect->getAddress().getUin();
            return uin;
        }
        return 0;
    }

    //-----------------------------------------------------------------------------
    NetAddress Win32IocpNetDriver::getAddress(I32 id)
    {
        NetAddress result;
        NetIocpConnectPtr netConnect = getConnectContainer(id);
        if ( netConnect.isNull() == false )
        {
            result = netConnect->getAddress();
        }
        return result;
    }

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetDriver::isInitialized()
	{
		return mNetDriverInited;
	}

    //-----------------------------------------------------------------------------
	void Win32IocpNetDriver::setHandle( I32 handle )
	{
		mHandle = handle;
	}
    //-----------------------------------------------------------------------------
	I32 Win32IocpNetDriver::getHandle( )
	{
		return mHandle;
	}
    //-----------------------------------------------------------------------------
	NetConnectInfo* Win32IocpNetDriver::getNetInfo()
	{
		return &mNetConnectInfo;
	}
    //-----------------------------------------------------------------------------
	NetConnectInfo* Win32IocpNetDriver::getConnectInfo( I32 id )
	{
		 NetIocpConnectPtr netConnect = getConnectContainer(id);
		 if (netConnect.isNull()==false)
		 {
			return &(netConnect->mConnectInfo);
		 }
		 return NULL;
	}
	//-----------------------------------------------------------------------------
	I32 Win32IocpNetDriver::getCurrConnectionCount()
	{
		return mCurrConnections;
	}

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::recordSendLog( NetIocpConnect* netSocket, I32 bytes )
    {
        if (!netSocket)
        {
            return;
        }
        //计算driver的链接信息
        if (mNetConnectInfo.mLastComingTime == 0)
        {
            mNetConnectInfo.mLastGoingTime = MGTimeOp::getCurrTick();
            mNetConnectInfo.mGoingRateBytes += bytes;
            mNetConnectInfo.mGoingBytes += bytes;
            mNetConnectInfo.mComingRateMaxBytes = 0;
            mNetConnectInfo.mComingRateMinBytes = 999999999.0f;
            mNetConnectInfo.mGoingRateCounts++;
            mNetConnectInfo.mGoingCounts++;
            mNetConnectInfo.mComingRateMaxCounts = 0;
            mNetConnectInfo.mComingRateMinCounts = 999999999.0f;
        }
        else
        {
            U32 tick =  MGTimeOp::getCurrTick();
            I32 interval = tick - mNetConnectInfo.mLastGoingTime;
            const I32 calcTime = 5000; //5 秒计算一次
            if (interval >= calcTime)
            {

                mNetConnectInfo.mGoingRateBytes += bytes;
                mNetConnectInfo.mGoingLastRateBytes = mNetConnectInfo.mGoingRateBytes * 1000 / interval ; //计算上5秒比率
                mNetConnectInfo.mGoingRateBytes = 0;
                mNetConnectInfo.mGoingBytes += bytes;
                if (mNetConnectInfo.mGoingLastRateBytes > mNetConnectInfo.mGoingRateMaxBytes)
                {
                    mNetConnectInfo.mGoingRateMaxBytes = mNetConnectInfo.mGoingLastRateBytes;
                }
                if(mNetConnectInfo.mGoingLastRateBytes < mNetConnectInfo.mGoingRateMinBytes)
                {
                    mNetConnectInfo.mGoingRateMinBytes = mNetConnectInfo.mGoingLastRateBytes;
                }

                mNetConnectInfo.mGoingRateCounts += 1;
                mNetConnectInfo.mGoingLastRateCounts = mNetConnectInfo.mGoingRateCounts * 1000 / interval;
                mNetConnectInfo.mGoingRateCounts = 0;
                mNetConnectInfo.mGoingCounts++;
                if (mNetConnectInfo.mGoingLastRateCounts > mNetConnectInfo.mGoingRateMaxCounts)
                {
                    mNetConnectInfo.mGoingRateMaxCounts = mNetConnectInfo.mGoingLastRateCounts;
                }
                if (mNetConnectInfo.mGoingLastRateCounts < mNetConnectInfo.mGoingRateMinCounts)
                {
                    mNetConnectInfo.mGoingRateMinCounts = mNetConnectInfo.mGoingLastRateCounts;
                }

                mNetConnectInfo.mLastGoingTime = tick;
            }
            else
            {
                /*netSocket->mConnectInfo.mGoingRateBytes += bytes;
                netSocket->mConnectInfo.mGoingBytes += bytes;
                netSocket->mConnectInfo.mGoingRateCounts++;
                netSocket->mConnectInfo.mGoingCounts++;*/
                mNetConnectInfo.mGoingRateBytes += bytes;
                mNetConnectInfo.mGoingBytes += bytes;
                mNetConnectInfo.mGoingRateCounts++;
                mNetConnectInfo.mGoingCounts++;

            }
        }

        //计算单个链接信息
        if (netSocket->mConnectInfo.mLastGoingTime == 0)
        {
            netSocket->mConnectInfo.mLastGoingTime = MGTimeOp::getCurrTick();

            netSocket->mConnectInfo.mGoingRateBytes += bytes;
            netSocket->mConnectInfo.mGoingBytes += bytes;
            netSocket->mConnectInfo.mComingRateMaxBytes = 0;
            netSocket->mConnectInfo.mComingRateMinBytes = 999999999.0f;
            netSocket->mConnectInfo.mGoingRateCounts++;
            netSocket->mConnectInfo.mGoingCounts++;
            netSocket->mConnectInfo.mComingRateMaxCounts = 0;
            netSocket->mConnectInfo.mComingRateMinCounts = 999999999.0f;
        }
        else
        {
            U32 tick =  MGTimeOp::getCurrTick();
            I32 interval = tick - netSocket->mConnectInfo.mLastGoingTime;
            const I32 calcTime = 5000; //5 秒计算一次
            if (interval >= calcTime)
            {

                netSocket->mConnectInfo.mGoingRateBytes += bytes;
                netSocket->mConnectInfo.mGoingLastRateBytes = netSocket->mConnectInfo.mGoingRateBytes * 1000 / interval; //计算上5秒比率
                netSocket->mConnectInfo.mGoingRateBytes = 0;
                netSocket->mConnectInfo.mGoingBytes += bytes;
                if (netSocket->mConnectInfo.mGoingLastRateBytes > netSocket->mConnectInfo.mGoingRateMaxBytes)
                {
                    netSocket->mConnectInfo.mGoingRateMaxBytes = netSocket->mConnectInfo.mGoingLastRateBytes;
                }
                if(netSocket->mConnectInfo.mGoingLastRateBytes < netSocket->mConnectInfo.mGoingRateMinBytes)
                {
                    netSocket->mConnectInfo.mGoingRateMinBytes = netSocket->mConnectInfo.mGoingLastRateBytes;
                }

                netSocket->mConnectInfo.mGoingRateCounts += 1;
                netSocket->mConnectInfo.mGoingLastRateCounts = netSocket->mConnectInfo.mGoingRateCounts * 1000 / interval;
                netSocket->mConnectInfo.mGoingRateCounts = 0;
                netSocket->mConnectInfo.mGoingCounts++;
                if (netSocket->mConnectInfo.mGoingLastRateCounts > netSocket->mConnectInfo.mGoingRateMaxCounts)
                {
                    netSocket->mConnectInfo.mGoingRateMaxCounts = netSocket->mConnectInfo.mGoingLastRateCounts;
                }
                if (netSocket->mConnectInfo.mGoingLastRateCounts < netSocket->mConnectInfo.mGoingRateMinCounts)
                {
                    netSocket->mConnectInfo.mGoingRateMinCounts = netSocket->mConnectInfo.mGoingLastRateCounts;
                }

                netSocket->mConnectInfo.mLastGoingTime = tick;
            }
            else
            {
                netSocket->mConnectInfo.mGoingRateBytes += bytes;
                netSocket->mConnectInfo.mGoingBytes += bytes;

                netSocket->mConnectInfo.mGoingRateCounts++;
                netSocket->mConnectInfo.mGoingCounts++;
            }
        }
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetDriver::recordRecvLog( NetIocpConnect* netSocket, I32 bytes )
    {
        ///////////////////////////////////////////////////////////////////////////////////////

        //检测用数据计算

        //计算driver的链接信息
        if (mNetConnectInfo.mLastComingTime == 0)
        {
            mNetConnectInfo.mLastComingTime = MGTimeOp::getCurrTick();
            mNetConnectInfo.mComingRateBytes += bytes;
            mNetConnectInfo.mComingBytes += bytes;
            mNetConnectInfo.mComingRateMaxBytes = 0;
            mNetConnectInfo.mComingRateMinBytes = 999999999.0f;
            mNetConnectInfo.mComingRateCounts++;
            mNetConnectInfo.mComingCounts++;
            mNetConnectInfo.mComingRateMaxBytes = 0;
            mNetConnectInfo.mComingRateMinBytes = 999999999.0f;
        }
        else
        {
            U32 tick =  MGTimeOp::getCurrTick();
            I32 interval = tick - mNetConnectInfo.mLastComingTime;
            const I32 calcTime = 5000; //5 秒计算一次
            if (interval >= calcTime)
            {

                mNetConnectInfo.mComingRateBytes += bytes;
                mNetConnectInfo.mComingLastRateBytes = mNetConnectInfo.mComingRateBytes * 1000 / interval; //计算上5秒比率
                mNetConnectInfo.mComingRateBytes = 0;
                mNetConnectInfo.mComingBytes += bytes;
                if (mNetConnectInfo.mComingLastRateBytes > mNetConnectInfo.mComingRateMaxBytes)
                {
                    mNetConnectInfo.mComingRateMaxBytes = mNetConnectInfo.mComingLastRateBytes;
                }
                if(mNetConnectInfo.mComingLastRateBytes < mNetConnectInfo.mComingRateMinBytes)
                {
                    mNetConnectInfo.mComingRateMinBytes = mNetConnectInfo.mComingLastRateBytes;
                }

                mNetConnectInfo.mComingRateCounts += 1;
                mNetConnectInfo.mComingLastRateCounts = mNetConnectInfo.mComingRateCounts * 1000 / interval;
                mNetConnectInfo.mComingRateCounts = 0;
                mNetConnectInfo.mComingCounts++;
                if (mNetConnectInfo.mComingLastRateCounts > mNetConnectInfo.mComingRateMaxCounts)
                {
                    mNetConnectInfo.mComingRateMaxCounts = mNetConnectInfo.mComingLastRateCounts;
                }
                if (mNetConnectInfo.mComingLastRateCounts < mNetConnectInfo.mComingRateMinCounts)
                {
                    mNetConnectInfo.mComingRateMinCounts = mNetConnectInfo.mComingLastRateCounts;
                }

                mNetConnectInfo.mLastComingTime = tick;
            }
            else
            {
                /*netSocket->mConnectInfo.mComingRateBytes += bytes;
                netSocket->mConnectInfo.mComingBytes += bytes;

                netSocket->mConnectInfo.mComingRateCounts++;
                netSocket->mConnectInfo.mComingCounts++;*/
                mNetConnectInfo.mComingRateBytes += bytes;
                mNetConnectInfo.mComingBytes += bytes;

                mNetConnectInfo.mComingRateCounts++;
                mNetConnectInfo.mComingCounts++;
            }
        }

        //计算单个链接信息
        if (netSocket->mConnectInfo.mLastComingTime == 0)
        {
            netSocket->mConnectInfo.mLastComingTime = MGTimeOp::getCurrTick();

            netSocket->mConnectInfo.mComingRateBytes += bytes;
            netSocket->mConnectInfo.mComingBytes += bytes;
            netSocket->mConnectInfo.mComingRateMaxBytes = 0;
            netSocket->mConnectInfo.mComingRateMinBytes = 999999999.0f;

            netSocket->mConnectInfo.mComingRateCounts++;
            netSocket->mConnectInfo.mComingCounts++;
            netSocket->mConnectInfo.mComingRateMaxCounts = 0;
            netSocket->mConnectInfo.mComingRateMinCounts = 999999999.0f;
        }
        else
        {
            U32 tick =  MGTimeOp::getCurrTick();
            I32 interval = tick - netSocket->mConnectInfo.mLastComingTime;
            const I32 calcTime = 5000; //5 秒计算一次
            if (interval >= calcTime)
            {

                netSocket->mConnectInfo.mComingRateBytes += bytes;
                netSocket->mConnectInfo.mComingLastRateBytes =netSocket->mConnectInfo.mComingRateBytes * 1000 / interval; //计算上5秒比率
                netSocket->mConnectInfo.mComingRateBytes = 0;
                netSocket->mConnectInfo.mComingBytes += bytes;
                if (netSocket->mConnectInfo.mComingLastRateBytes > netSocket->mConnectInfo.mComingRateMaxBytes)
                {
                    netSocket->mConnectInfo.mComingRateMaxBytes = netSocket->mConnectInfo.mComingLastRateBytes;
                }
                if(netSocket->mConnectInfo.mComingLastRateBytes < netSocket->mConnectInfo.mComingRateMinBytes)
                {
                    netSocket->mConnectInfo.mComingRateMinBytes = netSocket->mConnectInfo.mComingLastRateBytes;
                }

                netSocket->mConnectInfo.mComingRateCounts += 1;
                netSocket->mConnectInfo.mComingLastRateCounts = netSocket->mConnectInfo.mComingRateCounts * 1000 / interval;
                netSocket->mConnectInfo.mComingRateCounts = 0;
                netSocket->mConnectInfo.mComingCounts++;
                if (netSocket->mConnectInfo.mComingLastRateCounts > netSocket->mConnectInfo.mComingRateMaxCounts)
                {
                    netSocket->mConnectInfo.mComingRateMaxCounts = netSocket->mConnectInfo.mComingLastRateCounts;
                }
                if (netSocket->mConnectInfo.mComingLastRateCounts < netSocket->mConnectInfo.mComingRateMinCounts)
                {
                    netSocket->mConnectInfo.mComingRateMinCounts = netSocket->mConnectInfo.mComingLastRateCounts;
                }

                netSocket->mConnectInfo.mLastComingTime = tick;
            }
            else
            {
                netSocket->mConnectInfo.mComingRateBytes += bytes;
                netSocket->mConnectInfo.mComingBytes += bytes;

                netSocket->mConnectInfo.mComingRateCounts++;
                netSocket->mConnectInfo.mComingCounts++;
            }
        }
    }

}