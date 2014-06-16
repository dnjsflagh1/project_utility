/******************************************************************************/
#include "stdafx.h"
#include "MemBlockOp.h"
#include "StackOp.h"
/******************************************************************************/

namespace MG
{
    //------------------------------------------------------------
    typedef struct tagMBM
    {
        long        count;      // how many blocks
        long        size;       // how many byte in each block
        mhandle_t   stack;      // stack id
        void        *buf;
    } MBM;

    //------------------------------------------------------------
    long    MbmCurrCount = 0;

    //------------------------------------------------------------
    // Create a MBM struct
    // return 0 if failed
    mhandle_t MemBlockOp::create( long block_count, long block_size )
    {
        MBM *p = NULL;
        size_t h_size, b_size;
        long i;

        h_size = sizeof(MBM);
        b_size = block_count * block_size;

        if ( NULL == ( p = (MBM*)malloc( h_size + b_size ) ) )
            return 0;     // Todo... Add Error Code

        memset( p, 0, h_size + b_size );

        if ( 0 == ( p->stack = StackOp::create( block_count ) ) )
        {
            ::free(p);        // Todo.. Add Error Code
            return 0;
        }

        p->count = block_count;
        p->size  = block_size;
        p->buf   = (char*)p + h_size;

        // push block pointer into stack
        for ( i = 0; i < p->count; i++ )
        {
            long ptr = *reinterpret_cast<long*>(&(p->buf));
            StackOp::push( p->stack, ptr + i * p->size );
        }

        MbmCurrCount++;

        return (mhandle_t)p;
    }

    //------------------------------------------------------------
    // Destroy a MBM struct when it no longer used
    long MemBlockOp::release( mhandle_t no )
    {
        MBM *p = (MBM*)no;

        if ( p )
        {
            if ( p->stack > 0 )
                StackOp::release( p->stack );

            ::free( p );

            MbmCurrCount--;
        }
        return 0;
    }
    //------------------------------------------------------------
    // Clean MBM struct
    void MemBlockOp::clear( mhandle_t no )
    {
        MBM *p = (MBM*)no;
        int i;

        //    while ( StackPop( p->stack ) )
        //        ;
        StackOp::clean( p->stack );

        // push block pointer into stack
        for ( i = 0; i < p->count; i++ )
        {
            long ptr = *reinterpret_cast<long*>(&(p->buf));
            StackOp::push( p->stack, ptr + i * p->size );
        }
    }

    //------------------------------------------------------------
    // zero mem buffer
    void MemBlockOp::zero( mhandle_t no )
    {
        MBM *p = (MBM*)no;

        if ( p )
            memset( p->buf, 0, p->count * p->size );
    }

    //------------------------------------------------------------
    // return a empty block from MBM struct
    void* MbmAlloc( mhandle_t no )
    {
        MBM *p = (MBM*)no;

        long ret = StackOp::pop( p->stack );

        //return (void*)StackPop( p->stack );
        return *reinterpret_cast<void**>(&ret);
    }

    //------------------------------------------------------------
    // set a block empty
    void MemBlockOp::free( mhandle_t no, void *block )
    {
        MBM *p = (MBM*)no;
        long d;

        //d = (unsigned long)block - (unsigned long)p->buf;
        d = *reinterpret_cast<U32*>(&block) - *reinterpret_cast<U32*>(&(p->buf));
        if ( d < 0 || d >= p->count*p->size )
            return;

        //		StackPush( p->stack, (long)block );
        StackOp::push( p->stack, *reinterpret_cast<long*>(&block) );
    }

    //------------------------------------------------------------
    // return how many MBM struct has been Created
    long MemBlockOp::getCount( mhandle_t no )
    {
        MBM *p = (MBM*)no;

        return StackOp::getCount( p->stack );
    }

    //------------------------------------------------------------
    // Check is same memory address in the stack
    int MemBlockOp::getMemUsed( mhandle_t no )
    {
        MBM *p = (MBM*)no;
        int mem = 0;

        if ( p )
        {
            mem = sizeof(MBM) + p->count * p->size;
            mem += StackOp::getMemUsed( p->stack );
        }

        return mem;
    }

    //------------------------------------------------------------
    // Check is same memory address in the stack
    long MemBlockOp::debugCheck( mhandle_t no )
    {
        MBM *p = (MBM*)no;

        return StackOp::debugCheck( p->stack );
    }
    
}


