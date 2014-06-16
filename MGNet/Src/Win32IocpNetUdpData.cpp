/******************************************************************************/
#include "stdafx.h"
#include "Win32IocpNetUdpData.h"
#include "RamAlloc.h"
#include "Win32IocpNetUdpDriver.h"
/******************************************************************************/
namespace MG
{

    /******************************************************************************/
    //IocpNetUdpBufBlock
    /******************************************************************************/
    IocpNetUdpBufBlock::IocpNetUdpBufBlock()
        :mType( NET_IDLE ),
        mUsedBufLen( 0 ),
        mBufValidPos( 0 ),
        mBuf( 0 ),
        mBufferSize( 0 )
    {
    }

    //-----------------------------------------------------------------------------
    IocpNetUdpBufBlock::~IocpNetUdpBufBlock()
    {
        releaseBuff();
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpBufBlock::mallocBuff(UInt bufferSize)
    {
        if (mBuf)
        {
            DYNAMIC_ASSERT(  bufferSize <= mBufferSize );
        }else
        {
            mBufferSize = bufferSize * sizeof(Char8);
            mBuf        = (Char8*)RamAlloc::getInstance().get(mBufferSize,true);
        }

        clear();
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpBufBlock::releaseBuff()
    {
        if (mBuf)
        {
            RamAlloc::getInstance().release(mBuf);
            mBuf = NULL;
            mBufferSize = 0;
        }
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpBufBlock::clear()
    {
        // 不要清空缓冲区,因为可能存在大容量缓冲区,会影响效率
        //if (mBuf)
        //	memset( mBuf, 0, mBufferSize );

        clearOVERLAPPED();
        mBufValidPos = 0;
        mUsedBufLen = 0;
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpBufBlock::clearOVERLAPPED()
    {
        memset( &mWSAOverlapped, 0, sizeof(mWSAOverlapped) );
    }

    /******************************************************************************/
    //IocpNetUdpSendDataBuffer
    /******************************************************************************/

    IocpNetUdpSendDataBuffer::IocpNetUdpSendDataBuffer(  Win32IocpNetUdpDriver* owner, UInt bufferSize )
        :mOwner(owner)
        ,mRefCount( 0 )
    {
        mIocpNetUdpBufBlock.mallocBuff( bufferSize );
    }

    //-----------------------------------------------------------------------------
    IocpNetUdpSendDataBuffer::~IocpNetUdpSendDataBuffer()
    {

    }

    //-----------------------------------------------------------------------------
    U32 IocpNetUdpSendDataBuffer::getTotalBuffSize()
    {
        return mIocpNetUdpBufBlock.mBufferSize;
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpSendDataBuffer::clear()
    {
        mRefCount = 0;
        mIocpNetUdpBufBlock.clear();
    }

    //-----------------------------------------------------------------------------
    U16 IocpNetUdpSendDataBuffer::getRefCount()
    {
        U16 refCount = 0;
        mCs.lock();
        {
            refCount = mRefCount;
        }
        mCs.unlock();
        return refCount;
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpSendDataBuffer::addRefCount()
    {
        mCs.lock();
        {
            mRefCount ++;
        }
        mCs.unlock();
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpSendDataBuffer::subRefCount()
    {
        mCs.lock();
        {
            mRefCount --;
        }
        mCs.unlock();
    }

    //-----------------------------------------------------------------------------
    IocpNetUdpSectionPacket* IocpNetUdpSendDataBuffer::getIocpNetSectionPacket()
    {
        return (IocpNetUdpSectionPacket*)mIocpNetUdpBufBlock.mBuf;
    }

    //-----------------------------------------------------------------------------
    Char8* IocpNetUdpSendDataBuffer::getData()
    {
        IocpNetUdpSectionPacket*  packetData = getIocpNetSectionPacket();
        return packetData->data;
    }

    //-----------------------------------------------------------------------------
    U32 IocpNetUdpSendDataBuffer::getDataMaxSize()
    {
        return 0;//mIocpNetUdpBufBlock.mBufferSize - IocpNetUdpSectionPacket_INFO::headSize;
    }
    
    //-----------------------------------------------------------------------------
    void IocpNetUdpSendDataBuffer::send(I32 netId,U16 socketIndex, U64 code )
    {
        mOwner->send( this, netId,socketIndex, code );
    }

	void IocpNetUdpSendDataBuffer::send( NetAddress& addr,U16 socketIndex )
	{
		mOwner->send(this,addr,socketIndex);
	}
    //-----------------------------------------------------------------------------
    void IocpNetUdpSendDataBuffer::bind(void)
    {
        addRefCount();
    }

    //-----------------------------------------------------------------------------
    Bool IocpNetUdpSendDataBuffer::recall(void)
    {
        subRefCount();
        destroy();
        return true;
    }

    //-----------------------------------------------------------------------------
    Bool IocpNetUdpSendDataBuffer::destroy(void)
    {
        mOwner->destroyIocpNetUdpSendDataBuffer( this );
        return true;
    }

    /******************************************************************************/
    //IocpNetRecvDataBuffer
    /******************************************************************************/
    IocpNetUdpRecvDataBuffer::IocpNetUdpRecvDataBuffer(  Win32IocpNetUdpDriver* owner, UInt bufferSize )
        :mOwner(owner)
    {
        mIocpNetUdpBufBlock.mallocBuff( bufferSize );
		mRemoteAddrSize = sizeof(mRemoteAddr);
    }

    //-----------------------------------------------------------------------------
    IocpNetUdpRecvDataBuffer::~IocpNetUdpRecvDataBuffer()
    {
        
    }

    //-----------------------------------------------------------------------------
    void IocpNetUdpRecvDataBuffer::clear()
    {
        mIocpNetUdpBufBlock.clear();
    }

    //-----------------------------------------------------------------------------
    Bool IocpNetUdpRecvDataBuffer::destroy(void)
    {
        mOwner->destroyIocpNetUdpRecvDataBuffer( this );
        return true;
    }
	/******************************************************************************/
	//LogicSendPacket
	/******************************************************************************/
	LogicSendPacket::LogicSendPacket(IocpNetUdpSendDataBuffer** buffObjects,U32 count,U32 logicPacketId )
	{
		for (U32 i = 0; i < count; i++)
		{
			mSendBuffObjects.push_back(buffObjects[i]);
		}
		mLogicId = logicPacketId;
	}
	//-----------------------------------------------------------------------------
	void LogicSendPacket::send(I32 id,U16 socketIndex,U64 code)
	{
		std::vector<IocpNetUdpSendDataBuffer*>::iterator it = mSendBuffObjects.begin();
		for (; it != mSendBuffObjects.end(); it++)
		{
			(*it)->send(id,socketIndex,code);
		}
	}
	//-----------------------------------------------------------------------------
	void LogicSendPacket::send( NetAddress& addr,U16 socketIndex )
	{
		std::vector<IocpNetUdpSendDataBuffer*>::iterator it = mSendBuffObjects.begin();
		for (; it != mSendBuffObjects.end(); it++)
		{
			(*it)->send(addr,socketIndex);
		}
	}
	//-----------------------------------------------------------------------------
	void LogicSendPacket::destory()
	{
		std::vector<IocpNetUdpSendDataBuffer*>::iterator it = mSendBuffObjects.begin();
		for (; it != mSendBuffObjects.end(); it++)
		{
			(*it)->destroy();
		}
		mSendBuffObjects.clear();
	}
}
