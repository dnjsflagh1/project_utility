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
    // ���ݰ�ͷ��ǩ
    #define MG_NET_PACKET_TAG                       222
    

    ///////////////////////////////////////////////////////////////////////
    // ���糬ʱ���
    #define MG_NET_PACKET_TAG                       222

    // ��ʼ����������
    #define MG_NET_TIMEOUT_CHECK_HEARTBEAT          3000

    // ��ʼ�������ʱ��
    #define MG_NET_CLIENT_TIMEOUT_TOUCH_TIME        5000
    #define MG_NET_SERVER_TIMEOUT_TOUCH_TIME        1000*60*5

    // ��ʱ���ʱ��
    #define MG_NET_CLIENT_TIMEOUT_DURATION          30000
    #define MG_NET_SERVER_TIMEOUT_DURATION          600000
    
    ///////////////////////////////////////////////////////////////////////

    // Ĭ�Ϸ������ݻ����С
    #define MG_NET_DEFAULT_SEND_BUFFER_SIZE	        1024
    // Ĭ�Ͻ������ݻ���
	#define MG_NET_DEFAULT_RECV_BUFFER_SIZE	        1024*20

    //IOCP �����ӵ����ݻ�������С �ͻ���
    #define MG_NET_BUFFER_SIZE_CLIENT	            1024*20 //(20kb)//1000 Client == 20mb
    //IOCP �����ӵ����ݻ�������С �����
    #define MG_NET_BUFFER_SIZE_SERVER	            1024*20*1000 // 20mb
    //IOCP �����߳���
	#define MG_NET_MAX_THREADS 16
    //�������ķ���������
	#define MG_NET_MAX_SERVERS 32
    //�������Ŀͻ�������
	#define MG_NET_MAX_CLIENTS 10000

    ///////////////////////////////////////////////////////////////////////

    //�������ĺϲ����ݰ�����
    #define MG_NET_MAX_COMBINE_PACKET_COUNT         100

	///////////////////////////////////////////////////////////////////////

	//SOCKET��������С
	#define MG_NET_SOCKET_BUFF_SIZE					1024*32
	//server SOCKET���ͻ�������С
	#define MG_NET_MAX_SERVER_SOCKET_SENDBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE*10
	//server SOCKET���ջ�������С
	#define MG_NET_MAX_SERVER_SOCKET_RECVBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE*10
	//client SOCKET���ͻ�������С
	#define MG_NET_MAX_CLIENT_SOCKET_SENDBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE
	//client SOCKET���ջ�������С
	#define MG_NET_MAX_CLIENT_SOCKET_RECVBUFF_SIZE	MG_NET_SOCKET_BUFF_SIZE

    ///////////////////////////////////////////////////////////////////////
	//driver������
	#define DRIVER_TYPE_CLIENT  0
	#define DRIVER_TYPE_SERVER  1

	///////////////////////////////////////////////////////////////////////
	#define UDP_LOGIC_PACK_MIN_SIZE				512
	#define UDP_LOGIC_PACK_MAX_SIZE				1024 * 20 * 1000

	#define UDP_SECTION_PACKET_DEFAULT_SIZE		512
	#define UDP_SECTION_PACKET_MAX_SIZE			1024

    /******************************************************************************/
    //���緢�����ݻ���
    /******************************************************************************/
    class NetSendDataBuffer
    {
    public:

        NetSendDataBuffer(){};
        virtual ~NetSendDataBuffer(){};

        // ��ʼ��
        virtual Bool        initialize() = NULL;
        // �õ��߼�����ָ��
        virtual Char8*      getLogicData() = NULL;
        // �õ��߼������������
        virtual U32         getLogicDataMaxSize() = NULL;
        // �����߼����淢������
        virtual void        setLogicDataSendSize(U32 size) = NULL;
        // �õ��߼����淢������
        virtual U32         getLogicDataSendSize() = NULL;

		// �Ƿ񼤻� 
		virtual Bool		getActive() = NULL;
		// ���ü��� 
		virtual void		setActive(Bool active) = NULL;
    };  

    /******************************************************************************/
    //����ʵ��һ�η������Ӷ�������
    /******************************************************************************/
    class NetOnceSendConfig
    {
    public:

        // ��ʼ��
        virtual Bool    initialize(I32 driveID,I32 netID,I32 sendID){mDriverID=driveID;mNetID=netID;mSendID=sendID;mIsImmeSend=false;return true;};
        
        // ���ӷ�������
        virtual void    pushSendData( NetSendDataBuffer* buff ) = 0;

        // �õ������������
        I32             getDriverID(){ return mDriverID; };
        // �õ�������
        I32             getNetID(){ return mNetID; };
        // �õ�����˳����
        I32             getSendID(){ return mSendID; };
        // �Ƿ����̷��ͳ�ȥ��
        Bool            isImmeSend(){ return mIsImmeSend; };
        // �Ƿ��Ƿ����̷��ͳ�ȥ��
        void            setImmeSend(Bool isImme){ mIsImmeSend = isImme; };

		// �Ƿ񼤻� 
		virtual Bool	getActive(){ return mIsActive;}
		// ���ü��� 
		virtual void	setActive(Bool active){ mIsActive = active; }

    protected:
        
        Bool    mIsImmeSend;
		Bool    mIsActive;
        I32     mDriverID;
        I32     mNetID;
        I32     mSendID;
    };  


    /******************************************************************************/
    //����������ݻ���
    /******************************************************************************/
    class NetRecvDataBuffer
    {
    public:
        NetRecvDataBuffer(){};
        virtual ~NetRecvDataBuffer(){};
    }; 
       
}

#endif