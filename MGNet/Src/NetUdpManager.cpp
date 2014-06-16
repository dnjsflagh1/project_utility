/******************************************************************************/
#include "stdafx.h"
#include "NetUdpManager.h"
#include "Win32IocpNetUdpDriver.h"
#include "NetSocket.h"
/******************************************************************************/
namespace MG
{
	//-----------------------------------------------------------------------------
	NetUdpManager::~NetUdpManager()
	{
		unInitialize();
	}

	//-----------------------------------------------------------------------------
	Bool NetUdpManager::initialize(U16 sectionPacketSize)
	{
		mCurrSectionPacketSize = sectionPacketSize;
		return true;
	}

	//-----------------------------------------------------------------------------
	I32 NetUdpManager::update()
	{
		I32 ret = 0;
		std::vector<NetUdpDriver*>::iterator iter = mDrivers.begin();
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
	I32 NetUdpManager::unInitialize()
	{
		I32 ret = 0;

		std::vector<NetUdpDriver*>::iterator iter = mDrivers.begin();
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
	I32 NetUdpManager::createDriver(
		CChar* name, 
		I32	   maxConnect, 
		Bool   isSetNetworkTimeout, 
		Bool   isCloseWhenError,
		//U16    sectionPacketSize,
		UInt   sendNormalBuffSize,
		UInt   sendBigBuffSize )
	{	
		I32 handle = mDrivers.size();	//此处需要测试
		NetUdpDriver* driver = MG_NEW Win32IocpNetUdpDriver;
		if ( driver )
		{
			driver->setHandle(handle);
			{
				NetUdpDriverInitParameter parameter ;
				parameter.name                  = name;
				parameter.maxConnections        = maxConnect;
				parameter.isSetNetworkTimeout   = isSetNetworkTimeout;
				parameter.sendNormalBuffSize    = sendNormalBuffSize;
				parameter.sendBigBuffSize       = sendBigBuffSize;
				parameter.isCloseWhenError      = isCloseWhenError;
				//parameter.sectionPacketSize		= sectionPacketSize;
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
	void NetUdpManager::connect(I32 handle,NetAddress* addr,U16 socketIndex)
	{
		mCurrLogicPacketId++;
		NetUdpDriver* driver = getActiveDriver( handle );
		if ( driver )
		{
			driver->connect(addr,socketIndex,mCurrLogicPacketId);
		}

	}

	//-----------------------------------------------------------------------------

	MG::Bool NetUdpManager::configLocalSockets( I32 handle,NetAddress* addr,I32 count )
	{
		NetUdpDriver* driver = getActiveDriver( handle );
		if ( driver )
		{
			if (driver->configNetLocalSockets(addr,count))
			{
				return true;
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	void NetUdpManager::setCode(I32 handle,I32 id, U64 code)
	{
		NetUdpDriver* driver = getActiveDriver( handle );
		if ( driver )
		{
			driver->setCode(id,code);
		}
	}

	//-----------------------------------------------------------------------------
	void NetUdpManager::close(I32 handle, I32 id )
	{
		NetUdpDriver* driver = getActiveDriver( handle );
		if (driver)
		{
			driver->close(id);
		}
	}

	//-----------------------------------------------------------------------------
	void NetUdpManager::closeAll(I32 handle)
	{
		NetUdpDriver* driver = getActiveDriver( handle );
		if (driver)
		{
			driver->closeAll();
		}
	}

	//-----------------------------------------------------------------------------
	void NetUdpManager::shutdown()
	{
		std::vector<NetUdpDriver*>::iterator iter = mDrivers.begin();
		for (; iter != mDrivers.end(); iter++)
		{
			if ((*iter)->isInitialized())
			{
				(*iter)->closeAll();
			}
		}
	}

	//-----------------------------------------------------------------------------
	NetUdpDriver* NetUdpManager::getActiveDriver(I32 handle)
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
	U64 NetUdpManager::getAddressUin( I32 handle, I32 id )
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
	NetAddress NetUdpManager::getAddress(I32 handle, I32 id)
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
	NetConnectInfo* NetUdpManager::getNetInfo( I32 handle )
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
	NetConnectInfo* NetUdpManager::getConnectInfo( I32 handle, I32 id )
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
	I32 NetUdpManager::getCurrConnectionCount( I32 handle )
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
	//-----------------------------------------------------------------------------
	LogicSendPacket* NetUdpManager::getLogicSendPacket( I32 handle,Byte channel,Byte type, U16 sectionPacketSize, U32 logicDataSize,Char8* data)
	{
		mCurrLogicPacketId++;
		if (handle >= 0 && handle < (I32)mDrivers.size())
		{
			return mDrivers[handle]->getLogicSendPacket(channel,type,sectionPacketSize,mCurrLogicPacketId,logicDataSize,data);
		}
		return NULL;
	}
	//-----------------------------------------------------------------------------
	MG::U16 NetUdpManager::getSectionPacketSize()
	{
		return mCurrSectionPacketSize;
	}
	//-----------------------------------------------------------------------------
}