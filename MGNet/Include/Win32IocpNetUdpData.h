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
	//Iocp网络包最底层数据结构
	/******************************************************************************/
#pragma pack (push)
#pragma pack (1)

	struct IocpNetUdpSectionPacket
	{
		U32     logicId;		//逻辑编号
		U16		sectionCount;	//分片数量
		U16		sectionIndex;		//分片编号
		U16     dataLen;		//数据长度
		Byte	channel;		//逻辑频道
		Byte	type;			//逻辑类型
		Char8   data[1];		//数据
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
	//网络接受和发送缓存数据结构定义
	//自定义WSAOVERLAPPED结构
	/******************************************************************************/
	struct IocpNetUdpBufBlock 
	{
	public:

		//网络缓存类型
		NET_EVENT_TYPE	    mType;
		//重叠结构
		WSAOVERLAPPED	    mWSAOverlapped;

		//缓存指针
		Char8*			    mBuf;
		//缓存大小
		UInt                mBufferSize;
		//缓存已用长度
		Int				    mUsedBufLen;
		//缓存当前可用位置
		Int				    mBufValidPos;

	public:

		IocpNetUdpBufBlock();
		virtual ~IocpNetUdpBufBlock();

		// 分配缓存
		void    mallocBuff(UInt bufferSize);
		// 释放缓存
		void    releaseBuff();

		//清空属性
		void    clear();
		//清空重叠数据结构
		void    clearOVERLAPPED();
	};

	
	/******************************************************************************/
	//网络发送数据缓存 实现 管理对象
	/******************************************************************************/
	class IocpNetUdpSendDataBuffer : public NetSendDataBuffer 
	{
	public:

		//IOCP缓存对象
		IocpNetUdpBufBlock		mIocpNetUdpBufBlock;
		//创造者
		Win32IocpNetUdpDriver*	mOwner;
		//线程锁
		Critical				mCs;
		//引用次数
		U16						mRefCount;
		
		NetAddress				addr;

		U16						mSocketIndex;
	public:

		IocpNetUdpSendDataBuffer(  Win32IocpNetUdpDriver* owner, UInt bufferSize );
		virtual ~IocpNetUdpSendDataBuffer();

		//清空属性
		void						clear();

		// 得到数据包
		IocpNetUdpSectionPacket*	getIocpNetSectionPacket();

		// 得到数据缓存
		virtual Char8*				getData();
		// 得到数据缓存大小
		virtual U32					getDataMaxSize();
		// 发送数据
		virtual void				send(I32 netId, U16 socketIndex,U64 code=0 );
		virtual void				send(NetAddress& addr,U16 socketIndex);

		// 得到总缓存大小
		U32							getTotalBuffSize();

		// 得到引用数量
		U16							getRefCount();
		// 递增引用数量
		void						addRefCount();
		// 递减引用数量
		void						subRefCount();
	public:
		// 绑定,用于智能指针
		virtual void				bind(void);
		// 回收,用于智能指针
		virtual Bool				recall(void);    
		// 销毁
		virtual Bool	            destroy(void);

	};

    /******************************************************************************/
    //网络接收数据缓存 实现
    /******************************************************************************/
    class IocpNetUdpRecvDataBuffer : public NetRecvDataBuffer 
    {
    public:

        //IOCP缓存对象
        IocpNetUdpBufBlock     mIocpNetUdpBufBlock;

        //创造者
        Win32IocpNetUdpDriver* mOwner;

		sockaddr_in				mRemoteAddr;	
		Int						mRemoteAddrSize;

		U16						mSocketIndex;
    public:
        IocpNetUdpRecvDataBuffer(  Win32IocpNetUdpDriver* owner, UInt bufferSize );
        virtual ~IocpNetUdpRecvDataBuffer();

        //清空属性
        void                clear();
        // 销毁
        virtual Bool        destroy(void);
    };
	/******************************************************************************/
	//逻辑包
	/******************************************************************************/
	class LogicSendPacket
	{
	public:

		LogicSendPacket(IocpNetUdpSendDataBuffer** buffObjects,U32 count,U32 logicPacketId);
		~LogicSendPacket();
		void send(I32 id,U16 socketIndex,U64 code);
		void send(NetAddress& addr,U16 socketIndex );
		// 为重发机制写的接口
		void sendByIndex(U32 index);
		void destory();

	private:
		std::vector<IocpNetUdpSendDataBuffer*>	mSendBuffObjects;
		U32										mLogicId;

	};
}

#endif