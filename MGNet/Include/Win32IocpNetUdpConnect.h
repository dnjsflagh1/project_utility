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
    //Socket������������
    //��������Socket�����������
    /******************************************************************************/
    class Win32IocpNetUdpDriver;
    struct IocpNetUdpConnect
    {
        //Ψһ����
        I32					mId;	
        //�ⲿ��������
        U64				    mUserCode ; 

		Bool				mIsConnect;
		
		//�����Ǹ������˿�
		U16					mSocketIndex;
        /////////////////////////////////////////////////
        //�����ַ
        NetAddress			mAddr;

        /////////////////////////////////////////////////

        //������
        I32					mErrorNo;	
        //������Ϣ
        Char				mErrorStr[256];	

        /////////////////////////////////////////////////

        //����������Ϣ
        NetConnectInfo		mConnectInfo;

        /////////////////////////////////////////////////

        //������
        Win32IocpNetUdpDriver* mOwner;

    protected:

        //�߳���
        Critical			mCs;

    public:

        IocpNetUdpConnect(Win32IocpNetUdpDriver* owner);
        virtual ~IocpNetUdpConnect();

        //�������
        void            clear();
        //�߳�����
        void            lock();
        //�߳̽���
        void            unlock();

        //�Ƿ�����������
        Bool			isConnected();
        //�ر���������
        void			diconnect();
		void			setConnect();

        //���ñ��
        void            setID(I32 id);
        //�õ����
        I32             getID();

        //�õ������ַ
        NetAddress&     getAddress(){return mAddr;};

        //�����ⲿ�������
        void            setCode(U64 code);

        //����ⲿ�������
        Bool            checkCode(U64 code);

    };

    /******************************************************************************/
    //Socket������������ָ��
    //��NetIocpConnect�����ü������ã����˫�����ü���
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