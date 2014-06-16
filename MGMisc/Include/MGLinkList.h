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
    // MGLinkList节点的构造函数
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
    // 一个MGLinkList节点的析构函数，和Clear的功能一样
    //
    // ============

    template < class Type >
    MGLinkList<Type>::~MGLinkList()
    {
        Clear();
    }

    // ============
    //
    // 判断一个MGLinkList是否为空
    //
    // ============

    template < class Type >
    bool MGLinkList<Type>::IsListEmpty() const
    {
        return mHead->mNext == mHead;
    }

    // ============
    //
    // 判断当前的节点是否已经在一个MGLinkList中
    //
    // ============

    template < class Type >
    bool MGLinkList<Type>::InList() const
    {
        return mHead != this;
    }

    // ============
    //
    // 获得一个MGLinkList中的节点数量
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
    // 如果当前的节点为头节点(head)清空整个List，否则只是把当前节点从List中移除
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
    // 将当前节点插入到给定的节点之前
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
    // 将当前节点插入到给定的节点之后
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
    // 将当前节点插入到给定的节点List的最后
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
    // 将当前节点插入到给定的节点List的最前面
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
    // 将当前节点从List中移除
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
    // 返回下一个节点的内容
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
    // 返回上一个节点的内容
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
    // 返回当前节点的内容
    //
    // ============
    template < class Type >
    Type* MGLinkList<Type>::Owner() const
    {
        return mOwner;
    }

    // ============
    //
    // 设置当前节点的内容
    //
    // ============

    template < class Type >
    void MGLinkList<Type>::SetOwner( Type* object )
    {
        mOwner = object;
    }

    // ============
    //
    // 返回头节点
    //
    // ============

    template < class Type >
    MGLinkList<Type>* MGLinkList<Type>::ListHead() const
    {
        return mHead;
    }

    // ============
    //
    // 返回下一个节点
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
    // 返回上一个节点
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
