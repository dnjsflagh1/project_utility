/******************************************************************************/
#ifndef _WIN32NETUDPIOCPDATA_H_
#define _WIN32NETUDPIOCPDATA_H_
/******************************************************************************/
#include "NetDefine.h"
#include "NetAddress.h"
#include "Lock.h"
#include "SharedPtr.h"
#include "NetSocket.h"
#include "Win32IocpNetUdpConnect.h"
#include <WinSock2.h>
/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//Iocp�������ײ����ݽṹ
	/******************************************************************************/
#pragma pack (push)
#pragma pack (1)

	struct IocpNetUdpSectionPacket
	{
		U32     logicId;		//�߼����
		U16		sectionCount;	//��Ƭ����
		U16		sectionIndex;		//��Ƭ���
		U16     dataLen;		//���ݳ���
		Byte	channel;		//�߼�Ƶ��
		Byte	type;			//�߼�����
		Char8   data[1];		//����
	};

	struct IocpNetUdpSectionPacket_INFO
	{
		static const Int headSize = sizeof(IocpNetUdpSectionPacket) - sizeof(Char8);
		static const Int dataSize(IocpNetUdpSectionPacket& packet)
		{
			return headSize + packet.dataLen;
		}
	};

#pragma pack (pop)


	/******************************************************************************/
	//������ܺͷ��ͻ������ݽṹ����
	//�Զ���WSAOVERLAPPED�ṹ
	/******************************************************************************/
	struct IocpNetUdpBufBlock 
	{
	public:

		//���绺������
		NET_EVENT_TYPE	    mType;
		//�ص��ṹ
		WSAOVERLAPPED	    mWSAOverlapped;

		//����ָ��
		Char8*			    mBuf;
		//�����С
		UInt                mBufferSize;
		//�������ó���
		Int				    mUsedBufLen;
		//���浱ǰ����λ��
		Int				    mBufValidPos;

	public:

		IocpNetUdpBufBlock();
		virtual ~IocpNetUdpBufBlock();

		// ���仺��
		void    mallocBuff(UInt bufferSize);
		// �ͷŻ���
		void    releaseBuff();

		//�������
		void    clear();
		//����ص����ݽṹ
		void    clearOVERLAPPED();
	};

	
	/******************************************************************************/
	//���緢�����ݻ��� ʵ�� �������
	/******************************************************************************/
	class IocpNetUdpSendDataBuffer : public NetSendDataBuffer 
	{
	public:

		//IOCP�������
		IocpNetUdpBufBlock		mIocpNetUdpBufBlock;
		//������
		Win32IocpNetUdpDriver*	mOwner;
		//�߳���
		Critical				mCs;
		//���ô���
		U16						mRefCount;
		
		NetAddress				addr;

		U16						mSocketIndex;
	public:

		IocpNetUdpSendDataBuffer(  Win32IocpNetUdpDriver* owner, UInt bufferSize );
		virtual ~IocpNetUdpSendDataBuffer();

		//�������
		void						clear();

		// �õ����ݰ�
		IocpNetUdpSectionPacket*	getIocpNetSectionPacket();

		// �õ����ݻ���
		virtual Char8*				getData();
		// �õ����ݻ����С
		virtual U32					getDataMaxSize();
		// ��������
		virtual void				send(I32 netId, U16 socketIndex,U64 code=0 );
		virtual void				send(NetAddress& addr,U16 socketIndex);

		// �õ��ܻ����С
		U32							getTotalBuffSize();

		// �õ���������
		U16							getRefCount();
		// ������������
		void						addRefCount();
		// �ݼ���������
		void						subRefCount();
	public:
		// ��,��������ָ��
		virtual void				bind(void);
		// ����,��������ָ��
		virtual Bool				recall(void);    
		// ����
		virtual Bool	            destroy(void);

	};

    /******************************************************************************/
    //����������ݻ��� ʵ��
    /******************************************************************************/
    class IocpNetUdpRecvDataBuffer : public NetRecvDataBuffer 
    {
    public:

        //IOCP�������
        IocpNetUdpBufBlock     mIocpNetUdpBufBlock;

        //������
        Win32IocpNetUdpDriver* mOwner;

		sockaddr_in				mRemoteAddr;	
		Int						mRemoteAddrSize;

		U16						mSocketIndex;
    public:
        IocpNetUdpRecvDataBuffer(  Win32IocpNetUdpDriver* owner, UInt bufferSize );
        virtual ~IocpNetUdpRecvDataBuffer();

        //�������
        void                clear();
        // ����
        virtual Bool        destroy(void);
    };
	/******************************************************************************/
	//�߼���
	/******************************************************************************/
	class LogicSendPacket
	{
	public:

		LogicSendPacket(IocpNetUdpSendDataBuffer** buffObjects,U32 count,U32 logicPacketId);
		~LogicSendPacket();
		void send(I32 id,U16 socketIndex,U64 code);
		void send(NetAddress& addr,U16 socketIndex );
		// Ϊ�ط�����д�Ľӿ�
		void sendByIndex(U32 index);
		void destory();

	private:
		std::vector<IocpNetUdpSendDataBuffer*>	mSendBuffObjects;
		U32										mLogicId;

	};
}

#endif