/******************************************************************************/
#include "stdafx.h"
#include "SharedPtr.h"
/******************************************************************************/

namespace MG
{
	InterLocked TestSharePtr::useCountCount = 0;
	InterLocked TestSharePtr::SharedPtrCount = 0;
	InterLocked TestSharePtr::sendBuffCount = 0;
	InterLocked TestSharePtr::sendConfigCount = 0;
	InterLocked TestSharePtr::driverCount = 0;
}