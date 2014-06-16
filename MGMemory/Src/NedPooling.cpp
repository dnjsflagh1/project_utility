/******************************************************************************/

////////////////////////////////////////////////////////////////////
// include ned implementation


#include "stdafx.h"
#include "NedPooling.h"

#include <nedmalloc.c>
#include <memory>
#include <limits>

#define MG_SIMD_ALIGNMENT  16

/******************************************************************************/

namespace MG
{
	namespace _NedPoolingIntern
	{
		const UInt s_poolCount = 14; // Needs to be greater than 4
		void* s_poolFootprint = reinterpret_cast<void*>(0xBB1AA45A);
		nedalloc::nedpool* s_pools[s_poolCount + 1] = { 0 };
		nedalloc::nedpool* s_poolsAligned[s_poolCount + 1] = { 0 };

		UInt poolIDFromSize(UInt a_reqSize)
		{
			// Requests size 16 or smaller are allocated at a 4 byte granularity.
			// Requests size 17 or larger are allocated at a 16 byte granularity.
			// With a s_poolCount of 14, requests size 177 or larger go in the default pool.

			// spreadsheet style =IF(B35<=16; FLOOR((B35-1)/4;1); MIN(FLOOR((B35-1)/16; 1) + 3; 14))

			UInt poolID = 0;

			if (a_reqSize > 0)
			{
				if (a_reqSize <= 16)
				{
					poolID = (a_reqSize - 1) >> 2;
				}
				else
				{
					poolID = std::min<UInt>(((a_reqSize - 1) >> 4) + 3, s_poolCount);
				}
			}

			return poolID;
		}

		void* internalAlloc(UInt a_reqSize)
		{
			UInt poolID = poolIDFromSize(a_reqSize);
			nedalloc::nedpool* pool(0); // A pool pointer of 0 means the default pool.

			if (poolID < s_poolCount)
			{
				if (s_pools[poolID] == 0)
				{
					// Init pool if first use

					s_pools[poolID] = nedalloc::nedcreatepool(0, 8);
					nedalloc::nedpsetvalue(s_pools[poolID], s_poolFootprint); // All pools are stamped with a footprint
				}

				pool = s_pools[poolID];
			}

			return nedalloc::nedpmalloc(pool, a_reqSize);
		}

		void* internalAllocAligned(UInt a_align, UInt a_reqSize)
		{
			UInt poolID = poolIDFromSize(a_reqSize);
			nedalloc::nedpool* pool(0); // A pool pointer of 0 means the default pool.

			if (poolID < s_poolCount)
			{
				if (s_poolsAligned[poolID] == 0)
				{
					// Init pool if first use

					s_poolsAligned[poolID] = nedalloc::nedcreatepool(0, 8);
					nedalloc::nedpsetvalue(s_poolsAligned[poolID], s_poolFootprint); // All pools are stamped with a footprint
				}

				pool = s_poolsAligned[poolID];
			}

			return nedalloc::nedpmemalign(pool, a_align, a_reqSize);
		}

		void internalFree(void* a_mem)
		{
			if (a_mem)
			{
				nedalloc::nedpool* pool(0);

				// nedalloc lets us get the pool pointer from the memory pointer
				void* footprint = nedalloc::nedgetvalue(&pool, a_mem);

				// Check footprint
				if (footprint == s_poolFootprint)
				{
					// If we allocated the pool, deallocate from this pool...
					nedalloc::nedpfree(pool, a_mem);
				}
				else
				{
					// ...otherwise let nedalloc handle it.
					nedalloc::nedfree(a_mem);
				}
			}
		}
	}

	//---------------------------------------------------------------------
	void* NedPoolingImpl::allocBytes(UInt count, 
		const char* file, int line, const char* func)
	{
		void* ptr = _NedPoolingIntern::internalAlloc(count);

		// avoid unused params warning
		file = func = "";
		line = 0;

		return ptr;
	}
	//---------------------------------------------------------------------
	void NedPoolingImpl::deallocBytes(void* ptr)
	{
		// deal with null
		if (!ptr)
			return;

		_NedPoolingIntern::internalFree(ptr);
	}
	//---------------------------------------------------------------------
	void* NedPoolingImpl::allocBytesAligned(UInt align, UInt count, 
		const char* file, int line, const char* func)
	{
		// default to platform SIMD alignment if none specified
		void* ptr =  align ? _NedPoolingIntern::internalAllocAligned(align, count)
			: _NedPoolingIntern::internalAllocAligned(MG_SIMD_ALIGNMENT, count);

		// avoid unused params warning
		file = func = "";
		line = 0;

		return ptr;
	}
	//---------------------------------------------------------------------
	void NedPoolingImpl::deallocBytesAligned(UInt align, void* ptr)
	{
		// deal with null
		if (!ptr)
			return;
		_NedPoolingIntern::internalFree(ptr);
	}

}


