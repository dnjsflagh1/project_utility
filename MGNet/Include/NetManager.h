/******************************************************************************/
#ifndef _NETMANAGERH_
#define _NETMANAGERH_
/******************************************************************************/
#include "NetDriver.h"
#include "NetDefine.h"
/******************************************************************************/

namespace MG
{
	class NetManager
	{
        friend class Win32IocpNetDriver;
	public:
		NetManager(){};
		virtual ~NetManager();

		virtual Bool            initialize();
		virtual I32             update();
		virtual I32             unInitialize();

	public:		

		I32                     createDriver(CChar* name, 
                                        I32 maxConnect, 
                                        Bool isSetNetworkTimeout, 
                                        Int networkTimeoutTouchTime,
                                        Int networkTimeoutDuration,
                                        Bool isCloseWhenError, 
                                        UInt sendNormalBuffSize,
                                        UInt sendBigBuffSize,
                                        UInt recvBuffSize,
										UInt socketSendBuffSize,
										UInt socketRecvBuffSize,
										Int dirverType);

	public:

		I32                     connect(I32 handle,NetAddress* addr);
		Bool                    lisiten(I32 handle,NetAddress* addr);  

        NetSendDataBuffer*      createNetSendDataBuffer(U32 buffSize);
        void                    destroyNetSendDataBuffer(NetSendDataBuffer* buff);
        NetOnceSendConfig*      createNetOnceSendConfig();
        void                    destroyNetOnceSendConfig(NetOnceSendConfig* config);
        Bool                    send(NetOnceSendConfig* config);

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

        virtual	void	        addAcceptEvent(I32 handle,I32 id,NetAddress&address)=0;
        virtual	void	        addConnectEvent(I32 handle,I32 id,NetAddress&address)=0;
        virtual	void	        addCloseEvent(I32 handle,I32 id,Bool isException,NetAddress&address)=0;
        virtual	void	        addRecvEvent(I32 handle,I32 id, Char8* buff, I32 buffLen)=0;
        virtual	void	        addErrEvent(I32 handle,I32 id)=0;
        virtual	void	        addSendEvent(NetOnceSendConfig* config)=0;

    private:

		NetDriver*		        getActiveDriver(I32 handle);

		std::vector<NetDriver*> mDrivers;
	};
}
#endif