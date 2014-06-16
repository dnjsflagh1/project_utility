/******************************************************************************/
#ifndef _NETWORKERRORDEF_H_
#define _NETWORKERRORDEF_H_
/******************************************************************************/
namespace MG
{
	#define 		MG_NET_SUCCESS					0
	#define 		MG_NET_ERR						(-1)
	#define			MG_NET_ERR_NEED_INIT			(-2)
	#define 		MG_NET_ERR_CONNECT_TIMEOUT		(-3)
	#define 		MG_NET_ERR_CONNECT_DUPLICATE	(-4)
	#define 		MG_NET_ERR_NO_CONNECTION		(-5)
	#define 		MG_NET_ERR_NO_ENOUGH_MEM		(-6)
	#define 		MG_NET_ERR_NO_ENOUGH_HANDLE		(-7)
	#define 		MG_NET_ERR_INVALID_HANDLE		(-8)

	#define 		MG_NET_ERR_RECV_EMPTY			(-9)
	#define 		MG_NET_ERR_RECV_BUF_OVERFLOW	(-10)

	#define			MG_NET_ERR_ADDR_INVALID			(-11)
	#define			MG_NET_ERR_CONNECTION_RESET		(-12)

	#define			MG_NET_ERR_NEED_CONNECT			(-13)

	#define			MG_NET_ERR_DRIVER_TYPE_INVALID	(-14)
	#define			MG_NET_ERR_INVALID_CLIENT_ID	(-15)

	#define 		MG_NET_ERR_SEND_EMPTY			(-16)
	#define 		MG_NET_ERR_SEND_BUF_OVERFLOW	(-17)

	#define			MG_NET_ERR_PACKET_FREE_FAILED	(-18)

	#define			MG_NET_ERR_INVALID_PARAM		(-19)

	#define			MG_NET_ERR_MAX_REMOTES			(-20)

	#define			MG_NET_ERR_INVALID_REMOTE_ID	(-21)

	#define			MG_NET_ERR_ENCODING_FAILED		(-22)

	#define			MG_NET_ERR_INVALID_CALL			(-23)

	#define			MG_NET_ERR_SEND_ON_CLOSING_REMOTE	(-24)
}


#endif 