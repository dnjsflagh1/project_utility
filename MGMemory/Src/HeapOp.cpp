/******************************************************************************/
#include "stdafx.h"
#include "HeapOp.h"
/******************************************************************************/

namespace MG
{

#ifndef MG_USE_LIBRARY_MALLOC
    //#define MG_USE_LIBRARY_MALLOC	1
#endif


#ifndef MG_CRASH_ON_STATIC_ALLOCATION
    //#define MG_CRASH_ON_STATIC_ALLOCATION
#endif

#if (defined(_DEBUG) || defined(_RV_MEM_SYS_SUPPORT)) && !defined(ENABLE_INTEL_SMP)

    MemScopedTag* MemScopedTag::mTop = NULL;

#endif
    
    static Critical HeapCS;

    class HeapThreadCSWrapper
    {
    public:
        HeapThreadCSWrapper()
        {
            HeapCS.lock();
        }
        ~HeapThreadCSWrapper()
        {
            HeapCS.unlock();
        }
    };

    // ============
    //
    // 宏定义
    //
    // ============

    #define MG_SMALL_HEAP_HEADER_SIZE		( (I32)( sizeof(Char) + sizeof(Char) ) )
    #define MG_MEDIUM_HEAP_HEADER_SIZE		( (I32)( sizeof(MediumHeapHeader_s) + sizeof(Char) ) )
    #define MG_LARGE_HEAP_HEADER_SIZE		( (I32)( sizeof(void*) + sizeof(Char) ) )

    #define MG_ALIGN_SIZE( bytes )			( ( (bytes) + ALIGN - 1 ) & ~(ALIGN -1 ) )
    #define MG_ALIGN_SMALL_SIZE( bytes )	( MG_ALIGN_SIZE( (bytes) + MG_SMALL_HEAP_HEADER_SIZE ) - MG_SMALL_HEAP_HEADER_SIZE )
    #define MG_MEDIUM_MIN_SIZE				( MG_ALIGN_SIZE( 256 ) + MG_ALIGN_SIZE(MG_MEDIUM_HEAP_HEADER_SIZE) )


    // ==============================================================================
    //
    // MGHeap
    //
    // ==============================================================================

    class MGHeap
    {
    public:

        MGHeap();
        ~MGHeap();

        void		Init();									// 初始化
        void		Shutdown();								// 释放所有的内存资源

        void*		Alloc( const U32 size );				// 申请内存
        void		Free( void* p );						// 释放内存
        I32			MSize( void* p );						// 返回内存的大小

        void		DumpToFile( const Char* fileName );	// 保存当前内存到指定的文件

        void		DumpToConsole( );

        bool		AllocDefragBlock();						// 申请一个大内存为内存申请失败时做准备

        //private:
        enum
        {
            ALIGN = 8,										// 内存大小按ALIGN字节对齐
        };

        //内存分配的标志
        enum
        {
            INVALID_ALLOC			= 0xdd,						//
            SMALL_ALLOC				= 0xaa,						// 小内存 ( 1 ~ 255 )
            MEDIUM_ALLOC			= 0xbb,						// 中等大小内存 ( 256 ~ 32767 )
            LARGE_ALLOC				= 0xcc						// 大内存 ( 大于32768 )
        };

        struct Page_s
        {
            void*					mData;						// 当前page对应分配内存的地址
            U32						mDataSize;					// mData的大小
            Page_s*					mNext;						// 下一个Page
            Page_s*					mPrev;						// 上一个Page
            U32						mLargestFreeSize;			// 用于MEDIUM_HEAP, 标记当前Page中最大的块大小
            void*					mFirstFreeData;				// 用于MEDIUM_HEAP, 标记当前Page中第一个Free内存块的大小
        };

        // 对应于
        struct MediumHeapHeader_s
        {
            Page_s*					mPage;						// 当前内存块对应的Page
            U32						mSize;						// 当前块大小
            MediumHeapHeader_s*		mNext;						// 下一个块
            MediumHeapHeader_s*		mPrev;						// 上一个块
            MediumHeapHeader_s*		mNextFree;					// 下一个Free的内存块
            MediumHeapHeader_s*		mPrevFree;					// 上一个Free的内存块
            U32						mFreeBlock;					// 当前块是否是Free的
        };

        // Small Heap
        void*		mSmallFirstFree[256/ALIGN + 1];				// Small Heap Block的Free块的起始指针
        Page_s*		mSmallCurPage;								// 当前使用的Page
        U32			mSmallCurPageOffset;						// 当前使用的Page中没有用到的指针的偏移值
        Page_s*		mSmallFirstUsedPage;						// 第一个已用的Page

        Page_s*		mMediumFirstFreePage;						// 第一个Free Page
        Page_s*		mMediumLastFreePage;						// 最后一个Free Page
        Page_s*		mMediumFirstUsedPage;						// 第一个已用的Page

        Page_s*		mLargeFirstUsedPage;						// 第一个已用Large Heap Page

