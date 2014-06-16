/******************************************************************************/
#include "stdafx.h"
#include "NetManager.h"
#include "Win32NetIocpDriver.h"
#include "NetSocket.h"
#include "SharedPtr.h"
/******************************************************************************/
namespace MG
{
	

	//-----------------------------------------------------------------------------
	NetManager::~NetManager()
	{
		
	}

    //-----------------------------------------------------------------------------
	Bool NetManager::initialize()
	{
		return true;
	}

    //-----------------------------------------------------------------------------
	I32 NetManager::update()
	{
		I32 ret = 0;
		std::vector<NetDriver*>::iterator iter = mDrivers.begin();
		for (; iter != mDrivers.end(); iter++)
		{
			if ((*iter) && (*iter)->isInitialized())
			{
				ret += (*iter)->update();
			}
		}
		return ret;
	}

    //-----------------------------------------------------------------------------
	I32 NetManager::unInitialize()
	{
		I32 ret = 0;

		std::vector<NetDriver*>::iterator iter = mDrivers.begin();
		for (; iter != mDrivers.end(); iter++)
		{
			if ((*iter)->isInitialized())
			{
				ret += (*iter)->unInitialize();
				MG_SAFE_DELETE(*iter);
			}
		}
		mDrivers.clear();
		return ret;
	}

    //-----------------------------------------------------------------------------
I32 NetManager::createDriver(	CChar* name, 
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
								Int dirverType)
	{	
		I32 handle = mDrivers.size();	//此处需要测试
		NetDriver* driver = MG_NEW Win32IocpNetDriver;
		if ( driver )
		{
			driver->setHandle(handle);
			{
				NetDriverInitParameter parameter ;
				parameter.name                      = name;
				parameter.maxConnections            = maxConnect;
				parameter.isSetNetworkTimeout       = isSetNetworkTimeout;
				parameter.networkTimeoutTouchTime   = networkTimeoutTouchTime;
                parameter.networkTimeoutDuration    = networkTimeoutDuration;

                parameter.sendNormalBuffSize        = sendNormalBuffSize;
                parameter.sendBigBuffSize           = sendBigBuffSize;
                parameter.recvBuffSize              = recvBuffSize;
                parameter.isCloseWhenError          = isCloseWhenError;
				parameter.socketSendBuffSize	    = socketSendBuffSize;
				parameter.socketRecvBuffSize	    = socketRecvBuffSize;
				parameter.driverType			    = dirverType;

				if (!driver->initialize(parameter,this))
				{
					driver->closeAll();
					MG_DELETE (driver);
					return -1;
				}
			}
			mDrivers.push_back(driver);
			return driver->getHandle();
		}
		return -1;
	}

	//-----------------------------------------------------------------------------
	I32 NetManager::connect(I32 handle,NetAddress* addr)
	{
		I32 res = -1;
		NetDriver* driver = getActiveDriver( handle );
		if ( driver )
		{
			res = driver->connect(addr);
		}
		return res;
	}

	//-----------------------------------------------------------------------------
	Bool NetManager::lisiten(I32 handle,NetAddress* addr)
	{
		NetDriver* driver = getActiveDriver( handle );
		if ( driver )
		{
			if (driver->listen(addr) != -1)
			{
				return true;
			}
		}
		return false;
	}

    //-----------------------------------------------------------------------------
    NetSendDataBuffer* NetManager::createNetSendDataBuffer(U32 buffSize)
    {
        NetIocpSendDataBuffer* buff = MG_NEW NetIocpSendDataBuffer(buffSize);
        return buff;
    }

    //-----------------------------------------------------------------------------
    void NetManager::destroyNetSendDataBuffer(NetSendDataBuffer* buff)
    {
        MG_SAFE_DELETE(buff);
    }

