/******************************************************************************/
#ifndef __STLALLOCATOR_H__
#define __STLALLOCATOR_H__
/******************************************************************************/

#include <ObjectPool.h>
#include <new>
#include <limits>
using namespace std;

/******************************************************************************/
 //����׼�����ṩ�Զ���ռ������ map,list....��ò�Ҫ��vectorʹ��,vector�����ڴ�
//�������̶� ����Ҳ���̶���
//������
/******************************************************************************/
namespace MG
{
    #define STLALLOC_CATEGORY 3001


    /******************************************************************************/
    // STL�ڴ������
    /******************************************************************************/
	template < class T >
	class StlAllocator
	{
	public:

	    /****
	        ����ĸ�����Ͳ������� ��ϣ����������ֱ�ӱ���׼����ʹ��
	    ***/
	    //type define
	    typedef size_t         size_type;
	    typedef ptrdiff_t      difference_type;
	    typedef T*             pointer;
	    typedef const T*       const_pointer;
	    typedef T&             reference;
	    typedef const T&       const_reference;
	    typedef T              value_type;

        //--------------------------------------------------------------------------
	    //rebind
	    template<class U>
	    struct rebind
	    {
		    typedef  StlAllocator<U> other;
	    };

        //--------------------------------------------------------------------------
	    //fuction
	    pointer address(reference value) const
	    {
		    return &value;
	    }

        //--------------------------------------------------------------------------
	    const_pointer address(const_reference value) const
	    {
		    return &value;
	    }

        //--------------------------------------------------------------------------
	    StlAllocator() throw()
	    {

	    }

        //--------------------------------------------------------------------------
	    StlAllocator(const StlAllocator&) throw()
	    {

	    }

        //--------------------------------------------------------------------------
	    template<class U>
	    StlAllocator(const StlAllocator<U>&) throw()
	    {

	    }

        //--------------------------------------------------------------------------
	    ~StlAllocator() throw()
	    {

	    }

        //--------------------------------------------------------------------------
	    //���ܱ���������Ԫ������
	    size_type max_size() const throw()
	    {
		    return std::numeric_limits<size_t>::max();
	    }

        //--------------------------------------------------------------------------
	    pointer allocate(size_type num,const void* point = 0)
	    {
		    /***
		    ʹ���Լ����ڴ���似��
		    ****/
		    return MG_POOL_ALLOC_T(T,num,STLALLOC_CATEGORY);
	    }

        //--------------------------------------------------------------------------
	    void construct(pointer p,const T& value)
	    {

		    /****
		    ʹ��placement new ִ�й��������캯�� ��ʼ������
		    ****/
		    new((void*)p) T(value);
	    }

        //--------------------------------------------------------------------------
	    void destroy(pointer p)
	    {
		    p->~T();
	    }

        //--------------------------------------------------------------------------
	    void deallocate(pointer p,size_type num)
	    {
		    MG_POOL_DELETE_T(p,T,STLALLOC_CATEGORY);
	    }       
    };

    /******************************************************************************/
    // STL�ڴ������
    /******************************************************************************/
    template< class T >
    class StlSoloAllocator
    {
    public:

        /****
            ����ĸ�����Ͳ������� ��ϣ����������ֱ�ӱ���׼����ʹ��
        ***/
        //type define
        typedef size_t         size_type;
        typedef ptrdiff_t      difference_type;
        typedef T*             pointer;
        typedef const T*       const_pointer;
        typedef T&             reference;
        typedef const T&       const_reference;
        typedef T              value_type;

		//--------------------------------------------------------------------------

		// �ڲ�׼���ж����б�
		std::deque<pointer>     m_IdleObjectList;
		size_t					m_TypeSize;

        //--------------------------------------------------------------------------
        //rebind
        template<class U>
        struct rebind
        {
            typedef  StlSoloAllocator<U> other;
        };

        //--------------------------------------------------------------------------
        //fuction
        pointer address(reference value) const
        {
            return &value;
        }

        //--------------------------------------------------------------------------
        const_pointer address(const_reference value) const
        {
            return &value;
        }

        //--------------------------------------------------------------------------
        StlSoloAllocator() throw()
        {
			m_TypeSize = sizeof(T);
        }

        //--------------------------------------------------------------------------
        StlSoloAllocator(const StlSoloAllocator&) throw()
        {
			m_TypeSize = sizeof(T);
        }

        //--------------------------------------------------------------------------
        template<class U>
        StlSoloAllocator(const StlSoloAllocator<U>&) throw()
        {
			m_TypeSize = sizeof(T);
        }

        //--------------------------------------------------------------------------
        ~StlSoloAllocator() throw()
        {
			// ���տ��еĶ����б�
			std::deque<pointer>::iterator ite;
			for(ite=m_IdleObjectList.begin(); ite!=m_IdleObjectList.end(); ++ite)
			{
				MG_POOL_FREE((*ite),STLALLOC_CATEGORY);
			}
			m_IdleObjectList.clear();
        }

        //--------------------------------------------------------------------------
        //���ܱ���������Ԫ������
        size_type max_size() const throw()
        {
            return std::numeric_limits<size_t>::max();
        }

        //--------------------------------------------------------------------------
        pointer allocate(size_type num,const void* point = 0)
        {
            UNREFERENCED_PARAMETER( point );

            /***
                ʹ���Լ����ڴ���似��
            ****/

            DYNAMIC_ASSERT( num == 1 );

			pointer pObject = NULL;

			if ( m_IdleObjectList.empty() == false )
			{
				pObject = m_IdleObjectList.back();
				m_IdleObjectList.pop_back();
			}else
			{
				pObject = (T*)MG_POOL_MALLOC( m_TypeSize, STLALLOC_CATEGORY );
			}

            return pObject;
        }

        //--------------------------------------------------------------------------
        void construct(pointer p,const T& value)
        {
            /****
                ʹ��placement new ִ�й��������캯�� ��ʼ������
            ****/
            new((void*)p) T(value);
        }
        
        //--------------------------------------------------------------------------
        void destroy(pointer p)
        {
            UNREFERENCED_PARAMETER( p );

            p->~T();
        }

        //--------------------------------------------------------------------------
        void deallocate(pointer p,size_type num)
        {
            DYNAMIC_ASSERT( num == 1 );

            m_IdleObjectList.push_back( p );
        }       
    };

    //--------------------------------------------------------------------------
	template <class T1, class T2>
	bool operator== (const StlAllocator<T1>&,
		const StlAllocator<T2>&) throw()
	{
		return true;
	}
	template <class T1, class T2>

	bool operator!= (const StlAllocator<T1>&,
		const StlAllocator<T2>&) throw()
	{
		return false;
	}

    //--------------------------------------------------------------------------
    template <class T1, class T2>
    bool operator== (const StlSoloAllocator<T1>&,
        const StlSoloAllocator<T2>&) throw()
    {
        return true;
    }
    template <class T1, class T2>

    bool operator!= (const StlSoloAllocator<T1>&,
        const StlSoloAllocator<T2>&) throw()
    {
        return false;
    }

}


/******************************************************************************/
#endif