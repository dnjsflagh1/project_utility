/******************************************************************************/
#ifndef _HEAPOP_H_
#define _HEAPOP_H_
/******************************************************************************/

/******************************************************************************/
namespace MG
{
    // ============
    //
    // 内存操作的状态
    //
    // ============

    typedef struct HeapState_s
    {
        I32				mNum;					// 次数
        I32				mMaxSize;				// 最大大小
        I32				mMinSize;				// 最小大小
        I32				mTotalSize;				// 总共大小
    } HeapState_t;

    bool	Heap_Init( );
    void	Heap_Shutdown( );
    void	Heap_ClearFrameStats( );
    void	Heap_GetFrameStats( HeapState_t& alloc, HeapState_t& free );
    void	Heap_GetStates( HeapState_t& stats );
    void	Heap_AllocDefragBlock( void );
    void	Heap_DumpToFile( const Char* fileName );
    void	Heap_DumpToConsole( );

    void	Heap_DumpLeakToConsole( );
    void	Heap_DumpLeakToFile( const Char* fileName );

#ifndef _DEBUG

    // ============
    //
    // Release版
    //
    // ============

    void*	Heap_Alloc( const U32 size );
    void*	Heap_ClearedAlloc( const U32 size );
    void	Heap_Free( void* p );

#ifdef MG_REDIRECT_NEWDELETE

    // ============
    //
    // 重定向new, delete
    //
    // ============

    MG_INLINE void* operator new( size_t size )
    {
        return Heap_Alloc( (U32)size );
    }

    MG_INLINE void operator delete( void* p )
    {
        Heap_Free( p );
    }

    MG_INLINE void* operator new[]( size_t size )
    {
        return Heap_Alloc( (U32)size );
    }

    MG_INLINE void operator delete[]( void* p )
    {
        Heap_Free( p );
    }

#endif // MG_REDIRECT_NEWDELETE

#else

    // ============
    //
    // Debug版
    //
    // ============

    void*	Heap_Alloc( const U32 size, const Char* fileName, const Char* funcName, I32 lineNumber );
    void*	Heap_ClearedAlloc( const U32 size, const Char* fileName, const Char* funcName, I32 lineNumber );
    void	Heap_Free( void* p, const Char* fileName, const Char* funcName, I32 lineNumber );

#ifdef MG_REDIRECT_NEWDELETE

    // ============
    //
    // Debug版的new, delete
    //
    // ============

    MG_INLINE void* operator new( size_t size, int t1, int t2, const Char* fileName, const Char* funcName, I32 lineNumber )
    {
        return Heap_Alloc( (U32)size, fileName, funcName, lineNumber );
    }

    MG_INLINE void  operator delete( void* p, int t1, int t2, const Char* fileName, const Char* funcName, I32 lineNumber )
    {
        Heap_Free( p, fileName, funcName, lineNumber );
    }

    MG_INLINE void* operator new[]( size_t size, int t1, int t2, const Char* fileName, const Char* funcName, I32 lineNumber )
    {
        return Heap_Alloc( (U32)size, fileName, funcName, lineNumber );
    }

    MG_INLINE void  operator delete[]( void* p, int t1, int t2, const Char* fileName, const Char* funcName, I32 lineNumber )
    {
        Heap_Free( p, fileName, funcName, lineNumber );
    }

    MG_INLINE void* operator new( size_t size )
    {
        return Heap_Alloc( (U32)size, MG_EMPTY_STRING, MG_EMPTY_STRING, 0 );
    }

    MG_INLINE void operator delete( void* p )
    {
        Heap_Free( p, MG_EMPTY_STRING, MG_EMPTY_STRING, 0 );
    }

    MG_INLINE void* operator new[]( size_t size )
    {
        return Heap_Alloc( (U32)size, MG_EMPTY_STRING, MG_EMPTY_STRING, 0 );
    }

    MG_INLINE void operator delete[]( void* p )
    {
        Heap_Free( p, MG_EMPTY_STRING, MG_EMPTY_STRING, 0 );
    }

#define MG_DEBUG_NEW    new( 0, 0, __MG_FILE__, __MG_FUNC__, __MG_LINE__ )
#undef new
#define new     MG_DEBUG_NEW

#endif // MG_REDIRECT_NEWDELETE