        Page_s*		mSwapPage;

        I32			mAllocatedPageNum;							// 当前申请的Page数量
        U32			mPageSize;									// 默认页大小

        I32			mPageRequests;								// 当前请求的Allocated Page数量
        I32			mPageRealAlloced;							// 实际Allocated Page数量

        I32			mAllocedCounter;							// 内存申请数量计数

        I32			mSmallAllocNum;
        I32			mMediumAllocNum;
        I32			mLargeAllocNum;

        void*		mDefragBlock;


        Page_s*		AllocPage( U32 pageSize );					// 从系统申请一页(Page_s)内存
        void		FreePage( MGHeap::Page_s* page );			// 释放一页内存

        void*		SmallAlloc( U32 size );
        void		SmallFree( void* p );

        void*		MediumAllocFromPage( MGHeap::Page_s* page, U32 sizeNeeded );
        void*		MediumAlloc( U32 size );
        void		MediumFree( void* p );

        void*		LargeAlloc( U32 size );
        void		LargeFree( void* p );

        void		ReleaseSwappedPage( void );
        void		FreePageReal( MGHeap::Page_s* page );

        bool		SmallAllocCheck( void* p );
    };

    // ============
    //
    // MGHeap
    //
    // ============

    MGHeap::Page_s* MGHeap::AllocPage( U32 pageSize )
    {
        Page_s* p = NULL;
        mPageRequests++;

        if ( mSwapPage && mSwapPage->mDataSize == pageSize )
        {
            p = mSwapPage;
            mSwapPage = NULL;
        }
        else
        {
            U32 allocSize = pageSize + sizeof(Page_s);

            p = (Page_s*)malloc( allocSize + ALIGN - 1 );
            if ( !p )
            {
                if ( mDefragBlock )
                {
                    free( mDefragBlock );
                    mDefragBlock = NULL;
                    p = (Page_s*)malloc( allocSize + ALIGN - 1 );
                    AllocDefragBlock();
                }
                if ( !p )
                {
                    MG_LOG( out_error, MG_STR("%s error: no enough memory\n"), __MG_FUNC__ );
                    return p;
                }
            }
            memset( p, 0x11, allocSize + ALIGN - 1 );
            p->mData = (void*)( MG_ALIGN_SIZE( (mhandle_t)((Char*)p + sizeof(MGHeap::Page_s)) ) );
            p->mDataSize = allocSize - sizeof(MGHeap::Page_s);
            p->mLargestFreeSize = 0;
            p->mFirstFreeData = NULL;
            mPageRealAlloced++;
        }

        p->mNext	= NULL;
        p->mPrev	= NULL;

        mAllocatedPageNum++;

        return p;
    }

    // ============
    //
    // FreePage
    //
    // ============

    void MGHeap::FreePage( MGHeap::Page_s* page )
    {
        assert( page );

        if ( page->mDataSize == mPageSize && !mSwapPage )
        {
            mSwapPage = page;
        }
        else
        {
            FreePageReal( page );
        }

        mAllocatedPageNum--;
    }

    // ============
    //
    // SmallAlloc
    //		申请1~255大小的空间
    //
    // ============

    void* MGHeap::SmallAlloc( U32 size )
    {
        // 需要一个指针大小的空间保存Free块的链表地址
        if ( size < sizeof(void*) )
        {
            size = sizeof(void*);
        }

        size = MG_ALIGN_SIZE( size );

        Char*	block = (Char*)mSmallFirstFree[size/ALIGN];
        if ( block )
        {
            block[1] = (Char)SMALL_ALLOC;

            Char* data = block + MG_SMALL_HEAP_HEADER_SIZE;

            mSmallFirstFree[size/ALIGN] = *(void**)data;

            assert( SmallAllocCheck( data ) );
            return (void*)data;
        }

        U32	leftSize = mPageSize - mSmallCurPageOffset;
        if ( leftSize < size + MG_SMALL_HEAP_HEADER_SIZE )
        {
            // 申请新Page
            Page_s* page = AllocPage( mPageSize );
            if ( page == NULL )
            {
                MG_LOG( out_error, MG_STR("%s error, alloc %d memory faile\n"), __MG_FUNC__, size );
                return NULL;
            }

            mSmallCurPage->mNext	= mSmallFirstUsedPage;
            mSmallFirstUsedPage		= mSmallCurPage;

            mSmallCurPage = page;

            mSmallCurPageOffset = MG_ALIGN_SIZE( 0 );
        }

        block			= ( (Char*)mSmallCurPage->mData + mSmallCurPageOffset );
        block[0]		= (Char)(size / ALIGN);
        block[1]		= (Char)SMALL_ALLOC;

        mSmallCurPageOffset		+= size + MG_SMALL_HEAP_HEADER_SIZE;
        if ( mSmallCurPageOffset > mPageSize )
        {
            assert( true );
        }

        Char* data = block + MG_SMALL_HEAP_HEADER_SIZE;

        assert( SmallAllocCheck( data ) );

        return (void*)( data );
    }

