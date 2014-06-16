/******************************************************************************/
#ifndef _SINGLETON_H_
#define _SINGLETON_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
    /******************************************************************************/
	//静态用单件类
	/******************************************************************************/
    #define SINGLETON_INSTANCE(c) public: static c& getInstance(){static c ins;return ins;}


	/******************************************************************************/
	//继承用单件类
	/******************************************************************************/
    template <typename A> class MGSingleton
    {
	private:
		/** \brief Explicit private copy constructor. This is a forbidden operation.*/
		MGSingleton(const MGSingleton<A> &);

		/** \brief Private operator= . This is a forbidden operation. */
		MGSingleton& operator=(const MGSingleton<A> &);
    
	protected:

        static A* ms_Singleton;

    public:
        MGSingleton( void )
        {
            assert( !ms_Singleton );
#if defined( _MSC_VER ) && _MSC_VER < 1200	 
            int offset = (int)(A*)1 - (int)(Singleton <A>*)(A*)1;
            ms_Singleton = (A*)((int)this + offset);
#else
	    ms_Singleton = static_cast< A* >( this );
#endif
        }
        ~MGSingleton( void )
            {  assert( ms_Singleton );  ms_Singleton = 0;  }
        static A& getSingleton( void )
		{	assert( ms_Singleton );  return ( *ms_Singleton ); }
        static A* getSingletonPtr( void )
		{ return ms_Singleton; }
    };
	/** @} */
	/** @} */

    /** 单件类声明 */
    #define SINGLETON_DECLARE(c) public: static c& getSingleton(void);\
        public: static c* getSingletonPtr(void);
    /** 单件类定义 */
    #define SINGLETON_DEFINE(c) template<> c* MGSingleton<c>::ms_Singleton = 0;\
        c* c::getSingletonPtr(void)\
        {\
            return ms_Singleton;\
        }\
        c& c::getSingleton(void)\
        {\
            assert( ms_Singleton );  return ( *ms_Singleton );\
        }
	
}

/******************************************************************************/

#endif