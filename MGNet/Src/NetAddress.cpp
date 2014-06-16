/******************************************************************************/
#include "stdafx.h"
#include "NetAddress.h"
#include "MGByteSequenceOp.h"
/******************************************************************************/
namespace MG
{
	//-----------------------------------------------------------------------------
	void NetAddress::setPort( I32 port )
	{
		mPort = port;
		MGStrOp::sprintf( mPortStr,sizeof(mPortStr)/sizeof(mPortStr[0]), MG_STR("%d"), port );
	}
	//-----------------------------------------------------------------------------
	void NetAddress::setIP( CChar8* ipStr )
	{
		MGStrOp::sprintf( mIPStrA,sizeof(mIPStrA),"%s" ,ipStr );

	#ifdef MG_WCHAR_T_STRINGS
		std::wstring tempStr;
		MG::MGStrOp::toString(ipStr,tempStr);
		MG::MGStrOp::sprintf(mIPStr,tempStr.size(),MG_STR("%s"),tempStr.c_str());
	#else
		MG::MGStrOp::sprintf(mIPStr,sizeof(mIPStr) / sizeof(mIPStr[0]),MG_STR("%s"),ipStr);
	#endif

		mIP = net2Host( inetAddr( ipStr ) );
	}
    //-----------------------------------------------------------------------------
    U64 NetAddress::getUin()
    {
        return (U64)mIP * 10000 + mPort;
    }
	//-----------------------------------------------------------------------------
	U32 NetAddress::inetAddr( CChar8* ipStr )
	{
		static const U32 max[4] = { 0xffffffff, 0xffffff, 0xffff, 0xff };
		U32 val;
		Char8 base;
		U8 c;
		union iaddr {
		  U8	bytes[4];
		  U32	word;
		} res;
		U8 *pp = res.bytes;
		I32 digit;

		res.word = 0;

		c = *ipStr;
		for (;;) 
		{
			/*
			 * Collect number up to ``.''.
			 * vals are specified as for C:
			 * 0x=hex, 0=octal, isdigit=decimal.
			 */
			if (!isdigit(c))
			{
				return (0);
			}

			val = 0; base = 10; digit = 0;
			if (c == '0') 
			{
				c = *++ipStr;
				if (c == 'x' || c == 'X')
				{
					base = 16, c = *++ipStr;
				}
				else 
				{
					base = 8;
					digit = 1 ;
				}
			}

			for (;;)
			{
				if ( isascii(c) && isdigit(c) )
				{
					if (base == 8 && (c == '8' || c == '9'))
					{
						return (0);
					}
					val = (val * base) + (c - '0');
					c = *++ipStr;
					digit = 1;
				}
				else if (base == 16 && isascii(c) && isxdigit(c))
				{
					val = (val << 4) | (c + 10 - (islower(c) ? 'a' : 'A') );
					c = *++ipStr;
					digit = 1;
				}
				else
				{
					break;
				}
			}

			if (c == '.')
			{
				/*
				 * Internet format:
				 *  a.b.c.d
				 *  a.b.c   (with c treated as 16 bits)
				 *  a.b (with b treated as 24 bits)
				 */
				if (pp > res.bytes + 2 || val > 0xff)
				{
					return (0);
				}
				*pp++ = (U8)val;
				c = *++ipStr;
			}
			else
			{
				break;
			}
		}
		/*
		 * Check for trailing characters.
		 */
		if (c != '\0' && (!isascii(c) || !isspace(c)))
			return (0);
		/*
		 * Did we get a valid digit?
		 */
		if (!digit)
			return (0);

		/* Check whether the last part is in its limits depending on
		   the number of parts in total.  */
		if (val > max[pp - res.bytes])
		  return (0);

		return ( res.word | NetAddress::host2Net( val ) );

	}
	//-----------------------------------------------------------------------------
	void NetAddress::inetNtoA( U32 ip, Char8* ipBuf, I32 bufSize )
	{
		const U8* ipStr = reinterpret_cast<const U8*>(&ip);

		MG::MGStrOp::sprintf( ipBuf, bufSize, "%d.%d.%d.%d", ipStr[0], ipStr[1], ipStr[2], ipStr[3] );
	}
	//-----------------------------------------------------------------------------
	NetAddress::NetAddress( )
	{
		setIP( MG_NET_ADDR_IP_ANY_STR_A );
		setPort( 0 );
	}
	//-----------------------------------------------------------------------------
	NetAddress::NetAddress( I32 port )
	{
		setIP( MG_NET_ADDR_IP_ANY_STR_A );
		setPort( port );
	}
	//-----------------------------------------------------------------------------
	NetAddress::NetAddress( CChar8* ipStr, I32 port )
	{
		setIP( ipStr );
		setPort( port );
	}
	//-----------------------------------------------------------------------------
	NetAddress::NetAddress( U32 ip, I32 port )
	{
		Char8 ipStr[20];
		NetAddress::inetNtoA( host2Net(ip), ipStr, sizeof(ipStr) );
		setIP( ipStr );
		setPort( port );
	}
	//-----------------------------------------------------------------------------
	NetAddress::NetAddress( const NetAddress& addr )
	{
		setIP( addr.mIPStrA );
		setPort( addr.mPort );
	}
	//-----------------------------------------------------------------------------
	const NetAddress& NetAddress::operator=( const NetAddress& addr )
	{
		setIP( addr.mIPStrA );
		setPort( addr.mPort );
		return *this;
	}
	//-----------------------------------------------------------------------------
	bool	NetAddress::operator==( const NetAddress& addr )
	{
		return ( mIP == addr.getIP() && mPort == addr.getPort() );
	}
	//-----------------------------------------------------------------------------
	const Char8* NetAddress::getIPStrA() const
	{
		return mIPStrA;
	}
	//-----------------------------------------------------------------------------
	const Char* NetAddress::getIPStr() const
	{
		return mIPStr;
	}
	//-----------------------------------------------------------------------------
	const Char* NetAddress::getPortStr() const
	{
		return mPortStr;
	}
	//-----------------------------------------------------------------------------
	U32 NetAddress::getIP() const
	{
		return mIP;
	}
	//-----------------------------------------------------------------------------
	I32 NetAddress::getPort() const
	{
		return mPort;
	}
	//-----------------------------------------------------------------------------
	void NetAddress::clear()
	{
		setIP( MG_NET_ADDR_IP_ANY_STR_A );
		setPort( 0 );
	}
	//-----------------------------------------------------------------------------
	bool	NetAddress::isValid()
	{
		return !( ( mIP == 0 ) && ( mPort == 0 ) );
	}
	//-----------------------------------------------------------------------------
	U32 NetAddress::net2Host( U32 val )
	{
	#if MG_NET_BYTE_ORDER == MG_LITTLE_ENDIAN
		return MGByteSequenceOp::byteSwap32( val );
	#else
		return val;
	#endif
	}
	//-----------------------------------------------------------------------------
	U32 NetAddress::host2Net( U32 val )
	{
	#if MG_NET_BYTE_ORDER == MG_LITTLE_ENDIAN
		return MGByteSequenceOp::byteSwap32( val );
	#else
		return val;
	#endif
	}
	//-----------------------------------------------------------------------------
	U16 NetAddress::net2Host( U16 val )
	{
	#if MG_NET_BYTE_ORDER == MG_LITTLE_ENDIAN
		return MGByteSequenceOp::byteSwap16( val );
	#else
		return val;
	#endif
	}
	//-----------------------------------------------------------------------------
	U16 NetAddress::host2Net( U16 val )
	{
	#if MG_NET_BYTE_ORDER == MG_LITTLE_ENDIAN
		return MGByteSequenceOp::byteSwap16( val );
	#else
		return val;
	#endif
	}
	//-----------------------------------------------------------------------------
}