    // ============
    //
    // SmallFree
    //
    // ============

    void MGHeap::SmallFree( void* p )
    {
        assert( p );

        assert( SmallAllocCheck( p ) );

        Char* block = (Char*)p - MG_SMALL_HEAP_HEADER_SIZE;

        block[1] = (Char)INVALID_ALLOC;

        I32 idx = (I32)block[0];

        assert( idx <= ( 256 / ALIGN ) );
        if ( idx > ( 256 / ALIGN ) )
        {
            MG_LOG( out_error, MG_STR("%s error, idx(%d) out of range\n"), __MG_FUNC__, idx );
            return;
        }

        void** data = (void**)p;

        // 将当前的block连接到对应大小的Free block list中
        *data = mSmallFirstFree[idx];
        mSmallFirstFree[idx] = block;
    }

    // ============
    //
    // MediumAllocFromPage
    //		从给定的页面中分配一个指定大小的内存块
    //
    // ============

    void* MGHeap::MediumAllocFromPage( MGHeap::Page_s* page, U32 sizeNeeded )
    {
        MediumHeapHeader_s* firstBlock = (MediumHeapHeader_s*)page->mFirstFreeData;
        MediumHeapHeader_s* block = NULL;

        assert( firstBlock );
        assert( firstBlock->mSize == page->mLargestFreeSize );
        assert( firstBlock->mSize >= sizeNeeded );

        // 检查在这次分配完后，block的大小还够不够再分配一次
        if ( firstBlock->mSize >= sizeNeeded + MG_MEDIUM_MIN_SIZE )
        {
            // 下次可能还可以用
            block = (MediumHeapHeader_s*)( (Char*)firstBlock + firstBlock->mSize - sizeNeeded );

            block->mPage		= page;
            block->mSize		= sizeNeeded;
            block->mNextFree	= NULL;
            block->mPrevFree	= NULL;
            block->mPrev		= firstBlock;
            block->mNext		= firstBlock->mNext;
            block->mFreeBlock	= 0;
            if ( firstBlock->mNext )
            {
                firstBlock->mNext->mPrev = block;
            }
            firstBlock->mNext	= block;
            firstBlock->mSize	-= sizeNeeded;

            page->mLargestFreeSize = firstBlock->mSize;
        }
        else
        {
            // 剩下的大小不够下一次分配，把整个firstBlock都分配掉
            block = firstBlock;

            // 从Free列表中移除
            if ( block->mNextFree )
            {
                block->mNextFree->mPrevFree = block->mPrevFree;
            }
            if ( block->mPrevFree )
            {
                block->mPrevFree->mNextFree = block->mNextFree;
            }
            else
            {
                page->mFirstFreeData = block->mNextFree;
            }

            block->mNextFree	= NULL;
            block->mPrevFree	= NULL;
            block->mFreeBlock	= 0;

            page->mLargestFreeSize = 0;
        }

        Char* data = (Char*)block + MG_ALIGN_SIZE( MG_MEDIUM_HEAP_HEADER_SIZE );
        data[-1] = (Char)MEDIUM_ALLOC;

        return (void*)data;
    }

    // ============
    //
    // MediumAlloc
    //		>= 256 && <= 32767
    //
    // ============

    void* MGHeap::MediumAlloc( U32 size )
    {
        Page_s* page = NULL;

        U32	neededSize = MG_ALIGN_SIZE( size ) + MG_ALIGN_SIZE( MG_MEDIUM_HEAP_HEADER_SIZE );

        // 查找当前Free列表中没有满足指定大小的页
        for ( page = mMediumFirstFreePage; page; page = page->mNext )
        {
            if ( page->mLargestFreeSize >= neededSize )
            {
                break;
            }
        }

        if ( !page )
        {
            // 分配一个新页
            page = AllocPage( mPageSize );
            if ( !page )
            {
                // 没有足够的内存
                return NULL;
            }

            page->mPrev		= NULL;
            page->mNext		= mMediumFirstFreePage;
            if ( mMediumFirstFreePage )
            {
                mMediumFirstFreePage->mPrev = page;
            }
            else
            {
                mMediumLastFreePage = page;
            }

            mMediumFirstFreePage	= page;

            page->mLargestFreeSize	= mPageSize;
            page->mFirstFreeData	= (void*)page->mData;

            MediumHeapHeader_s* block = (MediumHeapHeader_s*)(page->mFirstFreeData);

            block->mPage		= page;
            block->mSize		= mPageSize & ~(ALIGN - 1);
            block->mNext		= NULL;
            block->mPrev		= NULL;
            block->mNextFree	= NULL;
            block->mPrevFree	= NULL;
            block->mFreeBlock	= 1;
        }

        void* data = MediumAllocFromPage( page, neededSize );

        assert( data );

        // 如果当前的page大小不够最小的medium alloc把当前的page放入Used列表
        if ( page->mLargestFreeSize < MG_MEDIUM_MIN_SIZE )
        {
            if ( page == mMediumLastFreePage )
            {
                mMediumLastFreePage = page->mPrev;
            }

            if ( page == mMediumFirstFreePage )
            {
                mMediumFirstFreePage = page->mNext;
            }

            if ( page->mPrev )
            {
                page->mPrev->mNext = page->mNext;
            }

            if ( page->mNext )
            {
                page->mNext->mPrev = page->mPrev;
            }

            page->mPrev = NULL;
            page->mNext = mMediumFirstUsedPage;
            if ( mMediumFirstUsedPage )
            {
                mMediumFirstUsedPage->mPrev = page;
            }

            mMediumFirstUsedPage = page;

            return data;
        }

        // 重新设定FirstFree和LastFree，可以提高下一次查找可用页的速度
        if ( page != mMediumFirstFreePage )
        {
            assert( mMediumLastFreePage );
            assert( mMediumFirstFreePage );
            assert( page->mPrev );

            mMediumLastFreePage->mNext	= mMediumFirstFreePage;
            mMediumFirstFreePage->mPrev	= mMediumLastFreePage;
            mMediumLastFreePage			= page->mPrev;
            page->mPrev->mNext			= NULL;
            page->mPrev					= NULL;
            mMediumFirstFreePage		= page;
        }

        return data;
    }

