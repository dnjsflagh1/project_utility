/******************************************************************************/
#ifndef _RAMALLOC_H_
#define _RAMALLOC_H_
/******************************************************************************/

#include "Singleton.h"
#include <cstddef>

/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//�ַ� ����
	/******************************************************************************/
	class RamAlloc
	{
    public:
        SINGLETON_INSTANCE(RamAlloc);
		RamAlloc();
		virtual ~RamAlloc();

		///allocate memory
		void*	get(int size, Bool isClear=true );
		///release memory
        void	release(void* buff);
		void	releaseRef(void*& buff);

	private:
        
	};
}

/******************************************************************************/

#endif