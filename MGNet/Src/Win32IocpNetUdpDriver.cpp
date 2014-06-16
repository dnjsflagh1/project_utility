/******************************************************************************/
#include "stdafx.h"
#include "Win32IocpNetUdpDriver.h"
#include "MGStrOp.h"
#include "NetErrorDefine.h"
#include "NetAddress.h"
#include "MGStrOp.h"
#include "NetUdpManager.h"
/******************************************************************************/
namespace MG
{
    //-----------------------------------------------------------------------------
    Win32IocpNetUdpDriver::Win32IocpNetUdpDriver()
        :mNetDriverInited(false),
		mNetDriverIniting(false),
        mNetDriverListened(false),
        mIsNetworkTimeoutValid(true),
		mMaxConnections(1),
        mCompletionPort(NULL),
        mNetUdpManager(NULL),
		mWaitAcceptSocketCount(5),
        mIocpNetUdpConnectIndexGenerator(0),
		mCurrConnections(0),
        mSendNormalBufferSize(0),
        mSendBigBufferSize(0),
        mRecvBufferSize(0),
		mLocalSockets(NULL),
		mLocalSocketsCount(0)
    {
    }

    //-----------------------------------------------------------------------------
    Win32IocpNetUdpDriver::~Win32IocpNetUdpDriver()
    {
        unInitialize();
    }

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetUdpDriver::initialize( NetUdpDriverInitParameter& parameter,NetUdpManager* mgr )
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
			mName                   = parameter.name;
			mIsNetworkTimeoutValid  = parameter.isSetNetworkTimeout;
			mMaxConnections         = parameter.maxConnections;
			mWaitAcceptSocketCount  = (mMaxConnections / 2 + 1) > parameter.waitAcceptSocketCount ? parameter.waitAcceptSocketCount : (mMaxConnections/2);
            mSendNormalBufferSize   = parameter.sendNormalBuffSize;
            mSendBigBufferSize      = parameter.sendBigBuffSize;
            mIsCloseWhenError       = parameter.isCloseWhenError;
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
		mNetDriverInited = true;
		mNetDriverIniting = false;
		