    // ============
    //
    // MediumFree
    //
    // ============

    void MGHeap::MediumFree( void* p )
    {
        U8* data	= (U8*)p;
        data[-1]	= INVALID_ALLOC;

        MediumHeapHeader_s* block = (MediumHeapHeader_s*)(data - MG_ALIGN_SIZE( MG_MEDIUM_HEAP_HEADER_SIZE ) );
        Page_s*				page = block->mPage;
        bool				isInFreePage;

        isInFreePage = ( page->mLargestFreeSize >= MG_MEDIUM_MIN_SIZE );

        assert( block->mSize );
        assert( block->mFreeBlock == 0 );

        MediumHeapHeader_s* prev = block->mPrev;
        // 如果前一个块为空和当前块合并
        if ( prev && prev->mFreeBlock )
        {
            prev->mSize += block->mSize;
            prev->mNext = block->mNext;

            if ( block->mNext )
            {
                block->mNext->mPrev = prev;
            }

            block = prev;
        }
        else
        {
            block->mPrevFree = NULL;
            block->mNextFree = (MediumHeapHeader_s*)(page->mFirstFreeData);
            if ( block->mNextFree )
            {
                assert( !(block->mNextFree->mPrevFree) );
                block->mNextFree->mPrevFree = block;
            }

            page->mFirstFreeData = block;
            page->mLargestFreeSize = block->mSize;
            block->mFreeBlock = 1;
        }


        MediumHeapHeader_s* next = block->mNext;
        // 如果下一个块为空和当前块合并
        if ( next && next->mFreeBlock )
        {
            block->mNext = next->mNext;
            block->mSize += next->mSize;

            if ( next->mNext )
            {
                next->mNext->mPrev = block;
            }

            if ( next->mPrevFree )
            {
                next->mPrevFree->mNextFree = next->mNextFree;
            }
            else
            {
                assert( page->mFirstFreeData == next );
                page->mFirstFreeData = next->mNextFree;
            }

            if ( next->mNextFree )
            {
                next->mNextFree->mPrevFree = next->mPrevFree;
            }
        }

        if ( page->mFirstFreeData )
        {
            page->mLargestFreeSize = ((MediumHeapHeader_s*)(page->mFirstFreeData))->mSize;
        }
        else
        {
            page->mLargestFreeSize = 0;
        }

        // 检查当前块是否为所在页中的最大块
        if ( block->mSize > page->mLargestFreeSize )
        {
            assert( block != page->mFirstFreeData );
            page->mLargestFreeSize = block->mSize;

            if ( block->mPrevFree )
            {
                block->mPrevFree->mNextFree = block->mNextFree;
            }

            if ( block->mNextFree )
            {
                block->mNextFree->mPrevFree = block->mPrevFree;
            }

            block->mNextFree = (MediumHeapHeader_s*)page->mFirstFreeData;
            block->mPrevFree = NULL;

            if ( block->mNextFree  )
            {
                block->mNextFree->mPrevFree = block;
            }

            page->mFirstFreeData = block;
        }

        // 检查所在页在不在FreeList中
        if ( !isInFreePage )
        {
            if ( page->mNext )
            {
                page->mNext->mPrev = page->mPrev;
            }

            if ( page->mPrev )
            {
                page->mPrev->mNext = page->mNext;
            }

            if ( page == mMediumFirstUsedPage )
            {
                mMediumFirstUsedPage = page->mNext;
            }

            page->mNext = NULL;
            page->mPrev = mMediumLastFreePage;

            if ( mMediumLastFreePage )
            {
                mMediumLastFreePage->mNext = page;
            }

            mMediumLastFreePage = page;

            if ( !mMediumFirstFreePage )
            {
                mMediumFirstFreePage = page;
            }
        }
    }

