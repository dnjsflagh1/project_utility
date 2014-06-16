/******************************************************************************/
#ifndef _MGBYTESEQUENCEOP_H_
#define _MGBYTESEQUENCEOP_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//MGByteSequenceOp �ֽ����в���
	/******************************************************************************/
	class MGByteSequenceOp
	{
	public:
		//#define bswap_16(X) ((((X)&0xFF) << 8) | (((X)&=0xFF00) >> 8))
		//#define bswap_32(X) ( (((X)&0x000000FF)<<24) | (((X)&0xFF000000) >> 24) | (((X)&0x0000FF00) << 8) | (((X) &0x00FF0000) >> 8))

		static U16		byteSwap16( U16 val );
		static U32		byteSwap32( U32 val );
	};
}

#endif