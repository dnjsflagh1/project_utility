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
	//����״̬ģʽ����
	/******************************************************************************/
	class LinedStateManager;
	class LinedStateObject
	{
		friend class LinedStateManager;
	public:
		LinedStateObject( Str name );
		virtual ~LinedStateObject();

		////////////////////////////////////////////////////////////////////

		///�õ�����
		Str                 getName();

		///�Ƿ��й���������
		Bool                isManagedLifeTime();
		///�����Ƿ��й���������
		void                setManagedLifeTime(Bool enable);

		// �õ�ʹ������
		U32                 getUsedCount();
		// ����ʹ������
		void                addUsedCount();

		////////////////////////////////////////////////////////////////////

		void				setDelayPushTime( Flt delayPushTime );



		////////////////////////////////////////////////////////////////////

		// ���Ӻ��Ƴ�������
		void                addListener(LinedStateObjectListener* listener);
		void                removeListener(LinedStateObjectListener* listener);
		void                clearAllListener();

		////////////////////////////////////////////////////////////////////

		///������һ��״̬����
		void                addNextState( LinedStateObject* object );
		///ɾ����һ��״̬����
		void                removeNextState( Str name, Bool isErgodic );
		///�õ���һ��״̬,���ݲ������ֺ��Ƿ����
		LinedStateObject*   getNextState( Str name, Bool isErgodic );

		////////////////////////////////////////////////////////////////////

		// �Ƿ������ⲿ������ɾ���Լ�
		void				setAllowExternalDestroy( Bool allow ){mAllowExternalDestroy=allow;};
		// �Ƿ������ⲿ������ɾ���Լ�
		Bool				getAllowExternalDestroy(){return mAllowExternalDestroy;};

		////////////////////////////////////////////////////////////////////

		///����
		void                destroy();
		///�Ƿ�����
		Bool                isDestroy();

		////////////////////////////////////////////////////////////////////

	protected:

		///֪ͨ��ʼ
		void                notifyEnter();
		///֪ͨ����
		void                notifyLeave();
		///֪ͨ�ı�
		void                notifyChanged();
		///֪ͨ����
		void                notifyLifeTimeOver();

		////////////////////////////////////////////////////////////////////

		///�����¼�
		virtual void        update( Flt delta ) = NULL;
		virtual void        draw(){};
		///�Ƿ������
		virtual Bool        isFinished() = NULL;

		///��ȥ״̬�¼�
		virtual void        onEnter() = NULL;
		///�뿪״̬�¼�
		virtual void        onLeave() = NULL;

        /// ֹͣ����
        void                stopUpdate( Flt time );
        /// ׼������
        void                preUpdate( Flt delta );

		///�����һ��״̬�����б�
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
	//����״̬ģʽ����
	//�ڲ�������ֶ��ͬ����״̬
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
		///����һ�������״̬����
		void										push(LinedStateObject* object, Bool isClearAll=true);

		///�������״̬�����б�
		void										clear(Bool isCheck=false);

		///���µ�ǰ״̬���󼯺�
		void										update( Flt delta );

		///�õ���һ��״̬,���ݲ������ֺ��Ƿ����
		LinedStateObject*							getState( Str name, Bool isErgodic );

		///ɾ����һ��״̬����
		void                						removeState( Str name, Bool isErgodic );

		///ɾ��״̬����
		void										destroyState( LinedStateObject* object );

        // �Ƿ�Ϊ��
        Bool                						isEmpty();

		////////////////////////////////////////////////////////////////////

		// ���Ӻ��Ƴ�������
		void                						addListener(LinedStateManagerListener* listener);
		void                						removeListener(LinedStateManagerListener* listener);
		void                						clearAllListener();

		////////////////////////////////////////////////////////////////////

		// �����Ƿ����push
		void										setIsPush( Bool isPush );

	protected:

		//void										push(LinedStateObject* object, LinedStateObject* ignoreObj);

		//void										clear(LinedStateObject* ignoreObj, Bool isCheck=false);

		///֪ͨ����
		void                						notifyAdd(LinedStateObject* obj);
		///֪ͨ�Ƴ�
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