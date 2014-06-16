/******************************************************************************/
#include "stdafx.h"
#include "StackOp.h"
/******************************************************************************/

namespace MG
{
    //------------------------------------------------------------
    typedef struct tagSTACK
    {
        long        *b;
        long        *s;
        long        size;   // stack size
    } STACK;
    
    //------------------------------------------------------------
    long    GameStackCount = 0;

    //------------------------------------------------------------
    // Create a Stack and return Stack Id
    //      return value == 0 if malloc failed
    //
    mhandle_t StackOp::create( long stack_size )
    {
        STACK *p = NULL;
        size_t h_size, b_size;

        h_size = sizeof(STACK);
        b_size = stack_size * sizeof(long);

        if ( NULL == ( p = (STACK*)malloc( h_size + b_size ) ) )
            return 0;   // Todo... Add ErrorCode

        //    heap = GetProcessHeap();

        //    extern HANDLE _crtheap;
        //    if ( NULL == ( p = (STACK*)HeapAlloc( _crtheap, HEAP_ZERO_MEMORY, h_size + b_size ) ) )
        //        return 0;
        //    p = (STACK *)new char[h_size+b_size];
        //    if ( p == NULL )
        //        return 0;
        //    if ( NULL == ( p = (STACK*)GlobalAlloc( GPTR, h_size + b_size ) ) )
        //        return 0;
        //    if ( NULL == ( p = (STACK*)LocalAlloc( LPTR, h_size + b_size ) ) )
        //        return 0;

        // stack buffer behind struct *p
        p->s = p->b = (long*)((size_t)p + h_size);
        p->size = stack_size;

        GameStackCount++;

        return (mhandle_t)p;
    }

    //------------------------------------------------------------
    // Release a Stack availavle
    void StackOp::release( mhandle_t no )
    {
        STACK *p = (STACK*)no;
        if ( p )
        {
            free( p );
            GameStackCount--;
        }
    }

    //------------------------------------------------------------
    // Push value to specify Stack,
    //      1->Success, 0->failure
    long StackOp::push( mhandle_t no, long value )
    {
        STACK *p = (STACK*)no;

        if ( p->s - p->b >= p->size )
            return 0;

        *p->s = value;
        p->s++;
        return 1;
    }

    //------------------------------------------------------------
    // Popup top value in specify Stack,
    //      return 0 if specify Stack is empty
    long StackOp::pop( mhandle_t no )
    {
        STACK *p = (STACK*)no;

        if ( p->s == p->b )
            return 0;

        p->s--;
        return *p->s;
    }

    //------------------------------------------------------------
    // Get top value in specify Stack,
    //      return 0 if specify Stack is empty
    long StackOp::getTop( mhandle_t no )
    {
        STACK *p = (STACK*)no;

        if ( p->s == p->b )
            return 0;

        return *p->s;
    }

    //------------------------------------------------------------
    // return Stack value count specify by no
    long StackOp::getCount( mhandle_t no )
    {
        STACK *p = (STACK*)no;

        return (long)(p->s - p->b);
    }

    //------------------------------------------------------------
    // Clean a Stack
    long StackOp::clean( mhandle_t no )
    {
        STACK *p = (STACK*)no;

        p->s = p->b;
        return 1;
    }

    //------------------------------------------------------------
    // Returns memory used by current Stack struct
    int StackOp::getMemUsed( mhandle_t no )
    {
        STACK *p = (STACK*)no;
        int mem = 0;

        if ( p )
            mem = sizeof(STACK) + sizeof(long) * p->size;

        return mem;
    }

    //------------------------------------------------------------
    // Check is same value in a stack
    long StackOp::debugCheck( mhandle_t no )
    {
#ifdef _DEBUG
        STACK *p = (STACK*)no;
        long *pb, *ps;
        long value, i;

        pb = p->b;
        ps = p->s;

        while ( ps > pb )
        {
            value = *(--ps);
            for ( i = 0; i < ps - pb; i++ )
            {
                if ( value == *( pb + i ) )
                    return 1;
            }
        }

#endif
        return 0;
    }
    
}