    //-----------------------------------------------------------------------------
    NetOnceSendConfig* NetManager::createNetOnceSendConfig()
    {
        NetIocpOnceSendConfig* config = MG_NEW NetIocpOnceSendConfig();
		TestSharePtr::sendConfigCount++;
        return config;
    }

    //-----------------------------------------------------------------------------
    void NetManager::destroyNetOnceSendConfig(NetOnceSendConfig* config)
    {
		TestSharePtr::sendConfigCount--;
        MG_SAFE_DELETE(config);
    }

    //-----------------------------------------------------------------------------
    Bool NetManager::send(NetOnceSendConfig* config)
    {
        Bool result = false;
        NetDriver* driver = getActiveDriver( config->getDriverID() );
        if ( driver )
        {
            result = driver->send(config);
        }else
        {
            DYNAMIC_ASSERT_LOG(false,"NetManager::send : not find driver!");
        }
        return result;
    }

    //-----------------------------------------------------------------------------
    void NetManager::setCode(I32 handle,I32 id, U64 code)
    {
        NetDriver* driver = getActiveDriver( handle );
        if ( driver )
        {
            driver->setCode(id,code);
        }
    }
 
    //-----------------------------------------------------------------------------
	void NetManager::close(I32 handle, I32 id )
	{
		NetDriver* driver = getActiveDriver( handle );
		if (driver)
		{
			driver->close(id);
		}
	}

    //-----------------------------------------------------------------------------
    void NetManager::closeAll(I32 handle)
    {
        NetDriver* driver = getActiveDriver( handle );
        if (driver)
        {
            driver->closeAll();
        }
    }

    //-----------------------------------------------------------------------------
	void NetManager::shutdown()
	{
		std::vector<NetDriver*>::iterator iter = mDrivers.begin();
		for (; iter != mDrivers.end(); iter++)
		{
			if ((*iter)->isInitialized())
			{
				(*iter)->closeAll();
			}
		}
	}

	//-----------------------------------------------------------------------------
	NetDriver* NetManager::getActiveDriver(I32 handle)
	{
		if (handle >= 0 && handle < (I32)mDrivers.size())
		{
			if (mDrivers[handle]->isInitialized())
			{
				return mDrivers[handle];
			}
		}
		return NULL;
	}
	//-----------------------------------------------------------------------------
	U64 NetManager::getAddressUin( I32 handle, I32 id )
	{
		if (handle >= 0 && handle < (I32)mDrivers.size())
		{
			if (mDrivers[handle]->isInitialized())
			{
				return mDrivers[handle]->getAddressUin(id);
			}
		}
		return 0;
	}

    //-----------------------------------------------------------------------------
    NetAddress NetManager::getAddress(I32 handle, I32 id)
    {
        if (handle >= 0 && handle < (I32)mDrivers.size())
        {
            if (mDrivers[handle]->isInitialized())
            {
                return mDrivers[handle]->getAddress(id);
            }
        }
        return NetAddress();
    }

    //-----------------------------------------------------------------------------
	NetConnectInfo* NetManager::getNetInfo( I32 handle )
	{
		if (handle >= 0 && handle < (I32)mDrivers.size())
		{
			if (mDrivers[handle]->isInitialized())
			{
				return mDrivers[handle]->getNetInfo();
			}
		}
		return NULL;
	}

    //-----------------------------------------------------------------------------
	NetConnectInfo* NetManager::getConnectInfo( I32 handle, I32 id )
	{
		if (handle >= 0 && handle < (I32)mDrivers.size())
		{
			if (mDrivers[handle]->isInitialized())
			{
				return mDrivers[handle]->getConnectInfo(id);
			}
		}
		return NULL;
	}

    //-----------------------------------------------------------------------------
	I32 NetManager::getCurrConnectionCount( I32 handle )
	{
		if (handle >= 0 && handle < (I32)mDrivers.size())
		{
			if (mDrivers[handle]->isInitialized())
			{
				return mDrivers[handle]->getCurrConnectionCount();
			}
		}
		return 0;
	}


}