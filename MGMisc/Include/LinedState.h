/******************************************************************************/
#ifndef _LINEDSTATE_H_
#define _LINEDSTATE_H_
/******************************************************************************/

/******************************************************************************/

namespace MG
{
	class LinedStateObject;
	class LinedStateObjectListener
	{
	public:
		virtual void    onLinedStateEnter(LinedStateObject* obj){UNREFERENCED_PARAMETER(obj);};
		virtual void    onLinedStateLeave(LinedStateObject* obj){UNREFERENCED_PARAMETER(obj);};
		virtual void    onLinedStateChanged(LinedStateObject* obj){UNREFERENCED_PARAMETER(obj);};
		virtual void    onLinedStateLifeTimeOver(LinedStateObject* obj){UNREFERENCED_PARAMETER(obj);};
	};

	/******************************************************************************/
	//线性状态模式对象
	/******************************************************************************/
	class LinedStateManager;
	class LinedStateObject
	{
		friend class LinedStateManager;
	public:
		LinedStateObject( Str name );
		virtual ~LinedStateObject();

		////////////////////////////////////////////////////////////////////

		///得到名字
		Str                 getName();

		///是否托管生命周期
		Bool                isManagedLifeTime();
		///设置是否托管生命周期
		void                setManagedLifeTime(Bool enable);

		// 得到使用数量
		U32                 getUsedCount();
		// 增加使用数量
		void                addUsedCount();

		////////////////////////////////////////////////////////////////////

		void				setDelayPushTime( Flt delayPushTime );



		////////////////////////////////////////////////////////////////////

		// 增加和移除监听器
		void                addListener(LinedStateObjectListener* listener);
		void                removeListener(LinedStateObjectListener* listener);
		void                clearAllListener();

		////////////////////////////////////////////////////////////////////

		///增加下一个状态对象
		void                addNextState( LinedStateObject* object );
		///删除下一个状态对象
		void                removeNextState( Str name, Bool isErgodic );
		///得到下一个状态,根据参数名字和是否遍历
		LinedStateObject*   getNextState( Str name, Bool isErgodic );

		////////////////////////////////////////////////////////////////////

		// 是否允许外部管理器删除自己
		void				setAllowExternalDestroy( Bool allow ){mAllowExternalDestroy=allow;};
		// 是否允许外部管理器删除自己
		Bool				getAllowExternalDestroy(){return mAllowExternalDestroy;};

		////////////////////////////////////////////////////////////////////

		///销毁
		void                destroy();
		///是否销毁
		Bool                isDestroy();

		////////////////////////////////////////////////////////////////////

	protected:

		///通知开始
		void                notifyEnter();
		///通知结束
		void                notifyLeave();
		///通知改变
		void                notifyChanged();
		///通知结束
		void                notifyLifeTimeOver();

		////////////////////////////////////////////////////////////////////

		///更新事件
		virtual void        update( Flt delta ) = NULL;
		virtual void        draw(){};
		///是否结束了
		virtual Bool        isFinished() = NULL;

		///进去状态事件
		virtual void        onEnter() = NULL;
		///离开状态事件
		virtual void        onLeave() = NULL;

        /// 停止更新
        void                stopUpdate( Flt time );
        /// 准备更新
        void                preUpdate( Flt delta );

		///清空下一个状态对象列表
		void                clearNextStataList( Bool isCheck = false );

	protected:

		Str mName;
		Bool mIsDestroy;
		Bool mManagedLifeTime;
		Bool mAllowExternalDestroy;
		std::list<LinedStateObject*> mStateList;
		U32 mUsedCount;
		std::list<LinedStateObjectListener*>    mListener;
		Flt mDelayPushTime;
        Flt mStopTime;
	};


	/******************************************************************************/
	//线性状态模式管理
	//内部不会出现多个同名的状态
	/******************************************************************************/

	class LinedStateManagerListener
	{
	public:
		virtual void    							onLinedStateAdd(LinedStateObject* obj){UNREFERENCED_PARAMETER(obj);};
		virtual void    							onLinedStateRemove(LinedStateObject* obj){UNREFERENCED_PARAMETER(obj);};
	};

	class LinedStateManager
	{
	public:
		LinedStateManager();
		virtual ~LinedStateManager();

	public:

		void										delayPush(LinedStateObject* object, Bool isClearAll=true);
		///增加一个激活的状态对象
		void										push(LinedStateObject* object, Bool isClearAll=true);

		///清空所有状态对象列表
		void										clear(Bool isCheck=false);

		///更新当前状态对象集合
		void										update( Flt delta );

		///得到下一个状态,根据参数名字和是否遍历
		LinedStateObject*							getState( Str name, Bool isErgodic );

		///删除下一个状态对象
		void                						removeState( Str name, Bool isErgodic );

		///删除状态对象
		void										destroyState( LinedStateObject* object );

        // 是否为空
        Bool                						isEmpty();

		////////////////////////////////////////////////////////////////////

		// 增加和移除监听器
		void                						addListener(LinedStateManagerListener* listener);
		void                						removeListener(LinedStateManagerListener* listener);
		void                						clearAllListener();

		////////////////////////////////////////////////////////////////////

		// 设置是否可以push
		void										setIsPush( Bool isPush );

	protected:

		//void										push(LinedStateObject* object, LinedStateObject* ignoreObj);

		//void										clear(LinedStateObject* ignoreObj, Bool isCheck=false);

		///通知增加
		void                						notifyAdd(LinedStateObject* obj);
		///通知移除
		void                						notifyRemove(LinedStateObject* obj);

	protected:

		std::list<LinedStateObject*>				mStateList;
		std::list<LinedStateObject*>				mDelayStateList;
		std::list<LinedStateManagerListener*>		mListener;
		Bool										mIsPush;

	};

}


/******************************************************************************/

#endif //