    // ============
    //
    // LargeAlloc
    //		>= 32768
    //
    // ============

    void* MGHeap::LargeAlloc( U32 size )
    {
        Page_s* page = AllocPage( size + MG_ALIGN_SIZE( MG_LARGE_HEAP_HEADER_SIZE ) );
        if ( page == NULL )
        {
            MG_LOG( out_error, MG_STR("%s error, alloc %d bytes failed\n"), size );
            return NULL;
        }

        Char* data		= (Char*)page->mData + MG_ALIGN_SIZE( MG_LARGE_HEAP_HEADER_SIZE );
        void** block	= (void**)( data - MG_ALIGN_SIZE( MG_LARGE_HEAP_HEADER_SIZE ) );
        *block			= page;
        data[-1]		= (Char)LARGE_ALLOC;

        // 把这个页加入到已使用的链表中
        page->mPrev = NULL;
        page->mNext = mLargeFirstUsedPage;
        if ( mLargeFirstUsedPage )
        {
            mLargeFirstUsedPage->mPrev = page;
        }

        mLargeFirstUsedPage = page;

        return (void*)(data);
    }

    // ============
    //
    // LargeFree
    //
    // ============

    void MGHeap::LargeFree( void* p )
    {
        Char* data = (Char*)p;

        data[-1] = (Char)INVALID_ALLOC;

        MGHeap::Page_s* page = *(MGHeap::Page_s**)((Char*)data - MG_ALIGN_SIZE( MG_LARGE_HEAP_HEADER_SIZE ));

        if ( page->mPrev )
        {
            page->mPrev->mNext = page->mNext;
        }

        if ( page->mNext )
        {
            page->mNext->mPrev = page->mPrev;
        }

        if ( page == mLargeFirstUsedPage )
        {
            mLargeFirstUsedPage = page->mNext;
        }

        page->mPrev = NULL;
        page->mNext = NULL;

        FreePage( page );
    }

    // ============
    //
    // ReleaseSwappedPage
    //
    // ============

    void MGHeap::ReleaseSwappedPage( void )
    {
        if ( mSwapPage )
        {
            FreePageReal( mSwapPage );
            mSwapPage = NULL;
        }
    }

    // ============
    //
    // FreePageReal
    //
    // ============

    void MGHeap::FreePageReal( MGHeap::Page_s* page )
    {
        assert( page );
        if ( page )
        {
            free( page );
        }
    }

    bool MGHeap::SmallAllocCheck( void* p )
    {
        if ( mSmallCurPage == NULL )
        {
            return false;
        }

        Char* start = (Char*)mSmallCurPage->mData;

        if ( p > start && p < start + mPageSize )
        {
            return true;
        }

        if ( mSmallFirstUsedPage == NULL )
        {
            return false;
        }

        Page_s* page = mSmallFirstUsedPage;
        start = (Char*)page->mData;
        for ( ; page; page = page->mNext )
        {
            start = (Char*)page->mData;
            if ( p > start && p < start + mPageSize )
            {
                return true;
            }
        }

        return false;
    }

    // ============
    //
    // MGHeap
    //
    // ============

    MGHeap::MGHeap()
    {
        Init();
    }

    // ============
    //
    // MGHeap
    //
    // ============

    MGHeap::~MGHeap()
    {
        Shutdown();
    }

    // ============
    //
    // Init
    //
    // ============

    void MGHeap::Init()
    {
        mMediumFirstFreePage	= NULL;
        mMediumLastFreePage		= NULL;
        mMediumFirstUsedPage	= NULL;

        mLargeFirstUsedPage		= NULL;

        mSwapPage				= NULL;

        mAllocatedPageNum		= 0;
        mPageRequests			= 0;
        mPageRealAlloced		= 0;
        mAllocedCounter			= 0;

        mDefragBlock			= NULL;

        mPageSize				= 65536 - sizeof(MGHeap::Page_s);

        memset( mSmallFirstFree, 0, sizeof(mSmallFirstFree) );
        mSmallCurPage			= AllocPage( mPageSize );
        mSmallCurPageOffset		= MG_ALIGN_SIZE(0);
        mSmallFirstUsedPage		= NULL;

        mSmallAllocNum	= 0;
        mMediumAllocNum	= 0;
        mLargeAllocNum	= 0;
    }

    // ============
    //
    // Shutdown
    //
    // ============

