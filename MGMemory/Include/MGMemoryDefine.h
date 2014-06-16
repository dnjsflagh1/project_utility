/******************************************************************************/
#ifndef _MGMEMORYDEFINE_H_
#define _MGMEMORYDEFINE_H_
/******************************************************************************/


/******************************************************************************/
// 系统内存分配宏
/******************************************************************************/
//重载new 和 delete
//void* operator new( std::size_t, const char*, long );
//void* operator new[]( std::size_t, const char*, long );
//#define MG_NEW new ( _FILE_, _LINE_ )

#define MG_NEW new
#define MG_DELETE delete
#define MG_DELETE_ARRAY delete[]
#define MG_SAFE_DELETE(ptr) if(ptr) { MG_DELETE (ptr); (ptr) = 0; }
#define MG_SAFE_DELETE_ARRAY(ptr) if(ptr) { MG_DELETE_ARRAY (ptr); (ptr) = 0; }

#define MG_MALLOC(size) malloc(size)
#define MG_FREE(ptr) free(ptr)


/******************************************************************************/
// 内存池分配
/******************************************************************************/

#include "NedPooling.h"

namespace MG
{
    // configure default allocators based on the options above
    // notice how we're not using the memory categories here but still roughing them out
    // in your allocators you might choose to create different policies per category

    // configurable category, for general malloc
    // notice how we ignore the category here, you could specialise
    template <int Cat> class CategorisedPoolingAllocPolicy : public NedPoolingPolicy{};
    template <int Cat, size_t align = 0> class CategorisedPoolingAlignAllocPolicy : public NedPoolingAlignedPolicy<align>{};

    /** Utility function for constructing an array of objects with placement new,
    without using new[] (which allocates an undocumented amount of extra memory
    and so isn't appropriate for custom allocators).
    */
    template<typename T>
    T* constructN(T* basePtr, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
        {
            new ((void*)(basePtr+i)) T();
        }
        return basePtr;
    }
}

/// Allocate a block of raw memory, and indicate the category of usage
#	define MG_POOL_MALLOC(bytes, category) ::MG::CategorisedPoolingAllocPolicy<category>::allocateBytes(bytes)
/// Allocate a block of memory for a primitive type, and indicate the category of usage
#	define MG_POOL_ALLOC_T(T, count, category) static_cast<T*>(::MG::CategorisedPoolingAllocPolicy<category>::allocateBytes(sizeof(T)*(count)))
/// Free the memory allocated with MG_MALLOC or MG_ALLOC_T. Category is required to be restated to ensure the matching policy is used
#	define MG_POOL_FREE(ptr, category) ::MG::CategorisedPoolingAllocPolicy<category>::deallocateBytes((void*)ptr)

/// Allocate space for one primitive type, external type or non-virtual type with constructor parameters
#	define MG_POOL_NEW_T(T, category) new (::MG::CategorisedPoolingAllocPolicy<category>::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types - do not use for classes that inherit from AllocatedObject
#	define MG_POOL_NEW_ARRAY_T(T, count, category) ::MG::constructN(static_cast<T*>(::MG::CategorisedPoolingAllocPolicy<category>::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with MG_NEW_T. Category is required to be restated to ensure the matching policy is used
#	define MG_POOL_DELETE_T(ptr, T, category) if(ptr){(ptr)->~T(); ::MG::CategorisedPoolingAllocPolicy<category>::deallocateBytes((void*)ptr);}
/// Free the memory allocated with MG_NEW_ARRAY_T. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define MG_POOL_DELETE_ARRAY_T(ptr, T, count, category) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::MG::CategorisedPoolingAllocPolicy<category>::deallocateBytes((void*)ptr);}

