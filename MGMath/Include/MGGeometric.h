/******************************************************************************/
#ifndef _MGGEOMETRIC_H_
#define _MGGEOMETRIC_H_
/******************************************************************************/

#include "MGVector.h"

/******************************************************************************/

namespace MG
{
	#define		MG_PI							3.141593f
	#define		MG_PI_2							1.570797f
    #define		COMPARE_NUMBER(n1,n2)   ( fabs(n1-n2) < 1.0E-10 )
    #define		COMPARE_FLOAT(n1,n2)    ( fabs( (float)( (n1)-(n2) ) ) < (float)(1.0E-6) )
	#define		MG_LERP(a,b,fraction) ( a * ( 1 - fraction ) + b * fraction )
	
	//get floor
	inline U32 MG_Floor(Flt flt)
	{
		if ( flt > 0 ) 
			return int(flt);
		else 
			return (int)(flt - 1);
	}

    class MGMath
    {

	public:

		//triangle functions

		/** Arc tangent function
		@param
			fValue The value whose arc tangent will be returned.
		*/
		static inline Flt ATan (Flt fValue) { return Flt(atan(fValue)); }

		/** Arc tangent between two values function
		@param
			fY The first value to calculate the arc tangent with.
		@param
			fX The second value to calculate the arc tangent with.
		*/
		static inline Flt ATan2 (Flt fY, Flt fX) { return Flt(atan2(fY,fX)); }

		/** Arc sine function
		@param
			fValue The value whose arc sine will be returned.
		*/
		static Flt ASin (Flt fValue);

		/** Arc cosine function
		@param
		fValue The value whose arc cosine will be returned.
		*/
		static Flt ACos (Flt fValue);

    public:

		//比较两个float的大小。
		static MG::Int MG_compareFloat( Flt a, Flt b );
		
        //get a perm  
        static I32 getPerm_2d( U32 x, U32 y );

        //get random normals
        static Vec3 getGradients_2d( U32 x, U32 y );

        //a noise smooth function 
        static Dbl Fade(float t);

        //a noise generator
        static Flt Noise1(I32 x, I32 y);

        //a smooth noise generator
        static Flt SmoothNoise_1(I32 x, I32 y);

        //a fixed interpolate function version which use cosine 
        static Dbl Cosine_Interpolate(Flt a, Flt b, Flt x);

        //a interpolate noise generator
        static Flt InterpolatedNoise_1(Flt x, Flt y);

        //a other version for classic PerlinNoise implementation
        static Flt PerlinNoise_2D(Flt x, Flt y, Flt persistence, I32 octaves);

        //my interpolate noise generator
        static Dbl MG_InterpolatedNoise_2D(Flt x, Flt y);

        //a classic PerlinNoise implementation
        static Flt MG_PerlinNoise_2D(Flt x, Flt y, Flt persistence, I32 octaves);

        static Flt getCosBetweenTwoVector( Vec3 vec1, Vec3 vec2 );

        static Bool isTwoVectorCoincide( const Vec3& vec1, const Vec3& vec2, Flt tolerance  = 0.01  );

        static Bool isTwoVectorCoincide( Flt cos, Flt tolerance  = 0.01 );

        static Vec3 getNormaliseVerticalVector( const Vec3& vector, Vec3 referPlaneVector1 = Vec3(1,0,0), Vec3 referPlaneVector2  = Vec3(0,0,1) );

        static Flt  grtVectorIncludedAngle( Vec3 vec1, Vec3 vec2 );

        static Vec3 rotateVector( Vec3 vec, Flt angle, Vec3 axis = Vec3(0, 1, 0 ) );

        static Vec3 normaliseVector( Vec3 vec );
    };
	
}
/******************************************************************************/
#endif