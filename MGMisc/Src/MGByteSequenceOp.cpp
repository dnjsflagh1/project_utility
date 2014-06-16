/******************************************************************************/
#include "stdafx.h"
#include "MGByteSequenceOp.h"
/******************************************************************************/

namespace MG
{
	//=================================================================================
	U16 MGByteSequenceOp::byteSwap16( U16 val )
	{
	#ifdef WINDOWS
		return ((((val) >> 8) & 0xff) | (((val) & 0xff) << 8));
	#else 
		return bswap_16( val );
	#endif
	}

	//=================================================================================
	U32 MGByteSequenceOp::byteSwap32( U32 val )
	{
	#ifdef WINDOWS
		U32 result;
		__asm
		{
			mov eax, val
				bswap eax
				mov result, eax
		}
		return result;
	#else
		return bswap_32( val );
	#endif
	}

}