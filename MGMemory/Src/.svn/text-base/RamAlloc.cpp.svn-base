/******************************************************************************/
#include "stdafx.h"
#include "RamAlloc.h"
#include "MsgBox.h"
#include <cstdlib>
#include <list>

/******************************************************************************/

namespace MG
{
   #undef new 
	//-----------------------------------------------------------------------------
	RamAlloc::RamAlloc()
	{
        
	}
	//-----------------------------------------------------------------------------
	RamAlloc::~RamAlloc()
	{
        
	}
	//-----------------------------------------------------------------------------
	void* RamAlloc::get( int size, Bool isClear )
	{
		void* p = malloc(size);
        if (!p)
        {
            MG_MESSAGE("RamAlloc::Get Error");
        }

        if ( isClear )
            memset(p,0,size);
		return p;
	}
	//-----------------------------------------------------------------------------
	void RamAlloc::release( void* buff )
	{
		free(buff);
	}
	//-----------------------------------------------------------------------------
    void RamAlloc::releaseRef(void*& buff)
    {
		release(buff);
        buff = NULL;
    }
    /******************************************************************************/
    //namespace
    //{
    //    //new出来的指针信息。
    //    struct Info
    //    {
    //        void* ptr;
    //        std::string file;
    //        long line;
    //    };
    //    std::list<Info> memList;
    //    std::size_t npos;
    //    //Search the list for an address;
    //    std::list<Info>::iterator findPtr(void* ptr)
    //    {
    //        std::list<Info>::iterator pos = memList.begin();
    //        while ( pos != memList.end() )
    //        {
    //            if ( (*pos).ptr == ptr )
    //                return pos;
    //            ++pos;
    //        }
    //        memList.end();
    //    }
    //    //delete ptr
    //    void deletePtr(void* ptr)
    //    {
    //        std::list<Info>::iterator pos = findPtr(ptr);
    //        assert( pos != memList.end() );
    //        memList.erase( pos );             
    //    }
    //    //Dummy type for static destructor
    //    struct Sentinel
    //    {
    //        ~Sentinel()
    //        {
    //            if ( memList.size() > 0 )
    //            {
    //                char temp[128] = "";
    //                OutputDebugStringA("Leaked memory at: ");
    //                std::list<Info>::iterator pos = memList.begin();
    //                while ( pos != memList.end() )
    //                {
    //                    sprintf(
    //                        temp, "\t%p (file: %s, line: %ld)",
    //                            (*pos).ptr,
    //                            (*pos).file.c_str(),
    //                            (*pos).line                    
    //                        );
    //                    OutputDebugStringA(temp);
    //                }
    //                OutputDebugStringA("Leaked memory end.");
    //            }
    //            else
    //                OutputDebugStringA("No Leaked memory.");
    //        }
    //    };
    //    Sentinel s;
    //}// end anonymous namepace
    ////overload scalar new 
    //void* operator new( std::size_t siz, const char* file, long line )
    //{
    //    void* p = malloc(siz);
    //    Info tempInfo;
    //    tempInfo.file = file;
    //    tempInfo.line = line;
    //    tempInfo.ptr = p;
    //    memList.push_back( tempInfo );
    //    return p;
    //}

    //void* operator new[]( std::size_t siz, const char* file, long line )
    //{
    //    return operator new(siz, file, line);
    //}
    ////overload scalar delete
    //void operator delete(void* p)
    //{
    //    deletePtr(p);
    //    free(p);
    //}

    //void operator delete[](void* p)
    //{
    //    operator delete(p);
    //}
}


