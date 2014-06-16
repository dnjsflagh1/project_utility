/******************************************************************************/
#ifndef _GRIDFAST_H_
#define _GRIDFAST_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	#define MGGRIDFASTPOOOL_POOL_CATEGORY 7001

    /******************************************************************************/
    template <typename TYPE0> class GridFast 
    {
    public:

        Vec2    mCenterPos;

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

        UInt	mAllocGroupSize;

        Bool	mIsIndexOneStart;

        UInt    mSurplusGridNum;

        TYPE0*  mGridDataList;	

    private:

        /// 是否创建了栅格
        Bool    mIsGridCreated;

    public:

        GridFast()
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
            {};

        virtual ~GridFast(){};

        /// 创建栅格
        virtual void        createGrids( UInt worldWidth, UInt worldHeight, Flt gridSize, Vec2 leftBottomPos, Bool isIndexOneStart = true );
        /// 销毁栅格
        virtual void        destroyGrids();

        /// 是否有栅格
        inline Bool         isHasGrids(){ return mIsGridCreated; };

        // 清空网格
        void                clearWorldGrid(Byte memSetNum = 0);

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
        inline TYPE0&       getGridDataByIndex( UInt index )
        {
            return mGridDataList[index];
        }

        //////////////////////////////////////////////////////////////////////////////////
        /// 得到栅格对象,通过索引坐标点
        inline TYPE0&       getGridDataByGridPoint( VecU2& gridPoint )
        {
			gridPoint.x = Ogre::Math::Clamp(gridPoint.x,UInt(0),(UInt)mWorldGridWidth-1);
			gridPoint.y = Ogre::Math::Clamp(gridPoint.y,UInt(0),(UInt)mWorldGridHeight-1);

            // IndexOneStart
            UInt worldIndex = (gridPoint.y - 1) * mWorldGridWidth + gridPoint.x;
	
            return mGridDataList[worldIndex];
        }

		//////////////////////////////////////////////////////////////////////////////////
		/// 得到栅格对象,通过索引坐标点
		inline TYPE0&       getGridDataByGridPointFast( VecU2& gridPoint )
		{
			// IndexOneStart
			UInt worldIndex = (gridPoint.y - 1) * mWorldGridWidth + gridPoint.x;
			return mGridDataList[worldIndex];
		}
		inline TYPE0&       getGridDataByGridPointFast( UInt& gridPointX, UInt& gridPointY )
		{
			// IndexOneStart
			UInt worldIndex = (gridPointY - 1) * mWorldGridWidth + gridPointX;
			return mGridDataList[worldIndex];
		}
        /////////////////////////////////////////////////////////////////////////////////////////

        /// 得到栅格对象,通过世界坐标坐标
        inline TYPE0&       getGridByWorldPosition( Flt& x, Flt& y )
        {
            Vec2 worldPos = Vec2(x,y);
            VecU2 gridPoint;
            convertWorldPositionToGridPoint(worldPos,gridPoint);

            return  getGridDataByGridPoint( gridPoint );
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

            // IndexOneStart
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
            // IndexOneStart
            gridPoint.y -= 1;

            return gridPoint.y * mWorldGridWidth + gridPoint.x;
        }

        //////////////////////////////////////////////////////////////////////////////////
        inline void convertGridIndexToGridPoint( UInt& index, VecU2& gridPoint )
        {
            gridPoint.y = index / mWorldGridWidth;
            gridPoint.x = index % mWorldGridWidth;

            // IndexOneStart
            gridPoint.y += 1;
            
        }

        //////////////////////////////////////////////////////////////////////////////////
        /// 转换地形坐标到世界坐标
        inline void convertGridPointToWorldPosition( VecU2& gridPoint, Vec2&worldPos  )
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
    void GridFast<TYPE0>::createGrids( UInt worldWidth, UInt worldHeight, Flt gridSize, Vec2 leftBottomPos, Bool isIndexOneStart )
    {   
        destroyGrids();

        // 网格坐标数据
        mWorldGridWidth			= worldWidth;
        mWorldGridHeight		= worldHeight;
        mWorldGridHalfWidth     = mWorldGridWidth * Flt(0.5);
        mWorldGridHalfHeight    = mWorldGridHeight * Flt(0.5);
        mWorldGridNum			= mWorldGridWidth * mWorldGridHeight;

        mGridSize				= gridSize;
        mGridHalfSize			= mGridSize * Flt(0.5);

        // 世界坐标系数据
        mWorldWidthSize			= worldWidth * gridSize;
        mWorldHeightSize		= worldHeight * gridSize;

        mWorldWidthHalfSize		= mWorldWidthSize * Flt(0.5);
        mWorldHeightHalfSize	= mWorldHeightSize * Flt(0.5);

		mLeftBottomPos		= leftBottomPos;

        // 创建网格列表
        mGridDataList           =  MG_POOL_NEW_ARRAY_T(TYPE0, mWorldGridNum + 2, MGGRIDFASTPOOOL_POOL_CATEGORY);

        // 清空列表
        clearWorldGrid();

        mIsGridCreated = true;
    }

    //////////////////////////////////////////////////////////////////////////////////
    template<typename TYPE0>
    void GridFast<TYPE0>::destroyGrids(void)
    {
        if (mIsGridCreated)
        {
			MG_POOL_DELETE_ARRAY_T(mGridDataList, TYPE0, mWorldGridNum + 2, MGGRIDFASTPOOOL_POOL_CATEGORY);
            //MG_SAFE_DELETE_ARRAY(mGridDataList);
            mIsGridCreated = false;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    template<typename TYPE0>
    void GridFast<TYPE0>::clearWorldGrid(Byte memSetNum)
    {
         memset(mGridDataList, memSetNum, sizeof(TYPE0) * (mWorldGridNum + 2));
    }
}

/******************************************************************************/

#endif
