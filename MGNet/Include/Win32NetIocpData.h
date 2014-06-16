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
    //Socket������������
    //��������Socket�����������
    /******************************************************************************/
    class Win32IocpNetDriver;
    struct NetIocpConnect
    {
    public:

        //Ψһ����
        I32					mId;	
        //�ⲿ��������
        U64				    mUserCode ; 

        /////////////////////////////////////////////////

        //����Socket
        SOCKET				mSocket;
        //�����ַ
        NetAddress			mAddr;

        /////////////////////////////////////////////////

        //������
        I32					mErrorNo;	
        //������Ϣ
        Char				mErrorStr[256];	

        /////////////////////////////////////////////////

        //����������Ϣ
        NetConnectInfo		mConnectInfo;

        /////////////////////////////////////////////////

        //������
        Win32IocpNetDriver* mOwner;

    protected:

        //�߳���
        Critical			mCs;

    public:

        NetIocpConnect(Win32IocpNetDriver* owner);
        virtual ~NetIocpConnect();

        //�������
        void            clear();

        //�߳�����
        void            lock();
        //�߳̽���
        void            unlock();

        //�Ƿ�����������
        Bool			isConnected();
        //�ر���������
        void			diconnect();

        //���ñ��
        void            setID(I32 id);
        //�õ����
        I32             getID();
        //�õ������ַ
        NetAddress&     getAddress(){return mAddr;};

        //�����ⲿ�������
        void            setCode(U64 code);
        //����ⲿ�������
        Bool            checkCode(U64 code);

    };

    /******************************************************************************/
    //Socket������������ָ��
    //��NetIocpConnect�����ü������ã����˫�����ü���
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
    //Iocp�������ײ����ݽṹ
    /******************************************************************************/
#pragma pack (push)
#pragma pack (1)

    struct NetIocpPacketData
    {
        U8      tag;        //��ǩ
        U16     dataLen;    //���ݳ���
        Char8   data[1];    //����
    };

	struct NetIocpPacketData_INFO
	{
		static const Int headSize = sizeof (NetIocpPacketData) - sizeof(Char8);
	};

#pragma pack (pop)


    /******************************************************************************/
    //�Զ���WSAOVERLAPPED�ṹ
    /******************************************************************************/

    struct NetIocpOverlapped
    {
    public:
        //���绺������
        NET_EVENT_TYPE	    mType;
        //�ص��ṹ
        WSAOVERLAPPED	    mWSAOverlapped;

        //����ص����ݽṹ
        void                clear();
    };

    /******************************************************************************/
    //������ܺͷ��ͻ������ݽṹ����
    //�Զ���WSAOVERLAPPED�ṹ
    /******************************************************************************/
    struct NetIocpBufBlock 
    {
    public:

        //����ָ��
        Char8*			    mBuf;
        //�����С
        UInt                mBufferSize;

        //�������ó���
        Int				    mUsedBufLen;
        //���浱ǰ����λ��
        Int				    mBufValidPos;

    public:

        NetIocpBufBlock();
        virtual ~NetIocpBufBlock();

        // ���仺��
        void        mallocBuff(UInt bufferSize);
        // �ͷŻ���
        void        releaseBuff();

        //�������
        void        clear();

    };


    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////

    /******************************************************************************/
    //���緢�����ݻ���������
    /******************************************************************************/
    class NetIocpSendDataBuffer : public NetSendDataBuffer 
    {
    public:

        // IOCP�������
        NetIocpBufBlock     mNetIocpBufBlock;

		Bool				mActive;

    public:

        NetIocpSendDataBuffer( Int bufferSize );
        virtual ~NetIocpSendDataBuffer();

        // ��ʼ��
        virtual Bool            initialize();
        // �������
        void                    clear();

        // ���
        Bool                    check();

        // �õ�ʵ�����ݰ�
        NetIocpPacketData*      getNetIocpPacketData();
        // �õ�ʵ�ʷ��ͻ����С
        U32                     getNetSendBuffSize();

        // �õ��߼�����ָ��
        virtual Char8*          getLogicData();
        // �õ��߼������������
        virtual U32             getLogicDataMaxSize();
        // �����߼����淢������
        virtual void            setLogicDataSendSize(U32 size);
        // �õ��߼����淢������
        virtual U32             getLogicDataSendSize();

		// �Ƿ񼤻� 
		virtual Bool			getActive();
		// ���ü��� 
		virtual void			setActive(Bool active) ;
    };

    /******************************************************************************/
    //����ʵ�ʷ����������ݻ���������
    /******************************************************************************/
    class NetIocpOnceSendConfig  : public NetOnceSendConfig 
    {
        friend class Win32IocpNetDriver;
    public:

        NetIocpOnceSendConfig();

        // IOCP Overlapped
        NetIocpOverlapped   mOverlapped;

        // ��ʼ��
        virtual Bool    initialize(I32 driveID,I32 netID,I32 sendID);
        // ���
        Bool            check();

        // ���ӷ�������
        virtual void    pushSendData( NetSendDataBuffer* buff );

        // ���WSABUF
        WSABUF*         getAndFillWsabuf();
        // ���WSABUF
        DWORD           getWsabufCount();
        // ����WSABUF
        Int             getAndSubWsabufBuffSize(Int size);

    protected:

        WSABUF	                                mWbuf[MG_NET_MAX_COMBINE_PACKET_COUNT];
        std::vector<NetIocpSendDataBuffer*>     mNetSendDataBufferList;
        Int                                     mAllWbufSize;

    };


    /******************************************************************************/
    //����������ݻ���������
    /******************************************************************************/
    class NetIocpRecvDataBuffer : public NetRecvDataBuffer 
    {
    public:

        // IOCP Overlapped
        NetIocpOverlapped   mOverlapped;

        //IOCP�������
        NetIocpBufBlock     mNetIocpBufBlock;

        //������
        Win32IocpNetDriver* mOwner;

        //������������ָ��
        NetIocpConnectPtr   mNetIocpConnectPtr;

    public:
        NetIocpRecvDataBuffer(  Win32IocpNetDriver* owner, UInt bufferSize );
        virtual ~NetIocpRecvDataBuffer();

        //�������
        void                clear();
        // ����
        virtual Bool        destroy(void);
    };
}

#endif