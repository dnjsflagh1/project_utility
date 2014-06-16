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
    //Iocp����������
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

        // ��������, �����Ӷ��Զ�̵�ַ
		virtual I32				        connect(NetAddress* addr);	
        // �ر���������
		virtual I32				        close(I32 id);
        // �쳣�ر���������
                I32				        exceptClose(I32 id);
        // �ر�������������
		virtual void			        closeAll();
        // ��ʼ��������˿����� , ֻ�ܼ���һ������˿�
		virtual I32				        listen(NetAddress* addr);  

        // ��������
        virtual Bool                    send(NetOnceSendConfig* config);

        // �����ⲿ��
        virtual void                    setCode(I32 id, U64 code);

        //////////////////////////////////////////////////////////////////////////////////////
        
        virtual	U64 	                getAddressUin(I32 id);
        virtual	NetAddress              getAddress(I32 id);

		virtual	NetConnectInfo*         getNetInfo();
		virtual	NetConnectInfo*         getConnectInfo(I32 id);
		virtual	I32				        getCurrConnectionCount();

	protected:

        //////////////////////////////////////////////////////////////////////////////////////

		// �����¼������߳���ط���
		static void                     handleIOCPStatusThread(Ptr ptr); // IOCP�����¼��̴߳�����
		Bool					        createIOCPStatusThreads(Int count);
		void					        destroyIOCPStatusThreads();

        //////////////////////////////////////////////////////////////////////////////////////

        // ����ϵͳ����ʱ���
        void                            setSystemKeepAlive( SOCKET socket );

        //////////////////////////////////////////////////////////////////////////////////////

        // �������������Ч�Դ����߳���ط���
        static void                     handleCheckSocketValidThread(Ptr ptr); 

        // �̴߳���������
        Bool					        createCheckSocketValidThread();
        void					        destroyCheckSocketValidThread();

        // ���ͼ��������
        void                            sendCheckSocketValidHeartBeat( I32 netID );
        // ˢ����������������Чʱ��
        void                            refreshSocketValidTime( SOCKET socket );

        // ����һ������¼
        void                            addCheckSocketValidRecord( SOCKET socket, I32 netID, Bool isAccept );
        // ȥ��һ������¼
        void                            removeCheckSocketValidRecord( SOCKET socket );

        //////////////////////////////////////////////////////////////////////////////////////

		// ��ȫ�˿���ط���
		Ptr						        createIOCP(); // ������ȫ�˿�
		void					        destroyIOCP();
		Ptr						        getIOCP();
		HANDLE					        associateSocketWithIOCP(SOCKET socket,ULONG_PTR key=NULL);

        //////////////////////////////////////////////////////////////////////////////////////

		// Socket��ط���
		SOCKET					        createTcpSocket();
		void					        destroySocket(SOCKET& socket);
        Bool					        configSocketWhenConnected(SOCKET socket, Bool isAccept);
		Bool					        configAcceptSocket(SOCKET socket);
		
        //////////////////////////////////////////////////////////////////////////////////////

		// ��������������ط���
        NetIocpConnectPtr               addConnectContainer();
        // �Ƴ��������Ӷ��� // isException �Ƿ񱻶�
        void                            removeConnectContainer(I32 id,Bool isException,Bool isClearSocketRecord);
        // �Ƴ��������Ӷ��� // isException �Ƿ񱻶�
        void                            removeConnectContainer(NetIocpConnect* socketObject,  Bool isException,Bool isClearSocketRecord);
        // �Ƴ������������Ӷ���
		void					        removeAllConnectContainers();
        // �õ�һ���������Ӷ��� 
        NetIocpConnectPtr               getConnectContainer(I32 id);	

    public:

        //�������Ӷ���
        NetIocpConnect*                 createConnectContainer( I32 id );
        //�������Ӷ���
        void                            destroyConnectContainer(NetIocpConnect* socketObject);

        //�����������ݻ������
        NetIocpRecvDataBuffer*          createNetIocpRecvDataBuffer( NetIocpConnectPtr& netConnect );
        //���ٽ������ݻ������
        void                            destroyNetIocpRecvDataBuffer(NetIocpRecvDataBuffer* buffObject);
        //�������н������ݻ������
        void                            destroyAllNetIocpRecvDataBuffer();

	protected:

        //////////////////////////////////////////////////////////////////////////////////////

		//Ͷ�ݽ�����������
		Bool					        postAcceptRequest();
        //Ͷ�ݷ�����������
		Bool                            postSendRequest(NetIocpOnceSendConfig* iocpSendConfig);
        //Ͷ�ݽ�����������
		Bool                            postRecvRequest(NetIocpRecvDataBuffer* buffObject);
		
        //����������ӻ�Ӧ
		void					        processAcceptResponse(NetIocpRecvDataBuffer* buffObject, I32 bytes, I32 error);
        //���������ݻ�Ӧ
		void					        processSendResponse(NetIocpOnceSendConfig* configObject, I32 bytes, I32 error);
        //����������ݻ�Ӧ
        ///�յ�0�ֽڰ�ʱ��Ĭ����Ϊ��Ҫ�Ͽ�����
		void					        processRecvResponse(NetIocpRecvDataBuffer* buffObject, I32 bytes, I32 error);
        
        //////////////////////////////////////////////////////////////////////////////////////

        //��¼������־
        void                            recordSendLog( NetIocpConnect* netSocket, I32 bytes );
        //��¼������־
        void                            recordRecvLog( NetIocpConnect* netSocket, I32 bytes );

	private:

        //////////////////////////////////////////////////////////////////////////////////////

		Str         mName;                                      // ��������
		I32         mHandle;									// DriverΨһ���к�
        I32         mErrorNo;                                   // ������

		Bool        mNetDriverInited;                           // ��ʼ��״̬
		Bool        mNetDriverIniting;                          // ��ʼ����״̬
		Bool        mNetDriverListened;                         // ����״̬

        Bool        mIsCloseWhenError;                          // �Ƿ�رյ������쳣ʱ
		
		Int			mDriverType;								// driver����

        //////////////////////////////////////////////////////////////////////////////////////

		I32							        mCpuCount;									// ��ǰ�����е�CPU����								
		Ptr							        mCompletionPort;							// ��ɶ˿ھ��
		LPFN_ACCEPTEX				        mAcceptEx;									// AcceptEx ָ��
		LPFN_GETACCEPTEXSOCKADDRS	        mGetAcceptExSockaddrs;						// GetAcceptExSockaddrs ָ��


        //////////////////////////////////////////////////////////////////////////////////////

		OneThreadHandle                     mThreadHandles[MG_NET_MAX_THREADS];     	// ����߳̾��
		I32							        mStatuses[MG_NET_MAX_THREADS];			    // ����߳�״̬
		I32							        mThreadCount;								// ����̼߳���
		
        //////////////////////////////////////////////////////////////////////////////////////

        ObjectPool<NetIocpConnect,false>    mNetIocpConnectPool;                        // ���Ӷ����
        std::map<I32,NetIocpConnectPtr>     mNetIocpConnectList;                        // ���Ӷ����б�
        RWLocker					        mNetIocpConnectListCs;                      // ���Ӷ����б��ٽ���
        I32                                 mNetIocpConnectIndexGenerator;              // ���Ӷ������ID

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

        std::map<SOCKET,NetIocpConnectSocket>   mNetIocpConnectSocketMap;               // ���Ӷ���Socket�б�
        Critical					        mNetIocpConnectSocketMapCs;                 // ���Ӷ����б��ٽ���
        OneThreadHandle                     mCheckNetIocpConnectSocket;                 // ������������Ӿ��
        NetIocpSendDataBuffer*              mCheckNetIocpConnectSendHeartBeatPacket;    // �������Ͱ�

        //////////////////////////////////////////////////////////////////////////////////////

        ObjectPool<NetIocpRecvDataBuffer,false>   
                                            mNetIocpRecvBuffPool;                        // �������ݻ���ṹ��
        Critical					        mNetIocpRecvBuffPoolCs;                      // ���Ӷ����б��ٽ���
        
        UInt                                mSendNormalBufferSize;                       // �������ݻ����С
        UInt                                mSendBigBufferSize;                          // �������ݻ����С

		UInt								mSocketSendBufferSize;                       // �������ݻ����С
		UInt								mSocketRecvBufferSize;                       // �������ݻ����С

        UInt                                mRecvBufferSize;                             // �������ݻ����С

		I32							        mMaxConnections;                             // �����������
		InterLocked					        mCurrConnections;                            // ���ڵ���������

        //////////////////////////////////////////////////////////////////////////////////////

		SOCKET						    mListenSocket;                               // �������Ӷ˿�Socket
		Int							    mWaitAcceptSocketCount;                      // �ȴ����Ӷ˿�����   
		NetAddress					    mAddrs;	                                     // ���������ַ
		NetManager					    *mNetManager;                                // ������


        //////////////////////////////////////////////////////////////////////////////////////

		Bool						    mIsNetworkTimeoutValid;						 // �Ƿ����������糬ʱ����
        Int						        mNetworkTimeoutTouchTime;                     // ���糬ʱ����ʱ��
        Int						        mNetworkTimeoutDuration;                     // ���糬ʱ����ʱ��

        //////////////////////////////////////////////////////////////////////////////////////

        Critical                        mNetConnectInfoCs;                           // ������Ϣ�߳���
		NetConnectInfo				    mNetConnectInfo;                             // ������Ϣ
	};
}

#endif