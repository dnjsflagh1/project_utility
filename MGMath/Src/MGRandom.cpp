//******************************************************************************************
#include "stdafx.h"
#include "MGRandom.h"
#include <time.h>
//******************************************************************************************
namespace MG
{
    MGRandom::MGRandom()
    {
        srand( (unsigned int)time(0) );
    }

    MGRandom::~MGRandom()
    {

    }

    U32 MGRandom::rand_ab_One( U32 min, U32 max )
    {
#if 0
        assert(a >= 0);
        assert(b > 0);
        assert(b < RAND_MAX);

        return U32( (Flt(b-a)+1.0)*rand()/(RAND_MAX+1.0) + a );
#endif
		DYNAMIC_ASSERT(min >= 0 && max > 0);
		DYNAMIC_ASSERT(max > min);

		double randNum = rand() / double(RAND_MAX);	//获取 0～1之间的浮点数
		U32 rd =  U32( (max - min) * randNum );
		U32 result = min + rd;

		return result;
    }	
}