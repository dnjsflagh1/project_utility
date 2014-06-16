/******************************************************************************/
#include "stdafx.h"
#include "Win32NetIocpData.h"
#include "RamAlloc.h"
#include "Win32NetIocpDriver.h"
/******************************************************************************/
namespace MG
{

    /******************************************************************************/
    //NetIocpConnect
    /******************************************************************************/
    NetIocpConnect::NetIocpConnect(Win32IocpNetDriver* owner)
        :mId(0)
        ,mSocket()
        ,mErrorNo(INVALID_SOCKET)
        ,mUserCode(0)
        ,mOwner(owner)
    {
    }

    //-----------------------------------------------------------------------------
    NetIocpConnect::~NetIocpConnect()
    {
        clear();
    }

    //-----------------------------------------------------------------------------
    void NetIocpConnect::lock()
    {
        
    }

    //-----------------------------------------------------------------------------
    void NetIocpConnect::unlock()
    {

    }

    //-----------------------------------------------------------------------------
    void NetIocpConnect::setCode(U64 code)
    {
        mUserCode = code;
    }

    //-----------------------------------------------------------------------------
    void NetIocpConnect::setID(I32 id)
    {
        mId = id;
    }

    //-----------------------------------------------------------------------------
    I32 NetIocpConnect::getID()
    {
        return mId;
    }

    //-----------------------------------------------------------------------------
    Bool NetIocpConnect::checkCode(U64 code)
    {
        Bool result = false;  
        result = ( mUserCode == code );
        return result;
    }

    //-----------------------------------------------------------------------------
    void NetIocpConnect::clear()
    {
        diconnect();
        mConnectInfo.clear();
        mUserCode = 0;
    }

    //-----------------------------------------------------------------------------
    Bool NetIocpConnect::isConnected()
    {
        return mSocket != INVALID_SOCKET;
    }

    //-----------------------------------------------------------------------------
    void NetIocpConnect::diconnect()
    {
        if (mSocket != INVALID_SOCKET)
        {
            closesocket(mSocket);
            mSocket = INVALID_SOCKET;
        }
    }

