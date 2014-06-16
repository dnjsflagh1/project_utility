/******************************************************************************/
#include "stdafx.h"
#include "LinedState.h"
#include "DetectionMgr.h"
/******************************************************************************/

namespace MG
{

    /******************************************************************************/
    //LinedStateObject
    /******************************************************************************/
    LinedStateObject::LinedStateObject( Str name )
        :mIsDestroy(false),
        mName( name ),
        mManagedLifeTime(true),
		mAllowExternalDestroy(true),
        mUsedCount(0),
		mDelayPushTime(0),
        mStopTime(0)
    {
        clearAllListener();
    }

    //-----------------------------------------------------------------------
    LinedStateObject::~LinedStateObject()
    {
        clearNextStataList();
        clearAllListener();
    }

    //-----------------------------------------------------------------------
    U32 LinedStateObject::getUsedCount()
    {
        return mUsedCount;
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::addUsedCount()
    {
        mUsedCount++;
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::addListener(LinedStateObjectListener* listener)
    {
        std::list<LinedStateObjectListener*>::iterator iter = mListener.begin();

        Bool isExist = false;
        for ( ;iter!=mListener.end();iter++)
        {
            if ( *iter == listener )
            {
                isExist = true;
                break;
            }
        }

        if ( !isExist )
        {       
            mListener.push_back( listener );
        }
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::removeListener(LinedStateObjectListener* listener)
    {
        std::list<LinedStateObjectListener*>::iterator iter = mListener.begin();

        for ( ;iter!=mListener.end();iter++)
        {
            if ( *iter == listener )
            {
                mListener.erase( iter );
                break;
            }
        }
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::clearAllListener()
    {
        mListener.clear();
    }
    
    //-----------------------------------------------------------------------
    void LinedStateObject::notifyEnter()
    {
        onEnter();

        std::list<LinedStateObjectListener*>::iterator iter = mListener.begin();
        for ( ;iter!=mListener.end();iter++)
        {
            (*iter)->onLinedStateEnter( this );
        }

        addUsedCount();
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::notifyLeave()
    {
        onLeave();

        std::list<LinedStateObjectListener*>::iterator iter = mListener.begin();
        for ( ;iter!=mListener.end();iter++)
        {
            (*iter)->onLinedStateLeave( this );
        }
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::notifyChanged()
    {
        std::list<LinedStateObjectListener*>::iterator iter = mListener.begin();
        for ( ;iter!=mListener.end();iter++)
        {
            (*iter)->onLinedStateChanged( this );
        }
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::notifyLifeTimeOver()
    {
		std::list<LinedStateObjectListener*> localListener = mListener;

        std::list<LinedStateObjectListener*>::iterator iter = localListener.begin();
        for ( ;iter!=localListener.end();iter++)
        {
            (*iter)->onLinedStateLifeTimeOver( this );
        }
    }

    //-----------------------------------------------------------------------
    Bool LinedStateObject::isManagedLifeTime()
    {
        return mManagedLifeTime;
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::setManagedLifeTime(Bool enable)
    {
        mManagedLifeTime = enable;
        std::list<LinedStateObject*>::iterator iter = mStateList.begin();
        for ( ;iter!=mStateList.end(); iter++)
        {
            LinedStateObject* object = *iter;
            object->setManagedLifeTime( enable );
        }
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::addNextState( LinedStateObject* object )
    {
        if (object)
        {
            Str name = object->getName();
            removeNextState( name, true );
            mStateList.push_back( object ) ;
        }
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::removeNextState( Str name, Bool isErgodic )
    {
        LinedStateObject* object = NULL;
        Bool isRemove = false;

        std::list<LinedStateObject*>::iterator iter = mStateList.begin();
        for ( ; iter!=mStateList.end();  )
        {
            object = *iter;

			if ( isErgodic )
				object->removeNextState( name, isErgodic );

            isRemove = false;

            if (object->getName() == name)
            {
				if ( object->getAllowExternalDestroy() )
					isRemove = true;
            }
			
            if ( isRemove )
            {
                if ( object->isManagedLifeTime() )
                {
                    object->clearNextStataList();
                    MG_SAFE_DELETE( object );
                }

                iter = mStateList.erase( iter );
            }else
            {
                iter++;
            }
        }
        
    }

    //-----------------------------------------------------------------------
    LinedStateObject* LinedStateObject::getNextState( Str name, Bool isErgodic )
    {
        LinedStateObject* object = NULL;
        {
            std::list<LinedStateObject*>::iterator iter = mStateList.begin();
            for ( ; iter!=mStateList.end(); iter++  )
            {
                if ( (*iter)->getName() == name )
                {
                    object = *iter;
                    break;
                }
            }
        }

        {
            if ( object == NULL )
            {
                if ( isErgodic )
                {
                    std::list<LinedStateObject*>::iterator iter = mStateList.begin();
                    for ( ;iter!=mStateList.end(); iter++)
                    {
                        object = (*iter)->getNextState( name, true );
                        if ( object )
                        {
                            break;
                        }
                    }
                }
            }
        }

        return object;
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::stopUpdate( Flt time )
    {
		mStopTime = time;
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::preUpdate( Flt delta )
    {
        // 停止更新
        if ( mStopTime > 0 )
        {
            mStopTime -= delta;
            return;
        }

        update( delta );
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::clearNextStataList( Bool isCheck )
    {

        std::vector<LinedStateObject*> clearList;

        std::list<LinedStateObject*>::iterator iter = mStateList.begin();
        for ( ;iter!=mStateList.end();  )
        {
            LinedStateObject* object = *iter;

			Bool isRemove = true;
			if ( isCheck )
			{
				if ( object->getAllowExternalDestroy() == false )
					isRemove = false;
			}

			if ( isRemove )
			{
                clearList.push_back( object );
                iter = mStateList.erase( iter );

			}else
			{
				iter++;
			}
        }

        //////////////////////////////////////////////////////////////

        for( UInt i = 0; i < clearList.size() ; i++ )
        {
            LinedStateObject* object = clearList[i];

            object->notifyLeave();

            object->clearNextStataList();

            if ( object->isManagedLifeTime() )
            {
                MG_SAFE_DELETE( object ) ;
            }else
            {
                object->notifyLifeTimeOver();
            }

        }
    }

    //-----------------------------------------------------------------------
    void LinedStateObject::destroy()
    {
        mIsDestroy = true;
    }

    //-----------------------------------------------------------------------
    Bool LinedStateObject::isDestroy()
    {
        return mIsDestroy;
    }

    //-----------------------------------------------------------------------
    Str LinedStateObject::getName()
    {
        return mName;
    }

	//-----------------------------------------------------------------------
	void LinedStateObject::setDelayPushTime( Flt delayPushTimeTime )
	{
		mDelayPushTime = delayPushTimeTime;
	}

    /******************************************************************************/
    //LinedStateManager
    /******************************************************************************/
	LinedStateManager::LinedStateManager(): mIsPush(true)
    {

    }

    //-----------------------------------------------------------------------
    LinedStateManager::~LinedStateManager()
    {
        clear();
    }

    //-----------------------------------------------------------------------
    Bool LinedStateManager::isEmpty()
    {
        return mStateList.empty();
    }

	//-----------------------------------------------------------------------
	void LinedStateManager::delayPush(LinedStateObject* object,Bool isClearAll)
	{
		//FUNDETECTION(__MG_FUNC__);
		if(!mIsPush)
		{
			destroyState(object);
			return;
		}

		if (object)
		{
			Str name = object->getName();

			// 如果需要更加灵活的设置，可以把下列逻辑移到逻辑层控制
			if ( isClearAll )
			{
				clear( true );
			}else
			{
				removeState( name, true );
			}

			if ( getState( name, true ) )
			{
				destroyState(object);
			}else
			{
				object->mDelayPushTime *= 1000;
				object->mDelayPushTime += MGTimeOp::getCurrTick();
				mDelayStateList.push_back( object ) ;
			}
		}
	}

    //-----------------------------------------------------------------------
    void LinedStateManager::push(LinedStateObject* object,Bool isClearAll)
    {
		//FUNDETECTION(__MG_FUNC__);
		if(!mIsPush)
		{
			destroyState(object);
			return;
		}

        if (object)
        {
			Str name = object->getName();

			// 如果需要更加灵活的设置，可以把下列逻辑移到逻辑层控制
			if ( isClearAll )
			{
				clear( true );
			}else
			{
				removeState( name, true );
			}

			if ( getState( name, true ) )
			{
                destroyState(object);
			}else
			{
				mStateList.push_back( object ) ;
                object->notifyEnter();
				notifyAdd( object );
			}
        }
    }

    //-----------------------------------------------------------------------
    LinedStateObject* LinedStateManager::getState( Str name, Bool isErgodic )
    {
		//FUNDETECTION(__MG_FUNC__);
        LinedStateObject* object = NULL;
        {
            std::list<LinedStateObject*>::iterator iter = mStateList.begin();
            for ( ; iter!=mStateList.end(); iter++  )
            {
                if ( (*iter)->getName() == name )
                {
                    object = *iter;
                    break;
                }
            }
        }

        {
            if ( object == NULL )
            {
                if ( isErgodic )
                {
                    std::list<LinedStateObject*>::iterator iter = mStateList.begin();
                    for ( ;iter!=mStateList.end(); iter++)
                    {
                        object = (*iter)->getNextState( name, true );
                        if ( object )
                        {
                            break;
                        }
                    }
                }
            }
        }

        return object;
    }

    //-----------------------------------------------------------------------
    void LinedStateManager::removeState( Str name, Bool isErgodic )
    {
		//FUNDETECTION(__MG_FUNC__);
        LinedStateObject* object = NULL;
        Bool isRemove = false;

        std::vector<LinedStateObject*> clearList;

        std::list<LinedStateObject*>::iterator iter = mStateList.begin();
        for ( ; iter!=mStateList.end();  )
        {
            object = *iter;

			if ( isErgodic )
				object->removeNextState( name, isErgodic );

			isRemove = false;

			if ( object->getAllowExternalDestroy() )
			{
				if (object->getName() == name)
				{
					isRemove = true;
				}
			}
			
            if ( isRemove )
            {
                clearList.push_back( object );
                iter = mStateList.erase( iter );
            }else
            {
                iter++;
            }
        }

		 //////////////////////////////////////////////////////////////
		iter = mDelayStateList.begin();
		for ( ; iter!=mDelayStateList.end();  )
		{
			object = *iter;

			if ( isErgodic )
				object->removeNextState( name, isErgodic );

			isRemove = false;

			if ( object->getAllowExternalDestroy() )
			{
				if (object->getName() == name)
				{
					isRemove = true;
				}
			}

			if ( isRemove )
			{
				clearList.push_back( object );
				iter = mDelayStateList.erase( iter );
			}else
			{
				iter++;
			}
		}

        //////////////////////////////////////////////////////////////

        for( UInt i = 0; i < clearList.size() ; i++ )
        {
            LinedStateObject* object = clearList[i];

            object->notifyLeave();

            destroyState(object);
        }
    }

	//-----------------------------------------------------------------------
	void LinedStateManager::destroyState( LinedStateObject* object )
	{
		notifyRemove( object );

		///////////////////////////////////////////////////////////

        object->clearNextStataList();

        ///////////////////////////////////////////////////////////

        if ( object->isManagedLifeTime() )
        {
            MG_SAFE_DELETE( object ) ;
        }else
        {
            object->notifyLifeTimeOver();
        }
	}

    //-----------------------------------------------------------------------
    void LinedStateManager::clear(Bool isCheck)
    {
		//FUNDETECTION(__MG_FUNC__);

        std::vector<LinedStateObject*> clearList;
        std::list<LinedStateObject*>::iterator iter = mStateList.begin();
        for (; iter!=mStateList.end(); )
        {
            LinedStateObject* object = *iter;

			Bool isRemove = true;

			if ( isCheck )
			{
				if ( object->getAllowExternalDestroy() == false )
					isRemove = false;
			}

			if ( isRemove )
			{
                clearList.push_back( object );
				iter = mStateList.erase(iter);
			}else
			{
				iter++;
			}
        }

		//////////////////////////////////////////////////////////////
		iter = mDelayStateList.begin();
		for (; iter!=mDelayStateList.end(); )
		{
			LinedStateObject* object = *iter;

			Bool isRemove = true;

			if ( isCheck )
			{
				if ( object->getAllowExternalDestroy() == false )
					isRemove = false;
			}

			if ( isRemove )
			{
				clearList.push_back( object );
				iter = mDelayStateList.erase(iter);
			}else
			{
				iter++;
			}
		}

        //////////////////////////////////////////////////////////////

        for( UInt i = 0; i < clearList.size() ; i++ )
        {
            LinedStateObject* object = clearList[i];

            object->notifyLeave();

            destroyState(object);
        }
    }

    //-----------------------------------------------------------------------
    void LinedStateManager::update( Flt delta )
    {
        std::vector<LinedStateObject*> clearList;
        std::list<LinedStateObject*> prepareList;

		{
			std::list<LinedStateObject*>::iterator iter = mDelayStateList.begin();
			for (;iter!=mDelayStateList.end();)
			{
				LinedStateObject* object = *iter;
				if(MGTimeOp::getCurrTick() >= object->mDelayPushTime)
				{
					iter = mDelayStateList.erase(iter);
					push(object, false);	
				}
				else
				{
					iter++;
				}
			}
		}

        {
            std::list<LinedStateObject*>::iterator iter = mStateList.begin();
            for (;iter!=mStateList.end();)
            {
                LinedStateObject* object = *iter;

                Bool isDestroy  = object->isDestroy();
                Bool isFinished = object->isFinished();
                if ( isDestroy || isFinished )
                {
                    //把接下的状态对象移动到当前激活队列
                    std::list<LinedStateObject*>::iterator jter = object->mStateList.begin();
                    for (;jter!=object->mStateList.end();jter++)
                    {
                        prepareList.push_back(*jter);
                    }
          
                    clearList.push_back( object );

                    iter = mStateList.erase( iter );
                }else
                {
                    object->preUpdate( delta );
                    iter++;
                }
            }
        }

        
        {
            std::list<LinedStateObject*>::iterator iter = prepareList.begin();
            for (;iter!=prepareList.end(); ++iter)
            {
                push(*iter);
            }
            prepareList.clear();
        }

        {
            for( UInt i = 0; i < clearList.size() ; i++ )
            {
                LinedStateObject* object = clearList[i];

                object->notifyLeave();

                destroyState(object);
            }
        }
    }

	//-----------------------------------------------------------------------
	void LinedStateManager::addListener(LinedStateManagerListener* listener)
	{
		std::list<LinedStateManagerListener*>::iterator iter = mListener.begin();

		Bool isExist = false;
		for ( ;iter!=mListener.end();iter++)
		{
			if ( *iter == listener )
			{
				isExist = true;
				break;
			}
		}

		if ( !isExist )
		{       
			mListener.push_back( listener );
		}
	}

	//-----------------------------------------------------------------------
	void LinedStateManager::removeListener(LinedStateManagerListener* listener)
	{
		std::list<LinedStateManagerListener*>::iterator iter = mListener.begin();

		for ( ;iter!=mListener.end();iter++)
		{
			if ( *iter == listener )
			{
				mListener.erase( iter );
				break;
			}
		}
	}

	//-----------------------------------------------------------------------
	void LinedStateManager::clearAllListener()
	{
        mListener.clear();
	}

	//-----------------------------------------------------------------------
	void LinedStateManager::notifyAdd(LinedStateObject* obj)
	{
		std::list<LinedStateManagerListener*>::iterator iter = mListener.begin();
		for ( ;iter!=mListener.end();iter++)
		{
			(*iter)->onLinedStateAdd( obj );
		}
	}

	//-----------------------------------------------------------------------
	void LinedStateManager::notifyRemove(LinedStateObject* obj)
	{
		std::list<LinedStateManagerListener*>::iterator iter = mListener.begin();
		for ( ;iter!=mListener.end();iter++)
		{
			(*iter)->onLinedStateRemove( obj );
		}
	}

	//-----------------------------------------------------------------------
	void LinedStateManager::setIsPush(Bool isPush)
	{
		mIsPush = isPush;
	}

	//-----------------------------------------------------------------------
	//void LinedStateManager::push(LinedStateObject* object, LinedStateObject* ignoreObj)
	//{
	//	if(!mIsPush)
	//	{
	//		destroyState(object);
	//		return;
	//	}

	//	if (object)
	//	{
	//		Str name = object->getName();

	//		// 如果需要更加灵活的设置，可以把下列逻辑移到逻辑层控制
	//
	//		clear( object, true );

	//		if ( getState( name, true ) )
	//		{
	//			destroyState(object);
	//		}else
	//		{
	//			mStateList.push_back( object ) ;
	//			object->notifyEnter();
	//			notifyAdd( object );
	//		}
	//	}
	//}

	////-----------------------------------------------------------------------
	//void LinedStateManager::clear(LinedStateObject* ignoreObj, Bool isCheck=false)
	//{
	//	//FUNDETECTION(__MG_FUNC__);

	//	std::vector<LinedStateObject*> clearList;
	//	std::list<LinedStateObject*>::iterator iter = mStateList.begin();
	//	for (; iter!=mStateList.end(); )
	//	{
	//		LinedStateObject* object = *iter;

	//		if(ignoreObj == object)
	//		{
	//			continue;
	//		}

	//		Bool isRemove = true;

	//		if ( isCheck )
	//		{
	//			if ( object->getAllowExternalDestroy() == false )
	//				isRemove = false;
	//		}

	//		if ( isRemove )
	//		{
	//			clearList.push_back( object );
	//			iter = mStateList.erase(iter);
	//		}else
	//		{
	//			iter++;
	//		}
	//	}

	//	//////////////////////////////////////////////////////////////
	//	iter = mDelayStateList.begin();
	//	for (; iter!=mDelayStateList.end(); )
	//	{
	//		LinedStateObject* object = *iter;

	//		if(ignoreObj == object)
	//		{
	//			continue;
	//		}

	//		Bool isRemove = true;

	//		if ( isCheck )
	//		{
	//			if ( object->getAllowExternalDestroy() == false )
	//				isRemove = false;
	//		}

	//		if ( isRemove )
	//		{
	//			clearList.push_back( object );
	//			iter = mDelayStateList.erase(iter);
	//		}else
	//		{
	//			iter++;
	//		}
	//	}

	//	//////////////////////////////////////////////////////////////

	//	for( UInt i = 0; i < clearList.size() ; i++ )
	//	{
	//		LinedStateObject* object = clearList[i];

	//		object->notifyLeave();

	//		destroyState(object);
	//	}
	//}


}