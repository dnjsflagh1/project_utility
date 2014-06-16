/******************************************************************************/
#ifndef _WIN32IOCPNETUDPCONNECT_H_
#define _WIN32IOCPNETUDPCONNECT_H_
/******************************************************************************/
#include "NetDefine.h"
#include "NetAddress.h"
#include "Lock.h"
#include "SharedPtr.h"
#include "NetSocket.h"
/******************************************************************************/
namespace MG
{
	
    /******************************************************************************/
    //Socket连接容器对象
    //用于描述Socket连接相关属性
    /******************************************************************************/
    class Win32IocpNetUdpDriver;
    struct IocpNetUdpConnect
    {
        //唯一编码
        I32					mId;	
        //外部锁定密码
        U64				    mUserCode ; 

		Bool				mIsConnect;
		
		//属于那个监听端口
		U16					mSocketIndex;
        /////////////////////////////////////////////////
        //网络地址
        NetAddress			mAddr;

        /////////////////////////////////////////////////

        //错误编号
        I32					mErrorNo;	
        //错误信息
        Char				mErrorStr[256];	

        /////////////////////////////////////////////////

        //网络连接信息
        NetConnectInfo		mConnectInfo;

        /////////////////////////////////////////////////

        //创造者
        Win32IocpNetUdpDriver* mOwner;

    protected:

        //线程锁
        Critical			mCs;

    public:

        IocpNetUdpConnect(Win32IocpNetUdpDriver* owner);
        virtual ~IocpNetUdpConnect();

        //清空属性
        void            clear();
        //线程锁定
        void            lock();
        //线程解锁
        void            unlock();

        //是否有网络连接
        Bool			isConnected();
        //关闭网络连接
        void			diconnect();
		void			setConnect();

        //设置编号
        void            setID(I32 id);
        //得到编号
        I32             getID();

        //得到网络地址
        NetAddress&     getAddress(){return mAddr;};

        //设置外部锁定编号
        void            setCode(U64 code);

        //检测外部锁定编号
        Bool            checkCode(U64 code);

    };

    /******************************************************************************/
    //Socket连接容器智能指针
    //和NetIocpConnect的引用计数合用，组成双重引用计数
    /******************************************************************************/
    class IocpNetUdpConnectPtr : public SharedPtr<IocpNetUdpConnect> 
    {
    public:
        virtual ~IocpNetUdpConnectPtr() { release(); }
        IocpNetUdpConnectPtr() : SharedPtr<IocpNetUdpConnect>() {}
        explicit IocpNetUdpConnectPtr(IocpNetUdpConnect* rep) : SharedPtr<IocpNetUdpConnect>(rep) {}
        IocpNetUdpConnectPtr(const IocpNetUdpConnectPtr& r) : SharedPtr<IocpNetUdpConnect>(r) {} 
        virtual void destroy(void);
        IocpNetUdpConnectPtr& operator=(const IocpNetUdpConnectPtr& r) 
        {
            if (pRep == r.pRep)
                return *this;
            IocpNetUdpConnectPtr tmp(r);
            swap(tmp);
            return *this;
        }
    };
    ///////////////////////////////////////////////////////////////////////////////////////
}

#endif