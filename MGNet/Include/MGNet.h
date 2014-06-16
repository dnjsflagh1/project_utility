/******************************************************************************/
#ifndef _MGNET_H_
#define _MGNET_H_
/******************************************************************************/

#if (defined(WIN32) || defined(WIN64))
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#endif

#include "Win32NetIocpData.h"
#include "Win32NetIocpDriver.h"

#include "NetDriver.h"
#include "NetManager.h"

#include "Win32IocpNetUdpConnect.h"
#include "Win32IocpNetUdpData.h"
#include "Win32IocpNetUdpDriver.h"
#include "NetUdpDriver.h"
#include "NetUdpManager.h"

#include "NetAddress.h"
#include "NetDefine.h"
#include "NetErrorDefine.h"
#include "NetSocket.h"

/******************************************************************************/

#endif

