/******************************************************************************/
#ifndef LINKLIST_H
#define LINKLIST_H
/******************************************************************************/

/******************************************************************************/
namespace MG
{

    template < class Type >
    class MGLinkList
    {
    public:
        MGLinkList();
        ~MGLinkList();

        bool			IsListEmpty() const;
        bool			InList() const;
        I32				Count() const;
        void			Clear();

        void			InsertBefore( MGLinkList& node );
        void			InsertAfter( MGLinkList& node );
        void			AddToEnd( MGLinkList& node );
        void			AddToFront( MGLinkList& node );

        void			Remove();

        Type*			Next() const;
        Type*			Prev() const;

        Type*			Owner() const;
        void			SetOwner( Type* object );

        MGLinkList*		ListHead() const;
        MGLinkList*		NextNode() const;
        MGLinkList*		PrevNode() const;

    private:
        MGLinkList*		mHead;
        MGLinkList*		mNext;
        MGLinkList*		mPrev;

        Type*			mOwner;
    };

    // ============
    //
    // MGLinkList�ڵ�Ĺ��캯��
    //
    // ============
    template < class Type >
    MGLinkList<Type>::MGLinkList( )
    {
        mOwner = NULL;
        mHead  = this;
        mNext  = this;
        mPrev  = this;
    }

    // ============
    //
    // һ��MGLinkList�ڵ��������������Clear�Ĺ���һ��
    //
    // ============

    template < class Type >
    MGLinkList<Type>::~MGLinkList()
    {
        Clear();
    }

    // ============
    //
    // �ж�һ��MGLinkList�Ƿ�Ϊ��
    //
    // ============

    template < class Type >
    bool MGLinkList<Type>::IsListEmpty() const
    {
        return mHead->mNext == mHead;
    }

    // ============
    //
    // �жϵ�ǰ�Ľڵ��Ƿ��Ѿ���һ��MGLinkList��
    //
    // ============

    template < class Type >
    bool MGLinkList<Type>::InList() const
    {
        return mHead != this;
    }

    // ============
    //
    // ���һ��MGLinkList�еĽڵ�����
    //
    // ============

    template < class Type >
    I32 MGLinkList<Type>::Count() const
    {
        MGLinkList<Type>	*node;
        int					count;

        count = 0;

        for ( node = mHead->mNext; node != mHead; node = node->mNext )
        {
            count++;
        }

        return count;
    }

    // ============
    //
    // �����ǰ�Ľڵ�Ϊͷ�ڵ�(head)�������List������ֻ�ǰѵ�ǰ�ڵ��List���Ƴ�
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::Clear()
    {
        if ( mHead == this )
        {
            while ( mNext != this )
            {
                mNext->Remove();
            }
        }
        else
        {
            Remove();
        }
    }

    // ============
    //
    // ����ǰ�ڵ���뵽�����Ľڵ�֮ǰ
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::InsertBefore( MGLinkList& node )
    {
        Remove();

        mNext = &node;
        mPrev = node.mPrev;
        mHead = node.mHead;

        node.mPrev = this;
        mPrev->mNext = this;
    }

    // ============
    //
    // ����ǰ�ڵ���뵽�����Ľڵ�֮��
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::InsertAfter( MGLinkList& node )
    {
        Remove();

        mNext = node.mNext;
        mPrev = &node;
        mHead = node.mHead;

        node.mNext = this;
        mNext->mPrev = this;
    }

    // ============
    //
    // ����ǰ�ڵ���뵽�����Ľڵ�List�����
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::AddToEnd( MGLinkList& node )
    {
        Remove();

        InsertBefore( *node.mHead );
    }

    // ============
    //
    // ����ǰ�ڵ���뵽�����Ľڵ�List����ǰ��
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::AddToFront( MGLinkList& node )
    {
        Remove();

        InsertAfter( *node.mHead );
    }

    // ============
    //
    // ����ǰ�ڵ��List���Ƴ�
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::Remove()
    {
        mPrev->mNext = mNext;
        mNext->mPrev = mPrev;

        mPrev = this;
        mNext = this;
        mHead = this;
    }

    // ============
    //
    // ������һ���ڵ������
    //
    // ============

    template < class Type >
    Type* MGLinkList<Type>::Next() const
    {
        if ( !mNext || mNext == mHead )
        {
            return NULL;
        }

        return mNext->mOwner;
    }

    // ============
    //
    // ������һ���ڵ������
    //
    // ============

    template < class Type >
    Type* MGLinkList<Type>::Prev() const
    {
        if ( !mPrev || mPrev == mHead )
        {
            return NULL;
        }

        return mPrev->mOwner;
    }

    // ============
    //
    // ���ص�ǰ�ڵ������
    //
    // ============
    template < class Type >
    Type* MGLinkList<Type>::Owner() const
    {
        return mOwner;
    }

    // ============
    //
    // ���õ�ǰ�ڵ������
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::SetOwner( Type* object )
    {
        mOwner = object;
    }

    // ============
    //
    // ����ͷ�ڵ�
    //
    // ============

    template < class Type >
    MGLinkList<Type>* MGLinkList<Type>::ListHead() const
    {
        return mHead;
    }

    // ============
    //
    // ������һ���ڵ�
    //
    // ============

    template < class Type >
    MGLinkList<Type>* MGLinkList<Type>::NextNode() const
    {
        if ( mNext == mHead )
        {
            return NULL;
        }
        else
        {
            return mNext;
        }
    }

    // ============
    //
    // ������һ���ڵ�
    //
    // ============

    template < class Type>
    MGLinkList<Type>* MGLinkList<Type>::PrevNode() const
    {
        if ( mPrev == mHead )
        {
            return NULL;
        }
        else
        {
            return mPrev;
        }
    }

}

#endif // LINKLIST_H