// aligned allocation
/// Allocate a block of raw memory aligned to SIMD boundaries, and indicate the category of usage
#	define MG_POOL_MALLOC_SIMD(bytes, category) ::MG::CategorisedPoolingAlignAllocPolicy<category>::allocateBytes(bytes)
/// Allocate a block of raw memory aligned to user defined boundaries, and indicate the category of usage
#	define MG_POOL_MALLOC_ALIGN(bytes, category, align) ::MG::CategorisedPoolingAlignAllocPolicy<category, align>::allocateBytes(bytes)
/// Allocate a block of memory for a primitive type aligned to SIMD boundaries, and indicate the category of usage
#	define MG_POOL_ALLOC_T_SIMD(T, count, category) static_cast<T*>(::MG::CategorisedPoolingAlignAllocPolicy<category>::allocateBytes(sizeof(T)*(count)))
/// Allocate a block of memory for a primitive type aligned to user defined boundaries, and indicate the category of usage
#	define MG_POOL_ALLOC_T_ALIGN(T, count, category, align) static_cast<T*>(::MG::CategorisedPoolingAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*(count)))
/// Free the memory allocated with either MG_MALLOC_SIMD or MG_ALLOC_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define MG_POOL_FREE_SIMD(ptr, category) ::MG::CategorisedPoolingAlignAllocPolicy<category>::deallocateBytes(ptr)
/// Free the memory allocated with either MG_MALLOC_ALIGN or MG_ALLOC_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define MG_POOL_FREE_ALIGN(ptr, category, align) ::MG::CategorisedPoolingAlignAllocPolicy<category, align>::deallocateBytes(ptr)

/// Allocate space for one primitive type, external type or non-virtual type aligned to SIMD boundaries
#	define MG_POOL_NEW_T_SIMD(T, category) new (::MG::CategorisedPoolingAlignAllocPolicy<category>::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types aligned to SIMD boundaries - do not use for classes that inherit from AllocatedObject
#	define MG_POOL_NEW_ARRAY_T_SIMD(T, count, category) ::MG::constructN(static_cast<T*>(::MG::CategorisedPoolingAlignAllocPolicy<category>::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with MG_NEW_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define MG_POOL_DELETE_T_SIMD(ptr, T, category) if(ptr){(ptr)->~T(); ::MG::CategorisedPoolingAlignAllocPolicy<category>::deallocateBytes(ptr);}
/// Free the memory allocated with MG_NEW_ARRAY_T_SIMD. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define MG_POOL_DELETE_ARRAY_T_SIMD(ptr, T, count, category) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::MG::CategorisedPoolingAlignAllocPolicy<category>::deallocateBytes(ptr);}
/// Allocate space for one primitive type, external type or non-virtual type aligned to user defined boundaries
#	define MG_POOL_NEW_T_ALIGN(T, category, align) new (::MG::CategorisedPoolingAlignAllocPolicy<category, align>::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types aligned to user defined boundaries - do not use for classes that inherit from AllocatedObject
#	define MG_POOL_NEW_ARRAY_T_ALIGN(T, count, category, align) ::MG::constructN(static_cast<T*>(::MG::CategorisedPoolingAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with MG_NEW_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define MG_POOL_DELETE_T_ALIGN(ptr, T, category, align) if(ptr){(ptr)->~T(); ::MG::CategorisedPoolingAlignAllocPolicy<category, align>::deallocateBytes(ptr);}
/// Free the memory allocated with MG_NEW_ARRAY_T_ALIGN. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define MG_POOL_DELETE_ARRAY_T_ALIGN(ptr, T, count, category, align) if(ptr){for (size_t _b = 0; _b < count; ++_b) { (ptr)[_b].~T();} ::MG::CategorisedPoolingAlignAllocPolicy<category, align>::deallocateBytes(ptr);}


/******************************************************************************/
// 对象池分配
/******************************************************************************/


//申请对象//每次都调用构造
#define MG_POOL_NEW(OBJ_MANAGER, CLASS_NAME, ArgList) (new(OBJ_MANAGER.retriveOrMallocObject()) CLASS_NAME ArgList)
//释放对象//每次都调用析构
#define MG_POOL_DELETE(OBJ_MANAGER, OBJ_POINTER) if (OBJ_POINTER){ OBJ_MANAGER.releaseObject(OBJ_POINTER,true); OBJ_POINTER=NULL; }

//申请对象//只在第一次创建对象内存时调用构造
#define MG_POOL_NEW_ONCE_CONSTRUCTION(OBJ_MANAGER, CLASS_NAME, ArgList) ( OBJ_MANAGER.idleEmpty() ? (new(OBJ_MANAGER.mallocObjectMemery()) CLASS_NAME ArgList):OBJ_MANAGER.retriveObject() )
//释放对象//只在释放对象内存时调用析构
#define MG_POOL_DELETE_ONCE_DESTRUCTOR(OBJ_MANAGER, OBJ_POINTER) if (OBJ_POINTER){ OBJ_MANAGER.releaseObject(OBJ_POINTER,false); OBJ_POINTER=NULL; }



/******************************************************************************/
#endif