    /******************************************************************************/
    //NetIocpConnectPtr
    /******************************************************************************/
    void NetIocpConnectPtr::destroy(void)
    {
        if ( isNull() == false )
        {
            getPointer()->mOwner->destroyConnectContainer( getPointer() );
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////


    /******************************************************************************/
    //NetIocpOverlapped
    /******************************************************************************/
    void NetIocpOverlapped::clear()
    {
        mType = NET_IDLE;
        memset( &mWSAOverlapped, 0, sizeof(mWSAOverlapped) );
    }

    /******************************************************************************/
    //NetIocpBufBlock
    /******************************************************************************/
    NetIocpBufBlock::NetIocpBufBlock()
        :mUsedBufLen( 0 ),
        mBufValidPos( 0 ),
        mBuf( 0 ),
        mBufferSize( 0 )
    {
    }

    //-----------------------------------------------------------------------------
    NetIocpBufBlock::~NetIocpBufBlock()
    {
        releaseBuff();
    }

    //-----------------------------------------------------------------------------
    void NetIocpBufBlock::mallocBuff(UInt bufferSize)
    {
        if (mBuf)
        {
            DYNAMIC_ASSERT(  bufferSize <= mBufferSize );
        }else
        {
            mBufferSize     = bufferSize;
            Int byteSize    = mBufferSize * sizeof(Char8);
            // TODO: 使用内存池
            mBuf            = (Char8*)RamAlloc::getInstance().get(byteSize,true);
			Int k			= TestSharePtr::sendBuffCount++;
        }

        clear();
    }

    //-----------------------------------------------------------------------------
    void NetIocpBufBlock::releaseBuff()
    {
        if (mBuf)
        {
            RamAlloc::getInstance().release(mBuf);
            mBuf        = NULL;
            mBufferSize = 0;
        }
    }

    //-----------------------------------------------------------------------------
    void NetIocpBufBlock::clear()
    {
        // 不要清空缓冲区,因为可能存在大容量缓冲区,会影响效率
        //if (mBuf)
        //	memset( mBuf, 0, mBufferSize );
        mBufValidPos = 0;
        mUsedBufLen = 0;
    }
    
    /******************************************************************************/
    //NetIocpSendDataBuffer
    /******************************************************************************/

    NetIocpSendDataBuffer::NetIocpSendDataBuffer( Int bufferSize )
    {
        mNetIocpBufBlock.mallocBuff( bufferSize + NetIocpPacketData_INFO::headSize );
		mActive = false;
    }

    //-----------------------------------------------------------------------------
    NetIocpSendDataBuffer::~NetIocpSendDataBuffer()
    {
    }

	//-----------------------------------------------------------------------------
	Bool NetIocpSendDataBuffer::getActive()
	{
		return mActive;
	}

	//-----------------------------------------------------------------------------
	void NetIocpSendDataBuffer::setActive(Bool active)
	{
		mActive = active;
	}

    //-----------------------------------------------------------------------------
    Bool NetIocpSendDataBuffer::initialize()
    {
        clear();
        return true;
    }

    //-----------------------------------------------------------------------------
    void NetIocpSendDataBuffer::clear()
    {
        mNetIocpBufBlock.clear();
    }

    //-----------------------------------------------------------------------------
    NetIocpPacketData* NetIocpSendDataBuffer::getNetIocpPacketData()
    {
        return (NetIocpPacketData*)mNetIocpBufBlock.mBuf;
    }

    //-----------------------------------------------------------------------------
    U32 NetIocpSendDataBuffer::getNetSendBuffSize()
    {
        return mNetIocpBufBlock.mBufferSize;
    }

    //-----------------------------------------------------------------------------
    Char8* NetIocpSendDataBuffer::getLogicData()
    {
        NetIocpPacketData*  packetData = getNetIocpPacketData();
        return packetData->data;
    }

    //-----------------------------------------------------------------------------
    U32 NetIocpSendDataBuffer::getLogicDataMaxSize()
    {
        return mNetIocpBufBlock.mBufferSize - NetIocpPacketData_INFO::headSize;
    }

    //-----------------------------------------------------------------------------
    void NetIocpSendDataBuffer::setLogicDataSendSize(U32 size)
    {
        mNetIocpBufBlock.mUsedBufLen = NetIocpPacketData_INFO::headSize + size;

		if ( check() )
		{
			NetIocpPacketData* packetData = getNetIocpPacketData();
			{
				//自定义一个简单协议
				//包 = 数据标签 + 数据大小 + 数据内容
				//可扩展其他，如加密等
				packetData->tag     = MG_NET_PACKET_TAG;
				packetData->dataLen = (U16)size;
			}
		}
    }

    //-----------------------------------------------------------------------------
    U32 NetIocpSendDataBuffer::getLogicDataSendSize()
    {
		if ( mNetIocpBufBlock.mUsedBufLen <= NetIocpPacketData_INFO::headSize )
		{
			DYNAMIC_EEXCEPT_LOG( "send size is empty!" );
			DYNAMIC_ASSERT( false );
			return 0;
		}

        return mNetIocpBufBlock.mUsedBufLen - NetIocpPacketData_INFO::headSize;
    }

    //-----------------------------------------------------------------------------
    Bool NetIocpSendDataBuffer::check()
    {
        if ( mNetIocpBufBlock.mUsedBufLen > (Int)mNetIocpBufBlock.mBufferSize )
        {
			DYNAMIC_EEXCEPT_LOG( "NetIocpSendDataBuffer::check : exceed max size!" );
			DYNAMIC_ASSERT( false );
            return false;
        }
		
        return true;
    }

    /******************************************************************************/
    //NetIocpOnceSendConfig
    /******************************************************************************/

    //-----------------------------------------------------------------------------
    NetIocpOnceSendConfig::NetIocpOnceSendConfig()
    {
        mNetSendDataBufferList.reserve( MG_NET_MAX_COMBINE_PACKET_COUNT );
        mAllWbufSize = 0;
    }

    //-----------------------------------------------------------------------------
    Bool NetIocpOnceSendConfig::initialize(I32 driveID,I32 netID,I32 sendID)
    {
        NetOnceSendConfig::initialize(driveID,netID,sendID);
        mNetSendDataBufferList.clear();
        mOverlapped.clear();
        mOverlapped.mType = NET_SEND;
        mAllWbufSize = 0;
        return true;
    }

    //-----------------------------------------------------------------------------
    void NetIocpOnceSendConfig::pushSendData( NetSendDataBuffer* buff )
    {
        NetIocpSendDataBuffer* iocBuff = (NetIocpSendDataBuffer*)buff;
        
        mNetSendDataBufferList.push_back( iocBuff );
    }

    //-----------------------------------------------------------------------------
    Bool NetIocpOnceSendConfig::check()
    {
        UInt count = mNetSendDataBufferList.size();
        NetIocpSendDataBuffer* buffObject = NULL;
        for ( UInt i=0; i<count; i++ )
        {
            buffObject = mNetSendDataBufferList[i];

            if ( buffObject->check() == false )
                return false;
        }

        return true;
    }

    //-----------------------------------------------------------------------------
    WSABUF* NetIocpOnceSendConfig::getAndFillWsabuf()
    {
        UInt count = mNetSendDataBufferList.size();

		if ( count > MG_NET_MAX_COMBINE_PACKET_COUNT )
		{
			DYNAMIC_ASSERT_LOG(" send combine packet count exceed MG_NET_MAX_COMBINE_PACKET_COUNT ");
			DYNAMIC_ASSERT(false);
			count = MG_NET_MAX_COMBINE_PACKET_COUNT;
		}

        NetIocpSendDataBuffer* buffObject = NULL;
        mAllWbufSize = 0;
        for ( UInt i=0; i<count; i++ )
        {
            buffObject = mNetSendDataBufferList[i];

            // 填充缓冲区描述结构
            mWbuf[i].buf = buffObject->mNetIocpBufBlock.mBuf;
            mWbuf[i].len = buffObject->mNetIocpBufBlock.mUsedBufLen;
            mAllWbufSize += buffObject->mNetIocpBufBlock.mUsedBufLen;
        }

        return mWbuf;
    }

    //-----------------------------------------------------------------------------
    DWORD NetIocpOnceSendConfig::getWsabufCount()
    {
        return mNetSendDataBufferList.size();
    }

    //-----------------------------------------------------------------------------
    Int NetIocpOnceSendConfig::getAndSubWsabufBuffSize(Int size)
    {
        mAllWbufSize -= size;
        return mAllWbufSize;
    }

    /******************************************************************************/
    //NetIocpRecvDataBuffer
    /******************************************************************************/
    NetIocpRecvDataBuffer::NetIocpRecvDataBuffer(  Win32IocpNetDriver* owner, UInt bufferSize )
        :mOwner(owner)
    {
        mNetIocpBufBlock.mallocBuff( bufferSize );
    }

    //-----------------------------------------------------------------------------
    NetIocpRecvDataBuffer::~NetIocpRecvDataBuffer()
    {
        
    }

    //-----------------------------------------------------------------------------
    void NetIocpRecvDataBuffer::clear()
    {
        mNetIocpBufBlock.clear();
        mOverlapped.clear();
    }

    //-----------------------------------------------------------------------------
    Bool NetIocpRecvDataBuffer::destroy(void)
    {
        mOwner->destroyNetIocpRecvDataBuffer( this );
        return true;
    }
}
