/******************************************************************************/
#ifndef _NETUDPMANAGERH_
#define _NETUDPMANAGERH_
/******************************************************************************/
#include "NetUdpDriver.h"
#include "NetDefine.h"
/******************************************************************************/
namespace MG
{
	struct IocpNetUdpSectionPacket;
	class NetUdpManager
	{
		friend class Win32IocpNetUdpDriver;
	public:
		NetUdpManager(){mCurrLogicPacketId = 1;};
		virtual ~NetUdpManager();

		virtual Bool            initialize(U16 sectionPacketSize);
		virtual I32             update();
		virtual I32             unInitialize();

	public:		

		I32                     createDriver(
											CChar* name, 
											I32	   maxConnect, 
											Bool   isSetNetworkTimeout, 
											Bool   isCloseWhenError,
											//U16    sectionPacketSize,
											UInt   sendNormalBuffSize,
											UInt   sendBigBuffSize);

	public:

		void                    connect(I32 handle,NetAddress* addr,U16 socketIndex);
		Bool                    configLocalSockets(I32 handle,NetAddress* addr,I32 count);   
		LogicSendPacket*		getLogicSendPacket(I32 handle, Byte channel,Byte type, U16 sectionPacketSize, U32 logicDataSize,Char8* data);
		U16						getSectionPacketSize();
		void                    close(I32 handle,I32 id);
		void                    closeAll(I32 handle);
		void                    shutdown();

	public:
		void                    setCode(I32 handle,I32 id, U64 code);
		U64				        getAddressUin(I32 handle, I32 id);
		NetAddress              getAddress(I32 handle, I32 id);

		NetConnectInfo*         getNetInfo(I32 handle);
		NetConnectInfo*         getConnectInfo(I32 handle, I32 id);

		I32				        getCurrConnectionCount(I32 handle);

	public:
		virtual	void	    addUdpAcceptEvent(I32 handle,I32 id,U32 logicId,NetAddress&address) = 0;
		virtual	void	    addUdpConnectEvent(I32 handle,I32 id,U32 logicId,NetAddress&address) = 0;
		virtual	void	    addUdpCloseEvent(I32 handle,I32 id,U32 logicId,Bool isException,NetAddress&address) = 0;
		virtual	void	    addUdpRecvEvent(I32 handle,I32 id,U32 logicId,IocpNetUdpSectionPacket* packet) = 0;
		virtual	void	    addUdpErrEvent(I32 handle,I32 id,U32 logicId) = 0;
	private:
		NetUdpDriver*		    getActiveDriver(I32 handle);

		std::vector<NetUdpDriver*> mDrivers;
		U16						mCurrSectionPacketSize;
		U32						mCurrLogicPacketId;
	};
}
#endif