    void MGHeap::Shutdown()
    {
        if ( mSmallCurPage )
        {
            FreePage( mSmallCurPage );
            mSmallCurPage = NULL;
        }

        Page_s* page;

        page = mSmallFirstUsedPage;
        while ( page )
        {
            Page_s* next = page->mNext;
            FreePage( page );
            page = next;
        }
        mSmallFirstUsedPage = NULL;

        page = mMediumFirstFreePage;
        while( page )
        {
            Page_s* next = page->mNext;
            FreePage( page );
            page = next;
        }
        mMediumFirstFreePage = NULL;

        page = mMediumFirstUsedPage;
        while ( page )
        {
            Page_s* next = page->mNext;
            FreePage( page );
            page = next;
        }
        mMediumFirstUsedPage = NULL;

        page = mLargeFirstUsedPage;
        while ( page )
        {
            Page_s* next = page->mNext;
            FreePage( page );
            page = next;
        }
        mLargeFirstUsedPage = NULL;

        if ( mSwapPage )
        {
            ReleaseSwappedPage( );
            mSwapPage = NULL;
        }

        if ( mDefragBlock )
        {
            free( mDefragBlock );
        }

        assert( mAllocatedPageNum == 0 );
    }

    // ============
    //
    // Alloc
    //
    // ============

    void* MGHeap::Alloc( const U32 size )
    {
#ifdef MG_USE_LIBRARY_MALLOC
        return malloc( (size_t)size );
#endif

        void* p = NULL;
        if ( size < 256 )
        {
            p = SmallAlloc( size );
            mSmallAllocNum++;
        }
        else if ( size < 32768 )
        {
            p = MediumAlloc( size );
            mMediumAllocNum++;
        }
        else
        {
            p = LargeAlloc( size );
            mLargeAllocNum++;
        }
        //Char buf[20];
        //
        //SprintF( buf, MG_STR("%p,%d\n"), p, size );


        //if ( MGLib::mLocalSys )
        //	MGLib::mLocalSys->PrintDebugString( buf );

        return p;
    }

    // ============
    //
    // Free
    //
    // ============

    void MGHeap::Free( void* p )
    {
        if ( !p )
        {
            return;
        }

#ifdef MG_USE_LIBRARY_MALLOC
        free( p );
        return;
#endif

        Char flag = ((Char*)p)[-1];

        switch ( flag )
        {
        case SMALL_ALLOC:
            {
                SmallFree( p );
            }
            break;
        case MEDIUM_ALLOC:
            {
                MediumFree( p );
            }
            break;
        case LARGE_ALLOC:
            {
                LargeFree( p );
            }
            break;
        default:
            MG_LOG( out_error, MG_STR("%s error: error heap block flag %x\n"), __MG_FUNC__, flag );
        }
    }

    // ============
    //
    // MSize
    //
    // ============

    I32 MGHeap::MSize( void* p )
    {
        if ( !p )
        {
            return 0;
        }

#ifdef MG_USE_LIBRARY_MALLOC
        return 0;
#endif

        I32 ret = 0;

        Char flag = ((Char*)p)[-1];
        switch ( flag )
        {
        case SMALL_ALLOC:
            {
                Char* block = (Char*)p - MG_SMALL_HEAP_HEADER_SIZE;
                ret = (I32)( block[0] * ALIGN );
            }
            break;
        case MEDIUM_ALLOC:
            {
                MediumHeapHeader_s* block = (MediumHeapHeader_s*)((Char*)p - MG_ALIGN_SIZE( MG_MEDIUM_HEAP_HEADER_SIZE ) );
                ret = block->mSize - MG_ALIGN_SIZE( MG_MEDIUM_HEAP_HEADER_SIZE );
            }
            break;
        case LARGE_ALLOC:
            {
                Page_s* page = *(Page_s**)((Char*)p - MG_ALIGN_SIZE(MG_LARGE_HEAP_HEADER_SIZE));
                if ( page )
                {
                    ret = page->mDataSize - MG_ALIGN_SIZE(MG_LARGE_HEAP_HEADER_SIZE);
                }
            }
            break;
        default:
            MG_LOG( out_error, MG_STR("%s error: error heap block flag %u\n"), __MG_FUNC__, flag );
        }

        return ret;
    }

    // ============
    //
    // DumpToFile
    //
    // ============

    void MGHeap::DumpToFile( const Char* fileName )
    {
    }

    // ============
    //
    // DumpToConsole
    //
    // ============

    void MGHeap::DumpToConsole( )
    {
    }

    // ============
    //
    // AllocDefragBlock
    //
    // ============

    bool MGHeap::AllocDefragBlock()
    {
        return false;
    }

    // ====================================================================================
    //
    // 内存的操作
    //
    // ====================================================================================

#undef new

    static MGHeap*			heap = NULL;
    static HeapState_t		heapAllocs = { 0, -1, 0x0fffffff, 0 };
    static HeapState_t		heapFrameAllocs;
    static HeapState_t		heapFrameFrees;


    // ============
    //
    // Heap_ClearFrameStats
    //
    // ============

