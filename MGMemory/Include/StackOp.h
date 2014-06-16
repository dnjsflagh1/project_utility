/******************************************************************************/
#ifndef _STACKOP_H_
#define _STACKOP_H_
/******************************************************************************/

/******************************************************************************/
namespace MG
{
    /******************************************************************************/
    //Stack ²Ù×÷
    /******************************************************************************/
    class StackOp
    {
    public:
        static mhandle_t    create( long stack_size );
        static void         release( mhandle_t no );

        static long		    push( mhandle_t no, long value );
        static long		    pop( mhandle_t no );
        static long		    getTop( mhandle_t no );
        static long		    getCount( mhandle_t no );
        static long		    clean( mhandle_t no );

        static int		    getMemUsed( mhandle_t no );
        static long		    debugCheck( mhandle_t no );
    };

}

/******************************************************************************/

#endif