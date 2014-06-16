/******************************************************************************/
#ifndef _MEMBLOCKOP_H_
#define _MEMBLOCKOP_H_
/******************************************************************************/

/******************************************************************************/
namespace MG
{
    /******************************************************************************/
    //Mem Block ²Ù×÷
    /******************************************************************************/
    class MemBlockOp
    {
    public:

        static mhandle_t	create( long block_count, long block_size );
        static long			release( mhandle_t no );
        static void			clear( mhandle_t no );

        static void			zero( mhandle_t no );

        static void*		alloc( mhandle_t no );
        static void			free( mhandle_t no, void *block );

        static long			getCount( mhandle_t no );

        static int			getMemUsed( mhandle_t no );

        static long			debugCheck( mhandle_t no );
    };

}

/******************************************************************************/

#endif