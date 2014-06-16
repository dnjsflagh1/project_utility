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
	//UDP���������׽�������
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
    //Iocp����������
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
		//���ñ��ض˿���Ϣ
		Bool							configNetLocalSockets(NetAddress* addr = NULL,I32 count = 0);

		void							postWaitRecv(U16 socketIndex);
		//����Զ�̵�ַ
		void							connect(NetAddress* addr,U16 socketIndex,U32 logicId);
		// �ر���������
		virtual I32				        close(I32 id);
        // �����ر���������
		I32								passivityClose(I32 id);
        // �ر�������������
		virtual void			        closeAll();

        // �������ݻ������, IocpNetUdpSendDataBuffer����
        Bool                            send(IocpNetUdpSendDataBuffer* buffObject, I32 id, U16 socketIndex,U64 code = 0);
		Bool                            send(IocpNetUdpSendDataBuffer* buffObject, NetAddress& addr,U16 socketIndex);

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
		// ��ȫ�˿���ط���
		Ptr						        createIOCP(); // ������ȫ�˿�
		void					        destroyIOCP();
		Ptr						        getIOCP();
		HANDLE					        associateSocketWithIOCP(SOCKET socket,ULONG_PTR key=NULL);

        //////////////////////////////////////////////////////////////////////////////////////
		// Socket��ط���
		SOCKET					        createUdpSocket();
		void					        destroySocket(SOCKET& socket);
		NetUdpLocalSocket*				getLocalSocket(U16 socketIndex);
		U16								getLocalSocketCount();

        //////////////////////////////////////////////////////////////////////////////////////
		// ��������������ط���
        IocpNetUdpConnectPtr			addConnectContainer(NetAddress& addr,U16 socketIndex);
        // �Ƴ��������Ӷ���//��������ָ�����ü���
        void							removeConnectContainer(I32 id,Bool isException);
        // �Ƴ��������Ӷ���
        void							removeConnectContainer(IocpNetUdpConnect* socketObject,  Bool isException);
        // �Ƴ������������Ӷ���
		void							removeAllConnectContainers();
        // �õ�һ���������Ӷ��� 
        IocpNetUdpConnectPtr			getConnectContainer(I32 id);
		IocpNetUdpConnectPtr			getConnectContainerByUin(U16 socketIndex,U64 uin);	

    public:
        //�������Ӷ���
        IocpNetUdpConnect*				createUdpConnectContainer(I32 id );
        //�������Ӷ���
        void							destroyUdpConnectContainer(IocpNetUdpConnect* socketObject);
        //�����������ݻ������
        IocpNetUdpSendDataBuffer*		createIocpNetUdpSendDataBuffer( U32 buffSize  );
        //���ٷ������ݻ������
        void							destroyIocpNetUdpSendDataBuffer(IocpNetUdpSendDataBuffer* buffObject);
        //�������з������ݻ������
        void							destroyAllIocpNetUdpSendDataBuffer();
        //�����������ݻ������
        IocpNetUdpRecvDataBuffer*		createIocpNetUdpRecvDataBuffer(U16 sectionPacketSize);
        //���ٽ������ݻ������
        void							destroyIocpNetUdpRecvDataBuffer(IocpNetUdpRecvDataBuffer* buffObject);
        //�������н������ݻ������
        void							destroyAllIocpNetUdpRecvDataBuffer();

		//////////////////////////////////////////////////////////////////////////////////////
		//�����߼��������
		LogicSendPacket*				getLogicSendPacket(Byte channel,Byte type,U16 sectionPacketSize,U32 logicId,U32 logicDataSize,Char8* data);

	protected:
        //////////////////////////////////////////////////////////////////////////////////////
        //Ͷ�ݷ�����������
		Bool                            postSendRequest(IocpNetUdpSendDataBuffer* buffObject,U16 socketIndex );
        //Ͷ�ݽ�����������
		Bool                            postRecvRequest(IocpNetUdpRecvDataBuffer* buffObject,U16 socketIndex );
		
        //���������ݻ�Ӧ
		void					        processSendResponse(IocpNetUdpSendDataBuffer* buffObject, I32 bytes, I32 error);
        //����������ݻ�Ӧ
        ///�յ�0�ֽڰ�ʱ��Ĭ����Ϊ��Ҫ�Ͽ�����
		void					        processRecvResponse(IocpNetUdpRecvDataBuffer* buffObject, I32 bytes, I32 error);
        
        //////////////////////////////////////////////////////////////////////////////////////
        //��¼������־
        void                            recordSendLog( IocpNetUdpConnect* netSocket, I32 bytes );
        //��¼������־
        void                            recordRecvLog( IocpNetUdpConnect* netSocket, I32 bytes );

	private:

        //////////////////////////////////////////////////////////////////////////////////////
		Str										mName;                                      // ��������
		I32										mHandle;									// DriverΨһ���к�
        I32										mErrorNo;                                   // ������

		Bool									mNetDriverInited;                           // ��ʼ��״̬
		Bool									mNetDriverIniting;                          // ��ʼ����״̬
		Bool									mNetDriverListened;                         // ����״̬

        Bool									mIsCloseWhenError;                          // �Ƿ�رյ������쳣ʱ
	
        //////////////////////////////////////////////////////////////////////////////////////

		I32										mCpuCount;									// ��ǰ�����е�CPU����								
		Ptr										mCompletionPort;							// ��ɶ˿ھ��

        //////////////////////////////////////////////////////////////////////////////////////

		OneThreadHandle							mThreadHandles[MG_NET_MAX_THREADS];     	// ����߳̾��
		I32										mStatuses[MG_NET_MAX_THREADS];			    // ����߳�״̬
		I32										mThreadCount;								// ����̼߳���
		
    
        //////////////////////////////////////////////////////////////////////////////////////

        ObjectPool<IocpNetUdpConnect,false>     mIocpNetUdpConnectPool;                        // ���Ӷ����
		
		//key = netId value = connectObj
        std::map<I32,IocpNetUdpConnectPtr>		mIocpNetUdpConnectList;                        // ���Ӷ����б�
	
		//connectObjs <key = uin,value = connectObj>
		//key = socketIndex value = connectObjs								                   // ���Ӷ����б�
		std::map< U16, std::map< U64,IocpNetUdpConnectPtr > >
												mIocpNetUdpConnectListsbyUin;

		Critical								mIocpNetUdpConnectListCs;                      // ���Ӷ����б��ٽ���
        I32										mIocpNetUdpConnectIndexGenerator;              // ���Ӷ������ID

        std::map<U32, ObjectPool<IocpNetUdpSendDataBuffer, false>>
												mIocpNetUdpSendBuffPoolMap;						// �������ݻ���ṹ���б�
        Critical								mIocpNetSendBuffPoolMapCs;						// ���Ӷ����б��ٽ���

        ObjectPool<IocpNetUdpRecvDataBuffer, false>	
                                                mIocpNetUdpRecvBuffPool;                        // �������ݻ���ṹ��
        Critical								mIocpNetRecvBuffPoolCs;							// ���Ӷ����б��ٽ���

        UInt									mSendNormalBufferSize;							// �������ݻ����С
        UInt									mSendBigBufferSize;								// �������ݻ����С

        UInt									mRecvBufferSize;								// �������ݻ����С

		I32										mMaxConnections;								// �����������
		I32										mCurrConnections;								// ���ڵ���������

        //////////////////////////////////////////////////////////////////////////////////////							 
		NetUdpLocalSocket*						mLocalSockets;									// ������������
		Int										mLocalSocketsCount;

		NetUdpManager*							mNetUdpManager;									// ������

		Bool									mIsNetworkTimeoutValid;							// �Ƿ����������糬ʱ����
        Int										mWaitAcceptSocketCount;							// �ȴ����紦������ 
        
		//////////////////////////////////////////////////////////////////////////////////////
		NetConnectInfo							mNetConnectInfo;								// ������Ϣ

	};
}

#endif