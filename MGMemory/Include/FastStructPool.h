/******************************************************************************/
#ifndef _FASTSTRUCTPOOL_H_
#define _FASTSTRUCTPOOL_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	#define FASTSTRUCTPOOL_INITSIZE 1000

	/******************************************************************************/
	template<typename Type>
	class FastStructPool
	{
	private:

		std::vector<Type*>  mObjectPoolList;
        UInt mOnePoolCapacity;
		UInt mCurrObjectPoolListSize;
		UInt mCurrObjectPoolSize;
		UInt mMallocObjectCount;

	public:

		explicit FastStructPool( UInt maxSize = FASTSTRUCTPOOL_INITSIZE );
		~FastStructPool();

		inline Type*	mallocObject();
		inline UInt&	getMallocObjectCount(){return mMallocObjectCount;};
		void			clear();

	private:

		void	handleOverflow();
		void	free();
		
	};

	//---------------------------------------------------------------------------------
	template <typename Type>
	FastStructPool<Type>::FastStructPool( UInt maxSize )
	{
		mOnePoolCapacity = maxSize;

		Type* st = new Type[mOnePoolCapacity];
		memset(st, 0, sizeof(Type) * mOnePoolCapacity);
		mObjectPoolList.push_back( st );

		clear();
	}

	//---------------------------------------------------------------------------------
	template <typename Type>
	FastStructPool<Type>::~FastStructPool()
	{
		free();
	}

	//-----------------------------------------------------------------------------
	template<typename Type>
	inline Type * FastStructPool<Type>::mallocObject()
	{
		if( mCurrObjectPoolSize >= mOnePoolCapacity )
		{
			mCurrObjectPoolListSize ++;
			mCurrObjectPoolSize = 0;

			if ( mCurrObjectPoolListSize >= mObjectPoolList.size() )
			{
				handleOverflow();
			}
		}
		
		Type* pObject = &mObjectPoolList[mCurrObjectPoolListSize][mCurrObjectPoolSize++];
		mMallocObjectCount++;

		return pObject;
	}

	//---------------------------------------------------------------------------------
	template <typename Type>
	void FastStructPool<Type>::clear()
	{
		mCurrObjectPoolListSize = 0;
		mCurrObjectPoolSize = 0;
		mMallocObjectCount = 0;
	}

	//---------------------------------------------------------------------------------
	template <typename Type>
	void FastStructPool<Type>::handleOverflow()
	{
		Type* st = new Type[mOnePoolCapacity];
		memset(st, 0, sizeof(Type) * mOnePoolCapacity);
		mObjectPoolList.push_back( st );

		mCurrObjectPoolListSize = mObjectPoolList.size() - 1;
		mCurrObjectPoolSize = 0;
	}

	//---------------------------------------------------------------------------------
	template <typename Type>
	void FastStructPool<Type>::free()
	{
		for ( UInt i = 0; i < mObjectPoolList.size(); i++ )
		{
			Type* st = mObjectPoolList[i];
			delete []st;
		}
		mObjectPoolList.clear();
		clear();
	}
}

/******************************************************************************/

#endif