    void	Heap_ClearFrameStats( )
    {
        heapFrameAllocs.mNum		= 0;
        heapFrameAllocs.mMaxSize	= -1;
        heapFrameAllocs.mMinSize	= 0x0fffffff;
        heapFrameAllocs.mTotalSize	= 0;

        heapFrameFrees.mNum			= 0;
        heapFrameFrees.mMaxSize		= -1;
        heapFrameFrees.mMinSize		= 0x0fffffff;
        heapFrameFrees.mTotalSize	= 0;
    }

    // ============
    //
    // Heap_GetFrameStats
    //
    // ============

    void	Heap_GetFrameStats( HeapState_t& allocs, HeapState_t& frees )
    {
        allocs	= heapFrameAllocs;
        frees	= heapFrameFrees;
    }

    // ============
    //
    // Heap_GetStats
    //
    // ============

    void	Heap_GetStats( HeapState_t& stats )
    {
        stats = heapAllocs;
    }

    // ============
    //
    // Heap_UpdateStats
    //
    // ============

    void	Heap_UpdateStats( HeapState_t& stats, I32 size )
    {
        stats.mNum++;
        stats.mTotalSize += size;

        if ( stats.mMaxSize < size )
        {
            stats.mMaxSize = size;
        }

        if ( stats.mMinSize > size )
        {
            stats.mMinSize = size;
        }
    }

    // ============
    //
    // Heap_UpdateAllocStats
    //
    // ============

    void	Heap_UpdateAllocStats( I32 size )
    {
        Heap_UpdateStats( heapAllocs, size );
        Heap_UpdateStats( heapFrameAllocs, size );
    }

    // ============
    //
    // Heap_UpdateFreeStats
    //
    // ============

    void	Heap_UpdateFreeStats( I32 size )
    {
        Heap_UpdateStats( heapFrameFrees, size );

        heapAllocs.mNum--;
        heapAllocs.mTotalSize -= size;
    }


    // ==============================================================================
    //
    // Heap_Init
    //
    // ==============================================================================

    bool	Heap_Init( )
    {
        HeapThreadCSWrapper hcs;
        if ( heap )
        {
            return false;
        }

        heap = new MGHeap;

        Heap_ClearFrameStats();

        return true;
    }

    // ============
    //
    // Heap_Shutdown
    //
    // ============

    void	Heap_Shutdown( )
    {
        HeapThreadCSWrapper hcs;
        MGHeap* ptr = heap;
        if ( heap )
        {
            heap = NULL;
            delete ptr;
        }
    }


    // ============
    //
    // Heap_AllocDefragBlock
    //
    // ============

    void	Heap_AllocDefragBlock( void )
    {
        HeapThreadCSWrapper hcs;
        if ( heap )
        {
            heap->AllocDefragBlock();
        }
    }

    // ============
    //
    // Heap_DumpToFile
    //
    // ============

    void	Heap_DumpToFile( const Char* fileName )
    {
        if ( heap )
        {
            heap->DumpToFile( fileName );
        }
    }

    // ============
    //
    // Heap_DumpToConsole
    //
    // ============

    void	Heap_DumpToConsole( )
    {
        //if ( MGLib::mLocalSys )
        {
            HeapState_t state;
            HeapState_t frameAlloc, frameFree;
            Heap_GetStats( state );
            Heap_GetFrameStats( frameAlloc, frameFree );

            MG_LOG( out_error, MG_STR("Heap_State: Allocated page number: %d\n"), heap->mPageRealAlloced );

            MG_LOG( out_error, MG_STR("Heap_State: small alloced %d, medium alloced %d, large alloced %d\n"),
                heap->mSmallAllocNum, heap->mMediumAllocNum, heap->mLargeAllocNum );

            MG_LOG( out_error, MG_STR("Heap_State: state: num %d, max size %d, min size %d, total size %d\n"),
                state.mNum, state.mMaxSize, state.mMinSize, state.mTotalSize );
            MG_LOG( out_error, MG_STR("Heap_State: frame alloc: num %d, max size %d, min size %d, total size %d\n"),
                frameAlloc.mNum, frameAlloc.mMaxSize, frameAlloc.mMinSize, frameAlloc.mTotalSize );
            MG_LOG( out_error, MG_STR("Heap_State: frame free: num %d, max size %d, min size %d, total size %d\n"),
                frameFree.mNum, frameFree.mMaxSize, frameFree.mMinSize, frameFree.mTotalSize );
        }
    }

#ifndef MG_DEBUG_NEWDELETE

    // ============
    //
    // Heap_Alloc
    //
    // ============

    //    void*	Heap_Alloc( const U32 size )
    //    {
    //        HeapThreadCSWrapper hcs;
    //        if ( size <= 0 )
    //        {
    //            return NULL;
    //        }
    //
    //        if ( !heap )
    //        {
    //#ifdef MG_CRASH_ON_STATIC_ALLOCATION
    //            *((int*)(0)) = 1;
    //#endif
    //            void* p = malloc( (size_t)size );
    //            return p;
    //        }
    //
    //        void* p = heap->Alloc( size );
    //        Heap_UpdateAllocStats( heap->MSize(p) );
    //
    //        return p;
    //    }