		return true;
	}

    //-----------------------------------------------------------------------------
	I32 Win32IocpNetUdpDriver::update()
	{
		//检查链接 是否有非法链接
		return 1;
	}

    //-----------------------------------------------------------------------------
	I32 Win32IocpNetUdpDriver::unInitialize()
	{
        if ( mNetDriverInited == true )
        {
            mNetDriverInited = false;
			
			//关闭所有连接
			closeAll();

            //关闭网络事件处理线程
			destroyIOCPStatusThreads();

			Sleep(10);

            //关闭完全端口
			destroyIOCP();
			
            WSACleanup();
			
			mCurrConnections = 0;
            //销毁网络连接对象池
			//destroyAllConnectContainers();
        }
		return 0;
	}

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetUdpDriver::setManager( NetUdpManager* mgr )
	{
		DYNAMIC_ASSERT(mgr);
		if (!mgr)
		{
			return false;
		}
		mNetUdpManager = mgr;
		return true;
	}
    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::setCode(I32 id, U64 code)
    {
        IocpNetUdpConnectPtr netConnect = getConnectContainer(id);
        if ( netConnect.isNull() == false )
        {
            netConnect->setCode(code);
        }
    }

    //-----------------------------------------------------------------------------
    Bool Win32IocpNetUdpDriver::send(IocpNetUdpSendDataBuffer* buffObject, I32 id,U16 socketIndex, U64 code)
    {
        Bool result = false;

        //得到网络连接对象
        IocpNetUdpConnectPtr netConnect = getConnectContainer(id);

        if ( netConnect.isNull() == false )
        {
            if ( netConnect->checkCode(code)==true )
            {
                // 绑定网络连接智能指针到缓存对象上
				buffObject->addr = netConnect.getPointer()->getAddress();
                IocpNetUdpSectionPacket* packet = buffObject->getIocpNetSectionPacket();
                {
                    UInt realSendLen = IocpNetUdpSectionPacket_INFO::dataSize(*packet);
                    if ( realSendLen <= buffObject->mIocpNetUdpBufBlock.mBufferSize )
                    {
                        buffObject->mIocpNetUdpBufBlock.mUsedBufLen = realSendLen;
                    }else
                    {
                        DYNAMIC_ASSERT(0);
                    }
                }

                result = postSendRequest( buffObject,socketIndex);

				//TODO:
				//由于processSendResponse会用到mIocpNetUdpConnectPtr,是否需要清空?
            }
        }
        return result;
    }
	//-----------------------------------------------------------------------------
	MG::Bool Win32IocpNetUdpDriver::send( IocpNetUdpSendDataBuffer* buffObject, NetAddress& addr,U16 socketIndex)
	{
		Bool result = false;
		buffObject->addr = addr;
		IocpNetUdpSectionPacket* packet = buffObject->getIocpNetSectionPacket();
		{
			UInt realSendLen = IocpNetUdpSectionPacket_INFO::dataSize(*packet);
			if ( realSendLen <= buffObject->mIocpNetUdpBufBlock.mBufferSize )
			{
				buffObject->mIocpNetUdpBufBlock.mUsedBufLen = realSendLen;
			}else
			{
				DYNAMIC_ASSERT(0);
			}
		}

		result = postSendRequest( buffObject, socketIndex);

		//TODO:
		//由于processSendResponse会用到mIocpNetUdpConnectPtr,是否需要清空?
		return result;

	}
    //-----------------------------------------------------------------------------
	I32 Win32IocpNetUdpDriver::close( I32 id )
	{
		removeConnectContainer(id,false);
		return 1;
	}

    //-----------------------------------------------------------------------------
    I32 Win32IocpNetUdpDriver::passivityClose(I32 id)
    {
        removeConnectContainer(id,true);
        return 1;
    }

    //-----------------------------------------------------------------------------
	void Win32IocpNetUdpDriver::closeAll()
	{
		removeAllConnectContainers();
	}

    //-----------------------------------------------------------------------------
	Str Win32IocpNetUdpDriver::getName() const
	{
		return mName;
	}

    //-----------------------------------------------------------------------------
	CChar* Win32IocpNetUdpDriver::getDriverDesc() const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------
	void Win32IocpNetUdpDriver::handleIOCPStatusThread(Ptr ptr)
	{
		OneThreadHandle* thd = (OneThreadHandle*)ptr;	
		if (thd)
		{
			if (thd->getParam())
			{
				Win32IocpNetUdpDriver*dev  = (Win32IocpNetUdpDriver *)(thd->getParam());
				IocpNetUdpBufBlock			*block = NULL;
				OVERLAPPED				*pol = NULL;
				DWORD					bytes = 0;
				DWORD					flags = 0;
				ULONG_PTR				key = 0;
				Int						rc;
				Int						error;
				Int						pass = 1;
				SOCKET					socket;

                IocpNetUdpSendDataBuffer*  sendBuffObject;
                IocpNetUdpRecvDataBuffer*  recvBuffObject;

				while ( pass && thd->isRun())
				{	
                    sendBuffObject = NULL;
                    recvBuffObject = NULL;

					error = NO_ERROR;

					// 阻塞等待I/O事件
					rc = GetQueuedCompletionStatus( dev->mCompletionPort, &bytes, &key, &pol, INFINITE );

					// 收到关闭消息
					if ( key == NET_UDP_CLOSE )
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
						DYNAMIC_ASSERT(false);
						continue;
					}


					// 通过指针偏移得到自定义OVERLAPPED结构
					block = CONTAINING_RECORD( pol, IocpNetUdpBufBlock, mWSAOverlapped );
					if ( block->mType == NET_UDP_SEND )
					{
						sendBuffObject = CONTAINING_RECORD( block, IocpNetUdpSendDataBuffer, mIocpNetUdpBufBlock );
					}
                    else
					if (  block->mType == NET_UDP_RECV )
					{	
						recvBuffObject = CONTAINING_RECORD( block, IocpNetUdpRecvDataBuffer, mIocpNetUdpBufBlock );
					}else
					{
						DYNAMIC_ASSERT(false);
						continue;
					}


					// 检查完成端口状态失败後调用 WSAGetOverlappedResult 获得出错信息
					if ( rc == FALSE )
					{

						// Show error
						if (ERROR_OPERATION_ABORTED == GetLastError())
						{
							continue;
						}
						if ( block->mType == NET_UDP_RECV )
						{
							socket = dev->getLocalSocket(recvBuffObject->mSocketIndex)->socket;
						}
						else
						{
							socket =dev->getLocalSocket(sendBuffObject->mSocketIndex)->socket;
						}

						rc = WSAGetOverlappedResult( socket, &block->mWSAOverlapped, &bytes, FALSE, &flags );

						if ( rc == SOCKET_ERROR )
						{
							error = WSAGetLastError();
							{
								pass = 0;
								continue;
							}
						}
					}

                    if ( block->mType == NET_UDP_RECV )
                    {
                        if ( recvBuffObject )
                            dev->processRecvResponse( recvBuffObject, bytes, error );
                    }
                    else
					if ( block->mType == NET_UDP_SEND )
					{
                        if ( sendBuffObject )
						    dev->processSendResponse( sendBuffObject, bytes, error );
					}
				}
			}
		}
	}

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetUdpDriver::postSendRequest( IocpNetUdpSendDataBuffer* buffObject, U16 socketIndex )
	{
		WSABUF	wbuf;
		DWORD	bytes = 0;
		Int		ret = NO_ERROR;
		Int		err = SOCKET_ERROR;
		Bool    isOk = false;
		if (!buffObject)
		{
			isOk = false;
		}
		else
		{
			wbuf.buf = buffObject->mIocpNetUdpBufBlock.mBuf;
			wbuf.len = buffObject->mIocpNetUdpBufBlock.mUsedBufLen;

			DYNAMIC_ASSERT( wbuf.len > 0 );
			buffObject->mIocpNetUdpBufBlock.mType = NET_UDP_SEND;

			buffObject->addRefCount();

			sockaddr_in remoteAddr;
			remoteAddr.sin_addr.s_addr = NetAddress::host2Net(buffObject->addr.getIP());
			remoteAddr.sin_port		   = NetAddress::host2Net((U16)buffObject->addr.getPort());
			remoteAddr.sin_family      = AF_INET;

			buffObject->mSocketIndex = socketIndex;

			ret = WSASendTo(mLocalSockets[socketIndex].socket, &wbuf, 1,&bytes,0,(SOCKADDR*) &remoteAddr,sizeof(remoteAddr),&buffObject->mIocpNetUdpBufBlock.mWSAOverlapped,NULL);

			// 当没有立即发送成功时
			if ( ret == SOCKET_ERROR )
			{
				// 发送错误，应该释放该SOCKET对应的所有资源
				if ( ( err = WSAGetLastError() ) != WSA_IO_PENDING )
				{
					ret = SOCKET_ERROR;
					isOk = false;
				}else
					//表示异步投递已经成功，但是稍后发送才会完成
					//此时投递的缓冲区将会被锁定，等到TCP/IP层缓冲区有空余时才会被发送，然后会发送给IOCP一个完成消息
				{
					isOk = true;
				}
			}else
				// 投递立即成功，TCP/IP将数据从程序缓冲区中拷贝到TCP/IP层缓冲区中，然后不锁定该程序缓冲区。
			{
				isOk = true;
				if ( wbuf.len != bytes )
				{
					// 虽然 发送数据返回值不等于发送值,但是TCP/IP之后还是会发送该数据
					MG_LOG( out_info, "%s: wbuf.len != bytes ...\n", __MG_FUNC__ );
				}
			}

			if ( isOk == false )
			{
				// 递减发送标志位
				buffObject->subRefCount();
			}
		}

		//失败后判断移除连接对象
		if ( isOk == false )
		{
			IocpNetUdpConnectPtr& netConnect = getConnectContainerByUin(socketIndex,buffObject->addr.getUin());
			if ( mIsCloseWhenError )
			{
				if ( netConnect.isNull() == false )
				{
					passivityClose( netConnect->getID() );
				}
			}

			// 删除发送数据缓存对象
			if ( buffObject )
				destroyIocpNetUdpSendDataBuffer( buffObject );
		}
		return isOk;
	}

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetUdpDriver::postRecvRequest(IocpNetUdpRecvDataBuffer* buffObject,U16 socketIndex )
	{
		WSABUF	wbuf;
		DWORD	bytes = 0;
		DWORD	flat = 0;
		Int		ret = NO_ERROR;
		Int		err = SOCKET_ERROR;
		Bool    isOk = false;
		buffObject->mSocketIndex = socketIndex;
        if ( buffObject )
        {
            // 填充缓冲区描述结构
			// udp 不用处理粘包
			wbuf.buf = buffObject->mIocpNetUdpBufBlock.mBuf;
			wbuf.len = buffObject->mIocpNetUdpBufBlock.mBufferSize;

            DYNAMIC_ASSERT( wbuf.len > 0 );

            //设置接受标志
            buffObject->mIocpNetUdpBufBlock.mType = NET_UDP_RECV;

            //清空OVERLAPPED
            buffObject->mIocpNetUdpBufBlock.clearOVERLAPPED();
			
			buffObject->mSocketIndex = socketIndex;

            //投递一个接受请求
			//WSARecvFrom(RecvSocket,					       &DataBuf, 1, &BytesRecv, &Flags, (SOCKADDR *) &SenderAddr,             &SenderAddrSize,             &Overlapped,                                     NULL);
			ret =   WSARecvFrom(mLocalSockets[socketIndex].socket, &wbuf,    1, &bytes,     &flat,  (SOCKADDR*)  &buffObject->mRemoteAddr,&buffObject->mRemoteAddrSize,&buffObject->mIocpNetUdpBufBlock.mWSAOverlapped,NULL);
            // 当没有立即接受到TCP/IP中数据
            if ( ret == SOCKET_ERROR )
            {
                // 调用WSARecv错误，应该释放该SOCKET对应的所有资源
                if ( ( err = WSAGetLastError() ) != WSA_IO_PENDING )
                {
                    ret = SOCKET_ERROR;
                    isOk = false ;
                }else
				//TCP/IP层缓冲区中没有数据可取，IOCP将锁定投递的WSARecvFrom的buffer，直到TCP/IP层缓冲区中有新的数据到来通知IOCP
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

        //失败后移除连接对象
        if ( isOk == false )
        {
            if ( buffObject )
                destroyIocpNetUdpRecvDataBuffer( buffObject );
        }

		return isOk;
	}
	
    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::processSendResponse(IocpNetUdpSendDataBuffer* buffObject, I32 bytes, I32 error)
    {
        //释放发送数据缓存对象
        if ( buffObject )
        {
			IocpNetUdpSectionPacket* sectionPacket = buffObject->getIocpNetSectionPacket();
			if (bytes < IocpNetUdpSectionPacket_INFO::dataSize(*sectionPacket))
			{
				DYNAMIC_ASSERT(false);
			}
			//channel == 0 && type == 0表示连接
			//channel == 0 && type == 1表示断开连接
			if (sectionPacket->channel == 0)
			{
				if (sectionPacket->type == 0)
				{
					IocpNetUdpConnectPtr& netConnect = addConnectContainer(buffObject->addr,buffObject->mSocketIndex);
					mNetUdpManager->addUdpConnectEvent(mHandle,netConnect.getPointer()->getID(),sectionPacket->logicId,buffObject->addr);
					if (!mLocalSockets[buffObject->mSocketIndex].isBind)
					{
						postWaitRecv(buffObject->mSocketIndex);
					}
				}
				else if (sectionPacket->type == 1)
				{
					IocpNetUdpConnectPtr& netConnect = addConnectContainer(buffObject->addr,buffObject->mSocketIndex);
					mNetUdpManager->addUdpCloseEvent(mHandle,netConnect.getPointer()->getID(),sectionPacket->logicId,false,buffObject->addr);
					removeConnectContainer(netConnect->getID(),false);
				}
				
			}
            // 递减引用次数
            buffObject->subRefCount();

            // 删除该发送缓存对象，内部会判断引用次数
            destroyIocpNetUdpSendDataBuffer( buffObject );
        }
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::processRecvResponse(IocpNetUdpRecvDataBuffer* buffObject, I32 bytes, I32 error)
    {
        //error;
		
        Bool isOk = true;
		static NetAddress addr;
		
        if ( buffObject  )
        {
			IocpNetUdpSectionPacket* sectionPacket = (IocpNetUdpSectionPacket*)buffObject->mIocpNetUdpBufBlock.mBuf;
			if (bytes < IocpNetUdpSectionPacket_INFO::dataSize(*sectionPacket))
			{
				DYNAMIC_ASSERT(false);
				destroyIocpNetUdpRecvDataBuffer(buffObject);
				postRecvRequest( buffObject,buffObject->mSocketIndex);
				return;
			}

			addr = NetAddress(NetAddress::net2Host((U32)buffObject->mRemoteAddr.sin_addr.s_addr),
				NetAddress::net2Host((U16)buffObject->mRemoteAddr.sin_port));
			
			if (sectionPacket->channel == 0) //链接包
			{
				IocpNetUdpConnectPtr netConnect = addConnectContainer(addr,buffObject->mSocketIndex);
				if (netConnect.isNull())
				{
					DYNAMIC_ASSERT(false);
				}
				else
					mNetUdpManager->addUdpAcceptEvent(mHandle,netConnect.getPointer()->getID(),sectionPacket->logicId,addr);
			}
			else
			{
				IocpNetUdpConnectPtr netConnect = getConnectContainerByUin(buffObject->mSocketIndex,addr.getUin());
				if (netConnect.isNull())
				{
					DYNAMIC_ASSERT(false);
				}
				else
					mNetUdpManager->addUdpRecvEvent(mHandle,netConnect.getPointer()->getID(),sectionPacket->logicId,(IocpNetUdpSectionPacket*)buffObject->mIocpNetUdpBufBlock.mBuf);
				
			}


			buffObject->clear();
					
	            
			if ( isOk )
			{
				//重新发送一个请求
				postRecvRequest( buffObject,buffObject->mSocketIndex);
			}else
			{   

				if ( buffObject )
					destroyIocpNetUdpRecvDataBuffer( buffObject );
			}
		}

    }

	//-----------------------------------------------------------------------------
	Ptr Win32IocpNetUdpDriver::createIOCP()
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
	void Win32IocpNetUdpDriver::destroyIOCP()
	{
		if ( mCompletionPort != NULL )
		{
			CloseHandle( mCompletionPort );
			mCompletionPort = NULL;
		}
	}

	//-----------------------------------------------------------------------------
	Ptr Win32IocpNetUdpDriver::getIOCP()
	{
		return mCompletionPort;
	}
	
	//-----------------------------------------------------------------------------
	HANDLE Win32IocpNetUdpDriver::associateSocketWithIOCP(SOCKET socket,ULONG_PTR key)
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
	SOCKET Win32IocpNetUdpDriver::createUdpSocket()
	{
		SOCKET res = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (res == INVALID_SOCKET)
		{
			mErrorNo = WSAGetLastError();
		}
		return res;
	}

	//-----------------------------------------------------------------------------
	void Win32IocpNetUdpDriver::destroySocket(SOCKET& socket)
	{
		if ( socket )
		{
			closesocket(socket);
			socket = NULL;
		}
	}

    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::recordSendLog( IocpNetUdpConnect* netSocket, I32 bytes )
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
    void Win32IocpNetUdpDriver::recordRecvLog( IocpNetUdpConnect* netSocket, I32 bytes )
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

	//-----------------------------------------------------------------------------
	Bool Win32IocpNetUdpDriver::createIOCPStatusThreads(Int count)
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
			if ( mThreadHandles[i].create( handleIOCPStatusThread,this, "handleIOCPStatusThread") == false )
			{
				mErrorNo = GetLastError();
				return false;
			}
		}
		return true;
	}

	//-----------------------------------------------------------------------------
	void Win32IocpNetUdpDriver::destroyIOCPStatusThreads()
	{
        if ( mCompletionPort != NULL )
        {
            for (int i = 0; i < mThreadCount; i++ )
            {
                PostQueuedCompletionStatus( mCompletionPort, 0, DWORD(NET_UDP_CLOSE), 0 );
            }
        }

		for (int i = 0; i < mThreadCount; i++ )
		{
			mThreadHandles[i].destory();
		}
	}

    //-----------------------------------------------------------------------------
    IocpNetUdpConnectPtr Win32IocpNetUdpDriver::addConnectContainer(NetAddress& addr,U16 socketIndex)
    {
        IocpNetUdpConnectPtr   netConnect;
        IocpNetUdpConnect*     netConnectPoint = NULL;
        I32 id = 0;
		if (socketIndex >= mLocalSocketsCount)
		{
			DYNAMIC_ASSERT(false);
			return netConnect;
		}
        //连接容器加锁
        mIocpNetUdpConnectListCs.lock();
        {   
			std::map< U64,IocpNetUdpConnectPtr >& udpConnectList = mIocpNetUdpConnectListsbyUin[socketIndex];
			std::map< U64,IocpNetUdpConnectPtr >::iterator it = udpConnectList.find(addr.getUin());
			if (it != udpConnectList.end())
			{
				netConnect = it->second;
				mIocpNetUdpConnectListCs.unlock();
				MG_LOG(out_error,"---id--- %d\n",netConnect->getID());
				return netConnect;
			}

            //得到当前连接数量
            UInt checkCount = mIocpNetUdpConnectList.size();
            //遍历一定数量
            for( UInt i=0; i <= checkCount; i++ )
            {
                //得到当前递增编号
                //最大数量为 2 147 483 647，一般够用
                id = mIocpNetUdpConnectIndexGenerator++;
                //如果小于零则归零
                if ( mIocpNetUdpConnectIndexGenerator < 0 )
                {
                    mIocpNetUdpConnectIndexGenerator = 0;
                }

                //查找是否有重编号的连接，如果没有则创建新连接
                //TODO:有极小机率在递增编号归零后出错
                if ( mIocpNetUdpConnectList.find(id) == mIocpNetUdpConnectList.end() )
                {
                    //创建新的连接
                    netConnectPoint = createUdpConnectContainer( mIocpNetUdpConnectIndexGenerator );
                    //退出遍历检测
                    break;
                }
            }

            //保存连接容器
            if ( netConnectPoint )
            {
				netConnectPoint->mAddr = addr;
                netConnectPoint->setID( id );
                netConnect.bind(netConnectPoint);
                mIocpNetUdpConnectList[id] = netConnect;
				mIocpNetUdpConnectListsbyUin[socketIndex][addr.getUin()] = netConnect;
            }

        }
        //连接容器解锁
        mIocpNetUdpConnectListCs.unlock();
		MG_LOG(out_error,"id %d\n",netConnect->getID());
        return netConnect;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::removeConnectContainer(IocpNetUdpConnect* socketObject, Bool isException)
    {
        if ( socketObject )
        {
            removeConnectContainer( socketObject->getID(), isException );
        }
    }
    
    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::removeConnectContainer(I32 id,Bool isException)
    {
        mIocpNetUdpConnectListCs.lock();
        {
            std::map<I32,IocpNetUdpConnectPtr>::iterator iter = mIocpNetUdpConnectList.find(id);
            if ( iter != mIocpNetUdpConnectList.end() )
            {
                IocpNetUdpConnectPtr& netConnect = iter->second;
                if (  netConnect.isNull() == false )
                {
					//移除mIocpNetUdpConnectListsbyUin中连接
					std::map< U16, std::map< U64,IocpNetUdpConnectPtr > >::iterator its = mIocpNetUdpConnectListsbyUin.find(netConnect->mSocketIndex);
					if (its != mIocpNetUdpConnectListsbyUin.end())
					{
						std::map< U64,IocpNetUdpConnectPtr >::iterator it = its->second.find(netConnect->getAddress().getUin());
						if (it != its->second.end())
						{
							its->second.erase(it);
						}

					}
                    //判断是否有连接
                    if ( netConnect->isConnected() )
					{
						//没有通知远程连接关闭
						//IocpNetUdpSendDataBuffer* sendBuff = createIocpNetUdpSendDataBuffer(mNetUdpManager->getSectionPacketSize());
						//if (!sendBuff)
						//{

						//}
						//else
						//{
						//	IocpNetUdpSectionPacket*  sectionPacket = sendBuff->getIocpNetSectionPacket();
						//	
						//	// sectionPacket->channel = 0;&& sectionPacket->type = 1; 表示关闭
						//	sectionPacket->channel = 0;
						//	sectionPacket->type = 1;
						//	postSendRequest(sendBuff,mLocalSockets[netConnect->mSocketIndex].socket);
						//}

                        //增加一个关闭事件
                        mNetUdpManager->addUdpCloseEvent(mHandle,id,1,isException,netConnect->mAddr);

                        //递减连接数量
                        mCurrConnections--;
                        // 关闭连接
                        // iocp会接受到一个0字节的关闭消息
                        netConnect->diconnect();
                    }
					
                }
                //移除连接
				mIocpNetUdpConnectList.erase(iter);
            }
        }
        mIocpNetUdpConnectListCs.unlock();
    }

    //-----------------------------------------------------------------------------
    IocpNetUdpConnect* Win32IocpNetUdpDriver::createUdpConnectContainer( I32 id )
    {
        IocpNetUdpConnect* netConnect = NULL;
        //mIocpNetUdpConnectPool.threadLock();
        {
            netConnect = MG_POOL_NEW_ONCE_CONSTRUCTION(mIocpNetUdpConnectPool,IocpNetUdpConnect,(this));
            netConnect->clear();
            netConnect->setID( id );
        }
        //mIocpNetUdpConnectPool.threadUnLock();
        return netConnect;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::destroyUdpConnectContainer(IocpNetUdpConnect* socketObject)
    {
        mIocpNetUdpConnectPool.threadLock();
        {
            MG_POOL_DELETE_ONCE_DESTRUCTOR(mIocpNetUdpConnectPool,socketObject);
        }
        mIocpNetUdpConnectPool.threadUnLock();
    }

    //-----------------------------------------------------------------------------
    IocpNetUdpRecvDataBuffer* Win32IocpNetUdpDriver::createIocpNetUdpRecvDataBuffer(U16 sectionPacketSize)
    {
        IocpNetUdpRecvDataBuffer* buffObject = NULL;
        mIocpNetRecvBuffPoolCs.lock();
        {
            buffObject = MG_POOL_NEW_ONCE_CONSTRUCTION(mIocpNetUdpRecvBuffPool,IocpNetUdpRecvDataBuffer,(this, sectionPacketSize));
        }
        mIocpNetRecvBuffPoolCs.unlock();

        // 清空属性
        buffObject->clear();

        return buffObject;
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::destroyIocpNetUdpRecvDataBuffer(IocpNetUdpRecvDataBuffer* buffObject)
    {
        //buffObject->mIocpNetUdpConnectPtr.setNull();
        mIocpNetRecvBuffPoolCs.lock();
        {
            MG_POOL_DELETE_ONCE_DESTRUCTOR(mIocpNetUdpRecvBuffPool,buffObject);
        }
        mIocpNetRecvBuffPoolCs.unlock();
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::destroyAllIocpNetUdpRecvDataBuffer()
    {
        //监听也用到,所以暂时不能删除
    }

    //-----------------------------------------------------------------------------
    void Win32IocpNetUdpDriver::removeAllConnectContainers()
    {
        //加锁,遍历连接对象
        mIocpNetUdpConnectListCs.lock();
        {
            std::map<I32,IocpNetUdpConnectPtr>::iterator iter = mIocpNetUdpConnectList.begin();
            std::map<I32,IocpNetUdpConnectPtr>::iterator end_iter = mIocpNetUdpConnectList.end();
            for ( ;iter != end_iter; iter++ )
            {
                IocpNetUdpConnectPtr& netConnect = iter->second;
                if (  netConnect.isNull() == false )
                {
                    //判断是否有连接
                    if ( netConnect->isConnected() )
                    {
                        //增加一个关闭事件
                        //mNetUdpManager->addCloseEvent(mHandle, netConnect->getID(), true, netConnect->getAddress());
                        // 关闭连接
                        // iocp会接受到一个0字节的关闭消息
                        netConnect->diconnect();
                    }
                }
            }

            mIocpNetUdpConnectList.clear();
        }

        mIocpNetUdpConnectListCs.unlock();
        //////////////////////////////////////////////////////////////////////

        // 释放缓存对象
        destroyAllIocpNetUdpRecvDataBuffer();
        destroyAllIocpNetUdpSendDataBuffer();

        //设置连接数为零
        mCurrConnections = 0;
    }
    //-----------------------------------------------------------------------------
    IocpNetUdpConnectPtr	Win32IocpNetUdpDriver::getConnectContainer(I32 id)
    {
        IocpNetUdpConnectPtr netConnect;
        mIocpNetUdpConnectListCs.lock();
        {
            std::map<I32,IocpNetUdpConnectPtr>::iterator iter = mIocpNetUdpConnectList.find(id);
            if ( iter != mIocpNetUdpConnectList.end() )
            {
                netConnect = iter->second;
            }
        }
        mIocpNetUdpConnectListCs.unlock();
        return netConnect;
    }
	//-----------------------------------------------------------------------------
	IocpNetUdpConnectPtr Win32IocpNetUdpDriver::getConnectContainerByUin( U16 socketIndex,U64 uin)
	{
		IocpNetUdpConnectPtr netConnect;
		mIocpNetUdpConnectListCs.lock();
		{
			std::map< U16, std::map< U64,IocpNetUdpConnectPtr > >::iterator connectsIt = mIocpNetUdpConnectListsbyUin.find(socketIndex);
			if (connectsIt != mIocpNetUdpConnectListsbyUin.end())
			{
				std::map<U64,IocpNetUdpConnectPtr>::iterator iter = connectsIt->second.find(uin);
				if (iter != connectsIt->second.end())
				{
					netConnect = iter->second;
				}
			}
		}
		mIocpNetUdpConnectListCs.unlock();
		return netConnect;
	}
    //-----------------------------------------------------------------------------
    U64 Win32IocpNetUdpDriver::getAddressUin(I32 id)
    {
        IocpNetUdpConnectPtr netConnect = getConnectContainer(id);
        if (netConnect.isNull()==false)
        {
			U64 uin = netConnect->getAddress().getUin();
            return uin;
        }
        return 0;
    }

    //-----------------------------------------------------------------------------
    NetAddress Win32IocpNetUdpDriver::getAddress(I32 id)
    {
        NetAddress result;
        IocpNetUdpConnectPtr netConnect = getConnectContainer(id);
        if ( netConnect.isNull() == false )
        {
            result = netConnect->getAddress();
        }
        return result;
    }

    //-----------------------------------------------------------------------------
	Bool Win32IocpNetUdpDriver::isInitialized()
	{
		return mNetDriverInited;
	}

    //-----------------------------------------------------------------------------
	void Win32IocpNetUdpDriver::setHandle( I32 handle )
	{
		mHandle = handle;
	}
    //-----------------------------------------------------------------------------
	I32 Win32IocpNetUdpDriver::getHandle( )
	{
		return mHandle;
	}
    //-----------------------------------------------------------------------------
	NetConnectInfo* Win32IocpNetUdpDriver::getNetInfo()
	{
		return &mNetConnectInfo;
	}
    //-----------------------------------------------------------------------------
	NetConnectInfo* Win32IocpNetUdpDriver::getConnectInfo( I32 id )
	{
		 IocpNetUdpConnectPtr netConnect = getConnectContainer(id);
		 if (netConnect.isNull()==false)
		 {
			return &(netConnect->mConnectInfo);
		 }
		 return NULL;
	}
	//-----------------------------------------------------------------------------
	I32 Win32IocpNetUdpDriver::getCurrConnectionCount()
	{
		return mCurrConnections;
	}
	//-----------------------------------------------------------------------------
	MG::Bool Win32IocpNetUdpDriver::configNetLocalSockets(NetAddress* addr, I32 count)
	{
		if (mLocalSockets)
		{
			return false;
		}

		if (addr )
		{
			mLocalSockets = MG_NEW NetUdpLocalSocket[count];
			Bool error = false;
			for (I32 i = 0; i < count; i++)
			{
				mLocalSockets[i].addr		= addr[i];
				mLocalSockets[i].socket		= createUdpSocket();
				mLocalSockets[i].isBind		= true;
				if (mLocalSockets[i].socket == INVALID_SOCKET)
				{
					mErrorNo = WSAGetLastError();
					return false;
				}
				sockaddr_in localAddr;
				localAddr.sin_family = AF_INET;
				localAddr.sin_addr.s_addr = NetAddress::host2Net((U32)addr[i].getIP());
				localAddr.sin_port = NetAddress::host2Net((U16)addr[i].getPort());

				I32 ret = bind(mLocalSockets[i].socket, (SOCKADDR *) &localAddr, sizeof(localAddr));

				if (ret == SOCKET_ERROR)
				{
					mErrorNo = WSAGetLastError();
					return  false;
				}

				associateSocketWithIOCP(mLocalSockets[i].socket);

				postWaitRecv(i);
			}

			mLocalSocketsCount = count;
			
			return true;

		}
		else
		{
			mLocalSockets = MG_NEW NetUdpLocalSocket[count];
			for (I32 i = 0; i < count; i++)
			{
				mLocalSockets[i].socket = createUdpSocket();
				mLocalSockets[i].isBind = false;
				if (mLocalSockets[i].socket == INVALID_SOCKET)
				{
					mErrorNo = WSAGetLastError();
					return false;
				}
				associateSocketWithIOCP(mLocalSockets[i].socket);
			}
			mLocalSocketsCount = count;

			//没有绑定端口不能投递 接收
			return true;
		}
	}
	//-----------------------------------------------------------------------------
	void Win32IocpNetUdpDriver::postWaitRecv( U16 socketIndex )
	{
		for (I32 j = 0; j < mWaitAcceptSocketCount; j++)
		{
			postRecvRequest(createIocpNetUdpRecvDataBuffer(mNetUdpManager->getSectionPacketSize()),socketIndex);
		}
	}
	//-----------------------------------------------------------------------------
	void Win32IocpNetUdpDriver::connect( NetAddress* addr ,U16 socketIndex,U32 logicId )
	{
		if (socketIndex >= mLocalSocketsCount)
		{
			DYNAMIC_ASSERT(false);
			return;
		}
		IocpNetUdpConnectPtr connectPtr = addConnectContainer(*addr,socketIndex);

		IocpNetUdpSendDataBuffer* sendBuff = createIocpNetUdpSendDataBuffer(mNetUdpManager->getSectionPacketSize());
		sendBuff->addr = *addr;
		if (!sendBuff)
		{
			return;
		}
		IocpNetUdpSectionPacket*  sectionPacket = sendBuff->getIocpNetSectionPacket();
		sectionPacket->logicId = logicId;
		sectionPacket->channel = 0;
		sectionPacket->type = 0;
		sectionPacket->dataLen = 0;

		UInt realSendLen = IocpNetUdpSectionPacket_INFO::dataSize(*sectionPacket);
		if ( realSendLen <= sendBuff->mIocpNetUdpBufBlock.mBufferSize )
		{
			sendBuff->mIocpNetUdpBufBlock.mUsedBufLen = realSendLen;
		}else
		{
			DYNAMIC_ASSERT(0);
		}

		postSendRequest(sendBuff,socketIndex);
	}
	//-----------------------------------------------------------------------------
	LogicSendPacket* Win32IocpNetUdpDriver::getLogicSendPacket(Byte channel,Byte type,U16 sectionPacketSize,U32 logicId,U32 logicDataSize,Char8* data)
	{

		Char8 tempData[1024] ={0};
		U32 maxSectionPacketDataSize = sectionPacketSize - IocpNetUdpSectionPacket_INFO::headSize;
		U32 count = logicDataSize / maxSectionPacketDataSize + 1;
		U32	remainder =logicDataSize % maxSectionPacketDataSize;
		if ( count * sizeof(IocpNetUdpSendDataBuffer**) > 1024)
		{
			DYNAMIC_ASSERT(false);
			return NULL;
		}
		IocpNetUdpSendDataBuffer** sendBuffs = (IocpNetUdpSendDataBuffer**)tempData;
		for (U32 i = 0; i < count; i++)
		{
			
			sendBuffs[i] = createIocpNetUdpSendDataBuffer(sectionPacketSize);
			if (!sendBuffs[i])
			{
				for (U32 j = 0; j < i; j++)
				{
					sendBuffs[i]->destroy();
				}
				DYNAMIC_ASSERT(sendBuffs[i]);
				return NULL;
				
			}
			IocpNetUdpSectionPacket* sectionPacket = sendBuffs[i]->getIocpNetSectionPacket();
			if (i == count - 1)
			{
				sectionPacket->dataLen = remainder;
				memcpy(sectionPacket->data,data + i * maxSectionPacketDataSize,sectionPacket->dataLen);
			}
			else
			{
				sectionPacket->dataLen = maxSectionPacketDataSize;
				memcpy(sectionPacket->data,data + i * maxSectionPacketDataSize,sectionPacket->dataLen);
				
			}
			sectionPacket->logicId = logicId;

			sectionPacket->channel = channel;
			sectionPacket->type = type;
			
			sectionPacket->sectionCount = count;
			sectionPacket->sectionIndex = i;
				
		}
		LogicSendPacket* logicPacket = MG_NEW LogicSendPacket(sendBuffs,count,logicId);
		if (logicPacket == NULL)
		{
			DYNAMIC_ASSERT(false);
			return NULL;
		}
		return logicPacket;
	}
	//-----------------------------------------------------------------------------
	NetUdpLocalSocket* Win32IocpNetUdpDriver::getLocalSocket( U16 socketIndex )
	{
		return &mLocalSockets[socketIndex];
	}


}