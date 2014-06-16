/******************************************************************************/
#ifndef _NEDPOOLING_H_
#define _NEDPOOLING_H_
/******************************************************************************/

#  undef min
#  undef max

/******************************************************************************/
namespace MG
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Memory
	*  @{
	*/
	/** Non-templated utility class just to hide nedmalloc.
	*/
	class NedPoolingImpl
	{
	public:
		static void* allocBytes(UInt count, 
			const char* file, int line, const char* func);
		static void deallocBytes(void* ptr);
		static void* allocBytesAligned(UInt align, UInt count, 
			const char* file, int line, const char* func);
		static void deallocBytesAligned(UInt align, void* ptr);

	};

	/**	An allocation policy for use with AllocatedObject and 
	STLAllocator. This is the class that actually does the allocation
	and deallocation of physical memory, and is what you will want to 
	provide a custom version of if you wish to change how memory is allocated.
	@par
	This allocation policy uses nedmalloc 
	(http://nedprod.com/programs/portable/nedmalloc/index.html). 
	*/
	class NedPoolingPolicy
	{
	public:
		static inline void* allocateBytes(UInt count, 
			const char* file = 0, int line = 0, const char* func = 0)
		{
			return NedPoolingImpl::allocBytes(count, file, line, func);
		}
		static inline void deallocateBytes(void* ptr)
		{
			NedPoolingImpl::deallocBytes(ptr);
		}
		/// Get the maximum size of a single allocation
		static inline UInt getMaxAllocationSize()
		{
			return std::numeric_limits<UInt>::max();
		}

	private:
		// No instantiation
		NedPoolingPolicy()
		{ }
	};


	/**	An allocation policy for use with AllocatedObject and 
	STLAllocator, which aligns memory at a given boundary (which should be
	a power of 2). This is the class that actually does the allocation
	and deallocation of physical memory, and is what you will want to 
	provide a custom version of if you wish to change how memory is allocated.
	@par
	This allocation policy uses nedmalloc 
	(http://nedprod.com/programs/portable/nedmalloc/index.html). 
	@note
	template parameter Alignment equal to zero means use default
	platform dependent alignment.
	*/
	template <UInt Alignment = 0>
	class NedPoolingAlignedPolicy
	{
	public:
		// compile-time check alignment is available.
		typedef int IsValidAlignment
			[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

		static inline void* allocateBytes(UInt count, 
			const char* file = 0, int line = 0, const char* func = 0)
		{
			return NedPoolingImpl::allocBytesAligned(Alignment, count, file, line, func);
		}

		static inline void deallocateBytes(void* ptr)
		{
			NedPoolingImpl::deallocBytesAligned(Alignment, ptr);
		}

		/// Get the maximum size of a single allocation
		static inline UInt getMaxAllocationSize()
		{
			return std::numeric_limits<UInt>::max();
		}

	private:
		// no instantiation allowed
		NedPoolingAlignedPolicy()
		{ }
	};

	/** @} */
	/** @} */
}

/******************************************************************************/

#endif