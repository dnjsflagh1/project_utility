/******************************************************************************/
#ifndef _GRIDPOOL_H_
#define _GRIDPOOL_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	#define MGGRIDPOOOL_POOL_CATEGORY 6001

    /******************************************************************************/
    template <typename TYPE0> class GridPool 
    {
    protected:
        Vec2    mLeftBottomPos;

        Flt		mWorldWidthHalfSize;

        Flt		mWorldHeightHalfSize;

        Flt		mWorldWidthSize;

        Flt		mWorldHeightSize;

        UInt    mWorldGridWidth;

        UInt    mWorldGridHeight;

        Flt		mWorldGridHalfWidth;

        Flt		mWorldGridHalfHeight;

        UInt	mWorldGridNum;

		Flt     mGridSize;

        Flt     mGridHalfSize;

        UInt	mAllocGroupNum;

        UInt	mAllocGroupSize;

        UInt    mSurplusGridNum;

        std::vector<TYPE0*>  mGridDataGroupList;	

    private:

        /// 是否创建了栅格
        Bool    mIsGridCreated;

    public:

        GridPool()
            :mLeftBottomPos(Vec2(0,0))
            ,mGridSize(0)
            ,mGridHalfSize(0)
            ,mIsGridCreated(false)
            ,mWorldWidthHalfSize(0)
            ,mWorldHeightHalfSize(0)
            ,mWorldWidthSize(0)
            ,mWorldHeightSize(0)
            ,mWorldGridWidth(0)
            ,mWorldGridHeight(0)
            ,mWorldGridHalfWidth(0)
            ,mWorldGridHalfHeight(0)
            ,mWorldGridNum(0)
            ,mAllocGroupSize(0)
            ,mSurplusGridNum(0)
            ,mAllocGroupNum(0){};

        virtual ~GridPool(){};

        /// 创建栅格
        virtual void        createGrids( UInt worldWidth, UInt worldHeight, Flt gridSize, Vec2 leftBottomPos, Int allocGroupNum = 10, Bool isIndexOneStart = true );

        /// 销毁栅格
        virtual void        destroyGrids();

        /// 是否有栅格
        inline Bool         isHasGrids(){ return mIsGridCreated; };

        // 清空网格
        virtual void        clearWorldGrid(Byte memSetNum = 0);

    public:

        /// 得到左下角位置
        inline  Vec2        getLeftBottomPos(){return mLeftBottomPos;};

        /// 得到区域大小
        inline  Flt         getWorldWidthSize(){return mWorldWidthSize;};

        inline  Flt         getWorldHeightSize(){return mWorldHeightSize;};


        /// 得到区域一半大小
        inline  Flt         getWorldWidtHalfSize(){return mWorldWidthHalfSize;};

        inline  Flt         getWorldHeightHalfSize(){return mWorldHeightHalfSize;};

        /// 得到栅格大小
        inline  Flt         getGridSize(){return mGridSize;};

        /// 得到栅格一边的数量 
        inline  Int         getWorldGridWidth(){return mWorldGridWidth;};

        inline  Int         getWorldGridHeight(){return mWorldGridHeight;};

        inline  Bool        getIsGridCreated(){return mIsGridCreated;};

        //////////////////////////////////////////////////////////////////////////////////

		inline void			changeWorldSize( Flt worldWidthSize, Flt worldHeightSize )
		{
			mWorldWidthSize = worldWidthSize;
			mWorldHeightSize = worldHeightSize;

			mWorldWidthHalfSize = mWorldWidthSize * 0.5;
			mWorldHeightHalfSize = worldHeightSize * 0.5;
		}

		//////////////////////////////////////////////////////////////////////////////////
		inline void			changeLeftBottomPos( Vec2 leftBottomPos )
		{
			mLeftBottomPos = leftBottomPos;
		};

		//////////////////////////////////////////////////////////////////////////////////
		inline void			changeGridSize( Flt gridSize )
		{
			mGridSize = gridSize;
			mGridHalfSize = mGridSize * 0.5;
		}

		//////////////////////////////////////////////////////////////////////////////////
        /// 得到栅格对象,通过索引
        inline TYPE0*       getGridDataByIndex( UInt index )
        {
            if(index > mWorldGridNum)
            {
                return NULL;
            }

            UInt gridGroupIndex = index / mAllocGroupSize;
            UInt gridIndex = index % mAllocGroupSize;

            TYPE0* data = mGridDataGroupList[gridGroupIndex];

            return &(data[gridIndex]);
        }

        //////////////////////////////////////////////////////////////////////////////////
        /// 得到栅格对象,通过索引
        inline TYPE0*       getGridDataByPoint( UInt x, UInt y )
        {
            // IndexOneStart
            y -= 1;

            UInt worldIndex = y * mWorldGridWidth + x;
            if(worldIndex > mWorldGridNum)
            {
                return NULL;
            }

            return getGridDataByIndex(worldIndex);
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        /// 得到栅格对象,通过世界坐标坐标
        inline TYPE0*       getGridByWorldPosition( Flt x, Flt y )
        {
            Vec2 worldPos = Vec2(x,y);
            VecU2 gridPoint;
            convertWorldPositionToGridPoint(worldPos,gridPoint);

            return  getGridDataByPoint( gridPoint.x, gridPoint.y );
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        /// 转换世界坐标到地形坐标
        inline void			convertWorldPositionToGridPoint( Vec2 worldPos, VecU2& gridPoint )
        {
            worldPos = worldPos - mLeftBottomPos;

            gridPoint.x = UInt( ( worldPos.x ) / mGridSize); 
            gridPoint.y = UInt( ( -worldPos.y ) / mGridSize);

            UInt worldGridWidth = mWorldGridWidth - 1;
            UInt worldGridHeight = mWorldGridHeight -1;


            //IndexOneStart
            gridPoint.x += 1;
            gridPoint.y += 1;
            worldGridWidth += 1; 
            worldGridHeight += 1;


            gridPoint.x = Ogre::Math::Clamp(gridPoint.x,UInt(0),(UInt)worldGridWidth);
            gridPoint.y = Ogre::Math::Clamp(gridPoint.y,UInt(0),(UInt)worldGridHeight);
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        /// 转换地形坐标到世界坐标
        inline UInt convertGridPointToGridIndex( VecU2& gridPoint )
        {
            //IndexOneStart
            return (gridPoint.y-1) * mWorldGridWidth + gridPoint.x;
        }

        /// 转换地形坐标到世界坐标
        inline UInt convertGridPointToGridIndex( UInt& gridPointX, UInt& gridPointY )
        {
            //IndexOneStart
            return (gridPointY-1) * mWorldGridWidth + gridPointX;
        }

		/// 转换地形坐标到世界坐标
		inline UInt convertGridPointToGridIndex( Int& gridPointX, Int& gridPointY )
		{
			//IndexOneStart
			return (gridPointY-1) * mWorldGridWidth + gridPointX;
		}

        //////////////////////////////////////////////////////////////////////////////////
        inline void convertGridIndexToGridPoint( UInt index, VecU2& gridPoint )
        {
            gridPoint.y = index / mWorldGridWidth;
            gridPoint.x = index % mWorldGridWidth;

            // IndexOneStart
            gridPoint.y += 1;
        }

        //////////////////////////////////////////////////////////////////////////////////
        /// 转换地形坐标到世界坐标
        inline void convertGridPointToWorldPosition( VecU2 gridPoint, Vec2&worldPos  )
        {
            // IndexOneStart
            gridPoint.x -= 1;
            gridPoint.y -= 1;

            worldPos.x =   mGridSize * gridPoint.x + mGridHalfSize  + mLeftBottomPos.x;
            worldPos.y = -(mGridSize * gridPoint.y + mGridHalfSize) + mLeftBottomPos.y;
        }

    };

    //////////////////////////////////////////////////////////////////////////////////
    template<typename TYPE0>
    void GridPool<TYPE0>::createGrids( UInt worldWidth, UInt worldHeight, Flt gridSize, Vec2 leftBottomPos, Int allocGroupNum, Bool isIndexOneStart )
    {   
        destroyGrids();

        mWorldGridWidth			= worldWidth;
        mWorldGridHeight		= worldHeight;

        mWorldGridHalfWidth     = mWorldGridWidth * Flt(0.5);

        mWorldGridHalfHeight    = mWorldGridHeight * Flt(0.5);


        mWorldGridNum			= worldWidth * worldHeight;

        mGridSize				= gridSize;
        mGridHalfSize			= mGridSize * Flt(0.5);

        mWorldWidthSize			= worldWidth * gridSize;
        mWorldHeightSize		= worldHeight * gridSize;

        mWorldWidthHalfSize		= mWorldWidthSize * Flt(0.5);
        mWorldHeightHalfSize	= mWorldHeightSize * Flt(0.5);

        mAllocGroupNum			= allocGroupNum;

		mLeftBottomPos			= leftBottomPos;
       // mLeftBottomPos			= centerPos + Vec2( -mWorldWidthHalfSize, mWorldHeightHalfSize );

        UInt worldGridNum = mWorldGridWidth * mWorldGridHeight;
        if(isIndexOneStart)
        {
            worldGridNum  += 2;
        }

        mAllocGroupSize = worldGridNum / mAllocGroupNum;
        mSurplusGridNum = worldGridNum % mAllocGroupNum;

        TYPE0* gridData = NULL;

		mGridDataGroupList.reserve( mAllocGroupNum + 1 );

        for(UInt i = 0; i < mAllocGroupNum; ++i)
        {
			//gridData = MG_POOL_NEW_ARRAY_T(TYPE0, mAllocGroupSize, MGGRIDPOOOL_POOL_CATEGORY);
            gridData = MG_NEW TYPE0 [mAllocGroupSize];
            mGridDataGroupList.push_back(gridData);
        }

        if(mSurplusGridNum > 0)
        {
			//gridData = MG_POOL_NEW_ARRAY_T(TYPE0, mSurplusGridNum, MGGRIDPOOOL_POOL_CATEGORY);
			gridData = MG_NEW TYPE0 [mSurplusGridNum];
            mGridDataGroupList.push_back(gridData);
            mAllocGroupNum++;
        }

        clearWorldGrid();

        mIsGridCreated = true;
    }

    //////////////////////////////////////////////////////////////////////////////////
    template<typename TYPE0>
    void GridPool<TYPE0>::destroyGrids(void)
    {
        if (mIsGridCreated)
        {
            TYPE0* gridData = NULL;
			UInt destroyNum = mAllocGroupSize;
            for(UInt i = 0; i < mAllocGroupNum; ++i)
            {
				if(i == mAllocGroupNum - 1)
				{
					destroyNum = mSurplusGridNum;
				}

                gridData = mGridDataGroupList[i];
				//MG_POOL_DELETE_ARRAY_T(gridData, TYPE0, destroyNum, MGGRIDPOOOL_POOL_CATEGORY);
                MG_SAFE_DELETE_ARRAY(gridData);
            }

            mGridDataGroupList.clear();
            mIsGridCreated = false;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    template<typename TYPE0>
    void GridPool<TYPE0>::clearWorldGrid(Byte memSetNum)
    {
        TYPE0* gridData = NULL;
        UInt clearNum = mAllocGroupSize;

        for(UInt i = 0; i < mAllocGroupNum; ++i)
        {
            if(i == mAllocGroupNum - 1)
            {
                clearNum = mSurplusGridNum;
            }

            memset(mGridDataGroupList[i], memSetNum, sizeof(TYPE0) * clearNum);
        }
    }
}

/******************************************************************************/

#endif