    // ============
    //
    // Heap_ClearedAlloc
    //
    // ============

    //void*	Heap_ClearedAlloc( const U32 size )
    //{
    //    HeapThreadCSWrapper hcs;
    //    void *p = Heap_Alloc( size );
    //    if ( p )
    //    {
    //        memset( p, 0, size );
    //    }

    //    return p;
    //}

    // ============
    //
    // Heap_Free
    //
    // ============

    //    void	Heap_Free( void* p )
    //    {
    //        HeapThreadCSWrapper hcs;
    //        if ( !p )
    //        {
    //            return;
    //        }
    //
    //        if ( !heap )
    //        {
    //#ifdef MG_CRASH_ON_STATIC_ALLOCATION
    //            *((int*)(0)) = 1;
    //#endif
    //            free( p );
    //            return;
    //        }
    //
    //        Heap_UpdateFreeStats( heap->MSize( p ) );
    //        heap->Free( p );
    //    }

    void	Heap_DumpLeakToConsole( )
    {
    }

    void	Heap_DumpLeakToFile( const Char* fileName )
    {
    }

#else // MG_DEBUG_NEWDELETE

    // ============
    //
    // Heap_DumpToConsole
    //
    // ============
#undef Heap_Alloc
#undef Heap_ClearedAlloc
#undef Heap_Free

#define MAX_STRING_SIZE		256

    typedef struct debugHeapInfo_s
    {
        const Char*				fileName;
        const Char*				funcName;
        U32							lineNumber;
        void*						pointer;
        I32							size;
        struct debugHeapInfo_s*		prev;
        struct debugHeapInfo_s*		next;
    } debugHeapInfo_t;

    static debugHeapInfo_t* debugMemoryPtr = NULL;

    void*	Heap_Alloc( const U32 size, const Char* fileName, const Char* funcName, I32 lineNumber )
    {
        if ( size <= 0 )
        {
            return NULL;
        }

        if ( !heap )
        {
#ifdef MG_CRASH_ON_STATIC_ALLOCATION
            *((int*)(0)) = 1;
#endif
            return  malloc( (size_t)size );
        }

        void* p = heap->Alloc( size + sizeof(debugHeapInfo_t));
        Heap_UpdateAllocStats( heap->MSize(p) );

        debugHeapInfo_t* m = (debugHeapInfo_t*)p;
        void* retPtr = (c8*)p + sizeof(debugHeapInfo_t);

        m->fileName		= fileName;
        m->funcName		= funcName;
        m->lineNumber	= lineNumber;
        m->pointer		= retPtr;
        m->size			= (I32)size;
        m->prev			= NULL;
        m->next			= debugMemoryPtr;
        if ( debugMemoryPtr )
        {
            debugMemoryPtr->prev = m;
        }

        debugMemoryPtr	= m;

        return retPtr;
    }

    void*	Heap_ClearedAlloc( const U32 size, const Char* fileName, const Char* funcName, I32 lineNumber )
    {
        void *p = Heap_Alloc( size, fileName, funcName, lineNumber );
        if ( p )
        {
            memset( p, 0, size );
        }

        return p;
    }

    void	Heap_Free( void* p, const Char* fileName, const Char* funcName, I32 lineNumber )
    {
        if ( !p )
        {
            return;
        }

        if ( !heap )
        {
#ifdef MG_CRASH_ON_STATIC_ALLOCATION
            *((int*)(0)) = 1;
#endif
            free( p );
            return;
        }

        debugHeapInfo_t* m = (debugHeapInfo_t*)((char*)p - sizeof(debugHeapInfo_t));
        if ( m->size < 0 )
        {
            MG_LOG( out_error, __MG_FUNC__ MG_STR(": bad point %p, freed twice\n"), p );
        }

        m->fileName		= fileName;
        m->funcName		= funcName;
        m->lineNumber	= lineNumber;
        m->size			= -m->size;

        if ( m->next )
        {
            m->next->prev = m->prev;
        }

        if ( m->prev )
        {
            m->prev->next = m->next;
        }
        else
        {
            debugMemoryPtr = m->next;
        }

        Heap_UpdateFreeStats( heap->MSize( m ) );
        heap->Free( m );

    }

    void	Heap_DumpLeakToConsole( )
    {

    }

    void	Heap_DumpLeakToFile( const Char* fileName )
    {
        FILE* fp = FOpen( fileName, MG_STR("w+t") );
        if ( fp == NULL )
        {
            return;
        }

        FPuts( MG_STR("pointer, size, file, function, line\n"), fp );

        debugHeapInfo_t *m = debugMemoryPtr;

        while ( m )
        {
            //MG_LOG( fp, MG_STR("%p,%d,\"%s\",\"%s\",%d\n"), m->pointer, m->size, m->fileName, m->funcName, m->lineNumber );

            m = m->next;
        }

        fclose( fp );
    }
#endif // MW_DEBUG_NEWDELETE

}

