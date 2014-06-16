/******************************************************************************/
#ifndef _SHAREDPTR_H_
#define _SHAREDPTR_H_
/******************************************************************************/

#include "MGMemory.h"

/******************************************************************************/
namespace MG
{
	class TestSharePtr
	{
	public:
		static	InterLocked useCountCount;
		static	InterLocked SharedPtrCount;
		static	InterLocked sendBuffCount;
		static	InterLocked sendConfigCount;
		static	InterLocked driverCount;
	};

    #define OP_SHAREDPTR_CATEGORY 2005

    /******************************************************************************/
    //π≤œÌ÷∏’Î
    /******************************************************************************/
    template<class T> class SharedPtr
    {
    public:

        T*				pRep;
		InterLocked*	pUseCount;
        bool			mIsReleased;
		bool			mIsAllowDestroy;
		
		//static	InterLocked destoryCount;

		
		//static	InterLocked destructCount;

    public:

        /** Constructor, does not initialise the SharedPtr.
        @remarks
        <b>Dangerous!</b> You have to call bind() before using the SharedPtr.
        */
        SharedPtr() : pRep(0), pUseCount(0), mIsReleased(false), mIsAllowDestroy(true)
        {
			TestSharePtr::SharedPtrCount++;
			Int temp1 = TestSharePtr::SharedPtrCount;
			Int temp2 = TestSharePtr::useCountCount;
			Int temp3 = 0;
        }

        /** Constructor.
        @param rep The pointer to take ownership of
        @param freeMode The mechanism to use to free the pointer
        */
        template< class Y>
        explicit SharedPtr(Y* rep) 
            : pRep(rep)
            , pUseCount(rep ? MG_POOL_NEW_T( InterLocked, OP_SHAREDPTR_CATEGORY )(1) : 0)
            , mIsReleased(false)
			, mIsAllowDestroy(true)
        {
			if (rep)
			{
				TestSharePtr::useCountCount++;
			}
			TestSharePtr::SharedPtrCount++;
			DYNAMIC_EEXCEPT_LOG("SharedPtr::template< class Y>: this is forbidden !");
        }
        SharedPtr(const SharedPtr& r)
            : pRep(0), pUseCount(0), mIsReleased(false), mIsAllowDestroy(true)
        {
			TestSharePtr::SharedPtrCount++;
            // lock & copy other mutex pointer
            {
                pRep = r.pRep;
                pUseCount = r.pUseCount; 
				mIsAllowDestroy = r.mIsAllowDestroy; 
                // Handle zero pointer gracefully to manage STL containers
                if(pUseCount)
                {
                    ++(*pUseCount); 
                }
            }
        }

        //////////////////////////////////////////////////////////////////

        SharedPtr& operator=(const SharedPtr& r) {
            if (pRep == r.pRep)
                return *this;
			mIsAllowDestroy = r.mIsAllowDestroy; 
            // Swap current data into a local copy
            // this ensures we deal with rhs and this being dependent
            SharedPtr<T> tmp(r);
            swap(tmp);
            return *this;
        }

        template< class Y>
        SharedPtr(const SharedPtr<Y>& r)
            : pRep(0), pUseCount(0), mIsReleased(false), mIsAllowDestroy(true)
        {
			DYNAMIC_EEXCEPT_LOG("SharedPtr::template< class Y>: this is forbidden !");
			TestSharePtr::SharedPtrCount++;
            // lock & copy other mutex pointer
            {
                pRep = r.getPointer();
                pUseCount = r.useCountPointer();
                // Handle zero pointer gracefully to manage STL containers
                if(pUseCount)
                {
                    ++(*pUseCount);
                }
            }
        }
        template< class Y>
        SharedPtr& operator=(const SharedPtr<Y>& r) {

			DYNAMIC_EEXCEPT_LOG("SharedPtr::template< class Y>: this is forbidden !");

            if (pRep == r.getPointer())
                return *this;
            // Swap current data into a local copy
            // this ensures we deal with rhs and this being dependent
            SharedPtr<T> tmp(r);
            swap(tmp);
            return *this;
        }

        //////////////////////////////////////////////////////////////////

        virtual ~SharedPtr() {
			TestSharePtr::SharedPtrCount--;
            release();
        }

        //////////////////////////////////////////////////////////////////

        inline T& operator*() const { assert(pRep); return *pRep; }
        inline T* operator->() const { assert(pRep); return pRep; }
        inline T* get() const { return pRep; }

        //////////////////////////////////////////////////////////////////

        /** Binds rep to the SharedPtr.
        @remarks
        Assumes that the SharedPtr is uninitialised!
        */
        void bind(T* rep) {
            setNull();
            //assert(!pRep && !pUseCount);
            pUseCount = MG_POOL_NEW_T( InterLocked, OP_SHAREDPTR_CATEGORY )(1);
			TestSharePtr::useCountCount++;
            pRep = rep;
        }

        //////////////////////////////////////////////////////////////////

        inline bool unique() const { assert(pUseCount); return *pUseCount == 1; }
        inline unsigned int useCount() const { assert(pUseCount); return *pUseCount; }
        inline unsigned int* useCountPointer() const { return pUseCount; }

        //////////////////////////////////////////////////////////////////

        inline T* getPointer() const { return pRep; }

        inline bool isNull(void) const { return pRep == 0; }

        inline void setNull(void) { 
            if (pRep)
            {
                // can't scope lock mutex before release in case deleted
                
                bool destroyThis = false;

                /* If the mutex is not initialized to a non-zero value, then
                neither is pUseCount nor pRep.
                */

                {
                    // lock own mutex in limited scope (must unlock before destroy)
                    if (pUseCount)
                    {
                        if (--(*pUseCount) == 0) 
                        {
                            destroyThis = true;
                        }
                    }
                }
                if (destroyThis)
                {
					if ( mIsAllowDestroy )
					{
						destroy();

						if ( pUseCount )
						{
							// use OGRE_FREE instead of OGRE_DELETE_T since 'unsigned int' isn't a destructor
							// we only used OGRE_NEW_T to be able to use constructor
							MG_POOL_DELETE_T( pUseCount, InterLocked, OP_SHAREDPTR_CATEGORY );
							TestSharePtr::useCountCount--;
						}
					}else
					{
						DYNAMIC_EEXCEPT_LOG("SharedPtr::setNull : this sharedPtr cound not be allow to destroy point !");
					}
                }
				
				///////////////////////////////////////////////////////////////

                pRep = 0;
                pUseCount = 0;
            }
        }

    protected:

        //////////////////////////////////////////////////////////////////

        inline void release(void)
        {
            if ( mIsReleased == false )
            {
                mIsReleased = true;
				setNull();
            }
        }

        //////////////////////////////////////////////////////////////////

        virtual void destroy(void)
        { 
            DYNAMIC_EEXCEPT_LOG( " SharedPtr : no destroy !" );
            //MG_SAFE_DELETE(pRep) ;
        };

        //////////////////////////////////////////////////////////////////

        virtual void swap(SharedPtr<T> &other) 
        {
            std::swap(pRep, other.pRep);
            std::swap(pUseCount, other.pUseCount);
        }
    };

    //////////////////////////////////////////////////////////////////

    template<class T, class U> inline bool operator==(SharedPtr<T> const& a, SharedPtr<U> const& b)
    {
        return a.get() == b.get();
    }

    template<class T, class U> inline bool operator!=(SharedPtr<T> const& a, SharedPtr<U> const& b)
    {
        return a.get() != b.get();
    }

    template<class T, class U> inline bool operator<(SharedPtr<T> const& a, SharedPtr<U> const& b)
    {
        return std::less<const void*>()(a.get(), b.get());
    }
    /** @} */
    /** @} */

	//template<class T> InterLocked SharedPtr<T>::destoryCount = 0;
	
	//template<class T> InterLocked SharedPtr<T>::destructCount = 0;
}

/******************************************************************************/

#endif