    void*	Heap_Alloc( const U32 size, const Char* fileName, const Char* funcName, I32 lineNumber );
    void*	Heap_ClearedAlloc( const U32 size, const Char* fileName, const Char* funcName, I32 lineNumber );
    void	Heap_Free( void* p, const Char* fileName, const Char* funcName, I32 lineNumber );

#define Heap_Alloc( size )			Heap_Alloc( size, __MG_FILE__, __MG_FUNC__, __MG_LINE__ )
#define Heap_ClearedAlloc( size )	Heap_ClearedAlloc( size, __MG_FILE__, __MG_FUNC__, __MG_LINE__ )
#define Heap_Free( size )			Heap_Free( size, __MG_FILE__, __MG_FUNC__, __MG_LINE__ )

#endif

    // ============
    //
    // 重定向new, delete
    //
    // ============

    typedef enum {
        MA_NONE = 0,
        MA_OPNEW,
        MA_DO_NOT_USE,
        MA_MAX
    } Mem_Alloc_Types_t;

#if (defined(_DEBUG) || defined(_RV_MEM_SYS_SUPPORT)) && !defined(ENABLE_INTEL_SMP)

    class MemScopedTag 
    {
        U8 mTag;
        MemScopedTag *mPrev;
        static MemScopedTag *mTop;

    public:
        MemScopedTag( U8 tag ) {
            mTag = tag;
            mPrev = mTop;
            mTop = this;
        }
        ~MemScopedTag() {
            assert( mTop != NULL );
            mTop = mTop->mPrev;
        }

        void SetTag( U8 tag ) {
            mTag = tag;
        }

        static U8 GetTopTag( void ) {
            if ( mTop ) {
                return mTop->mTag;
            } else {
                return MA_OPNEW;
            }
        }
    };
    #define MemScopedTag_GetTopTag() MemScopedTag::GetTopTag()
    #define MEM_SCOPED_TAG(var, tag) MemScopedTag var(tag)
    #define MEM_SCOPED_TAG_SET(var, tag) var.SetTag(tag)
#else
    #define MemScopedTag_GetTopTag() MA_OPNEW
    #define MEM_SCOPED_TAG(var, tag)
    #define MEM_SCOPED_TAG_SET(var, tag)
#endif

    template<class type, int blockSize, U8 memoryTag>
    class BlockAlloc 
    {
    public:
        BlockAlloc( void );
        ~BlockAlloc( void );

        void    Shutdown( void );

        type *  Alloc( void );
        void    Free( type *element );

        int     GetTotalCount( void ) const { return total; }
        int     GetAllocCount( void ) const { return active; }
        int     GetFreeCount( void ) const { return total - active; }

        size_t  Allocated( void ) const { return( total * sizeof( type ) ); }

    private:

        typedef struct element_s {
            struct element_s *	next;
            type				t;
        } element_t;
        typedef struct block_s {
            element_t			elements[blockSize];
            struct block_s *	next;
        } block_t;

        block_t *				blocks;
        element_t *				free;
        int						total;
        int						active;
    };

    template<class type, int blockSize, U8 memoryTag>
    BlockAlloc<type,blockSize,memoryTag>::BlockAlloc( void ) {
        blocks = NULL;
        free = NULL;
        total = active = 0;
    }

    template<class type, int blockSize, U8 memoryTag>
    BlockAlloc<type,blockSize,memoryTag>::~BlockAlloc( void ) {
        Shutdown();
    }

    template<class type, int blockSize, U8 memoryTag>
    type *BlockAlloc<type,blockSize,memoryTag>::Alloc( void ) {
        if ( !free ) {
            MEM_SCOPED_TAG(tag, memoryTag);
            block_t *block = new block_t;
            block->next = blocks;
            blocks = block;
            element_t *elememt;
            for ( int i = 0; i < blockSize; i++ ) {
                elememt = &block->elements[i];
                elememt->next = free;
                free = elememt;
            }
            total += blockSize;
        }
        active++;
        element_t *element = free;
        free = free->next;
        element->next = NULL;
        return &element->t;
    }

    template<class type, int blockSize, U8 memoryTag>
    void BlockAlloc<type,blockSize,memoryTag>::Free( type *t ) {
        element_t *element = (element_t *)( ( (unsigned char *) t ) - ( (mhandle_t) &((element_t *)0)->t ) );
        element->next = free;
        free = element;
        active--; 
    }

    template<class type, int blockSize, U8 memoryTag>
    void BlockAlloc<type,blockSize,memoryTag>::Shutdown( void ) {
        while( blocks ) {
            block_t *block = blocks;
            blocks = blocks->next;
            delete block;
        }
        blocks = NULL;
        free = NULL;
        total = active = 0;
    }

}

/******************************************************************************/

#endif