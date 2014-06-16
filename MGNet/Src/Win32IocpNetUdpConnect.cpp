/******************************************************************************/
#include "stdafx.h"
#include "Win32IocpNetUdpConnect.h"
#include "RamAlloc.h"
#include "Win32IocpNetUdpDriver.h"
/******************************************************************************/
namespace MG
{

    /******************************************************************************/
    //IocpNetUdpConnect
    /******************************************************************************/
    IocpNetUdpConnect::IocpNetUdpConnect(Win32IocpNetUdpDriver* owner)
        :mId(0)
        ,mErrorNo(INVALID_SOCKET)
        ,mUserCode(0)
        ,mOwner(owner)
		,mIsConnect(false)
    {
    }

    //-----------------------------------------------------------------------------
    IocpNetUdpConnect::~IocpNetUdpConnect()
    {
        clear();
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpConnect::setCode(U64 code)
    {
        mUserCode = code;
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpConnect::setID(I32 id)
    {
        mId = id;
    }

    //-----------------------------------------------------------------------------
    I32 IocpNetUdpConnect::getID()
    {
        return mId;
    }

    //-----------------------------------------------------------------------------
    Bool IocpNetUdpConnect::checkCode(U64 code)
    {
        Bool result = false;  
        result = ( mUserCode == code );
        return result;
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpConnect::clear()
    {
        diconnect();
        mConnectInfo.clear();
        mUserCode = 0;
    }

    //-----------------------------------------------------------------------------
    Bool IocpNetUdpConnect::isConnected()
    {
		return mIsConnect;
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpConnect::diconnect()
    {
		mIsConnect = false;
    }
	//-----------------------------------------------------------------------------
	void IocpNetUdpConnect::setConnect()
	{
		mIsConnect = true;
	}
    /******************************************************************************/
    //IocpNetUdpConnectPtr
    /******************************************************************************/
    void IocpNetUdpConnectPtr::destroy(void)
    {
        if ( isNull() == false )
        {
            getPointer()->mOwner->destroyUdpConnectContainer( getPointer() );
        }
    }



}
