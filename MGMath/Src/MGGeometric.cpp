/******************************************************************************/
#include "stdafx.h"
#include "MGGeometric.h"
#include "MGQuaternion.h"
/******************************************************************************/

namespace MG
{

	/******************************************************************************/
	
	/* These are Ken Perlin's proposed gradients for 3D noise. I kept them for
	better consistency with the reference implementation, but there is really
	no need to pad this to 16 gradients for this particular implementation.
	If only the "proper" first 12 gradients are used, they can be extracted
	from the grad4[][] array: grad3[i][j] == grad4[i*2][j], 0<=i<=11, j=0,1,2 */

	U8 perm[256] = {151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};		
	
	I32 grad3[16][3] = {{0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
	{1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
	{1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0}, // 12 cube edges
	{1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}}; // 4 more to make 16

	Dbl grad2[12][2] = { {1.0, 0},{-1.0, 0}, {0.8660254, 0.5},{-0.8660254, -0.5},
		{0.5, 0.8660254},{-0.5, -0.8660254}, {0.0, 1.0},	  {0.0, -1.0}, // equally divide a circle into 12 parts
		{-0.5, 0.8660254},{0.5, -0.8660254}, {-0.8660254, 0.5},{0.8660254, -0.5} };  // 4 more to make 16 

	/******************************************************************************/

	Flt MGMath::ACos (Flt fValue)
	{
		if ( -1.0 < fValue )
		{
			if ( fValue < 1.0 )
				return Flt(acos(fValue));
			else
				return Flt(0.0);
		}
		else
		{
			return Flt(MG_PI);
		}
	}
	//-----------------------------------------------------------------------
	Flt MGMath::ASin (Flt fValue)
	{
		if ( -1.0 < fValue )
		{
			if ( fValue < 1.0 )
				return Flt(asin(fValue));
			else
				return Flt(MG_PI_2);
		}
		else
		{
			return Flt(-MG_PI_2);
		}
	}
	//-----------------------------------------------------------------------
    I32 MGMath::getPerm_2d( U32 x, U32 y )
	{
		return perm[( y + perm [x & 0xFF] ) & 0xFF];
	}
	//-----------------------------------------------------------------------------
	Vec3 MGMath::getGradients_2d( U32 x, U32 y )
	{
		U8 perm_value = perm[(y+perm[x & 0xFF]) & 0xFF];

		Vec3 result;
		result.x = (Flt)grad3[perm_value & 0x0F][0] * 64 + 64;
		result.y = (Flt)grad3[perm_value & 0x0F][1] * 64 + 64;
		result.z = (Flt)grad3[perm_value & 0x0F][2] * 64 + 64;

		return result;
	}
	//-----------------------------------------------------------------------------
	Dbl MGMath::Fade(float t) 
	{
		// return t*t*(3.0-2.0*t); // Old fade, yields discontinuous second derivative
		return t*t*t*(t*(t*6.0-15.0)+10.0); // Improved fade, yields C2-continuous noise
	}
	//-----------------------------------------------------------------------------
	Flt MGMath::Noise1(I32 x, I32 y)
	{
		x = x % 25;
		y = y % 25;
		int n = x + y * 57;
		n = (n<<13) ^ n;
		return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f); 
	}
	//-----------------------------------------------------------------------------
	Flt MGMath::SmoothNoise_1(I32 x, I32 y)
	{
		Flt corners = ( Noise1(x-1, y-1)+Noise1(x+1, y-1)+Noise1(x-1, y+1)+Noise1(x+1, y+1) ) / 16.0f;
		Flt sides   = ( Noise1(x-1, y)  +Noise1(x+1, y)  +Noise1(x, y-1)  +Noise1(x, y+1) ) /  8.0f;
		Flt center  =  Noise1(x, y) / 4.0f;
		return corners + sides + center;
	}
	//-----------------------------------------------------------------------------
	Dbl MGMath::Cosine_Interpolate(Flt a, Flt b, Flt x)
	{
		Dbl ft = x * MG_PI;
		Dbl f = (1 - cos(ft)) * 0.5f;

		return  a * ( 1 - f ) + b * f;
	}
	//-----------------------------------------------------------------------------
	Flt MGMath::InterpolatedNoise_1(Flt x, Flt y)
	{
		I32 integer_X    = I32(x);
		Flt fractional_X = x - integer_X;

		I32 integer_Y    = I32(y);
		Flt fractional_Y = y - integer_Y;

		Flt v1 = SmoothNoise_1(integer_X,     integer_Y);
		Flt v2 = SmoothNoise_1(integer_X + 1, integer_Y);
		Flt v3 = SmoothNoise_1(integer_X,     integer_Y + 1);
		Flt v4 = SmoothNoise_1(integer_X + 1, integer_Y + 1);

		Flt i1 = (Flt)Cosine_Interpolate(v1 , v2 , fractional_X);
		Flt i2 = (Flt)Cosine_Interpolate(v3 , v4 , fractional_X);

		return (Flt)Cosine_Interpolate(i1 , i2 , fractional_Y);
	}
	//-----------------------------------------------------------------------------
	Dbl MGMath::MG_InterpolatedNoise_2D(Flt x, Flt y)
	{
		I32 integer_X	= MG_Floor(x);
		Flt fractional_X = x - integer_X;

		I32 integer_Y    = MG_Floor(y);
		Flt fractional_Y = y - integer_Y;

		// Wrap the integer cells at 255 (smaller integer period can be introduced here)
		integer_X = integer_X & 0xFF;
		integer_Y = integer_Y & 0xFF;

		// Calculate a set of eight hashed gradient indices  
		I32 g00_index = perm[(integer_X + perm[integer_Y]) & 0xFF] % 12;
		I32 g10_index = perm[((integer_X + 1) + perm[integer_Y]) & 0xFF] % 12;
		I32 g01_index = perm[(integer_X + perm[integer_Y + 1]) & 0xFF] % 12;
		I32 g11_index = perm[((integer_X + 1) + perm[integer_Y + 1]) & 0xFF] % 12;

		Dbl n00	= grad2[g00_index][0] * fractional_X + grad2[g00_index][1] * fractional_Y;
		Dbl n10	= grad2[g10_index][0] * (fractional_X - 1) + grad2[g10_index][1] * fractional_Y;
		Dbl n01	= grad2[g01_index][0] * fractional_X + grad2[g01_index][1] * (fractional_Y - 1);
		Dbl n11	= grad2[g11_index][0] * (fractional_X - 1) + grad2[g11_index][1] * (fractional_Y - 1);

		Dbl	fade_x = Fade(fractional_X);
		Dbl	fade_y = Fade(fractional_Y);

		Dbl mix_x_0 = MG_LERP( n00, n10, fade_x );
		Dbl mix_x_1 = MG_LERP( n01, n11, fade_x );

		Dbl mix_final = MG_LERP( mix_x_0, mix_x_1, fade_y );

		return mix_final;
	}
	//-----------------------------------------------------------------------------
	Flt MGMath::PerlinNoise_2D(Flt x, Flt y, Flt persistence, I32 octaves)
	{
		Flt total = 0.0f;
		Flt p = persistence;
		I32 n = octaves - 1;

		for(int i=0;i<=n;i++)
		{
			float frequency = pow((float)2,i);
			float amplitude = pow(p,i);

			total = total + InterpolatedNoise_1(x * frequency, y * frequency) * amplitude;
		}

		return total;
	} 
	//-----------------------------------------------------------------------------
	Flt MGMath::MG_PerlinNoise_2D(Flt x, Flt y, Flt persistence, I32 octaves)
	{
		Flt total = 0.0f;
		Flt p = persistence;
		I32 n = octaves - 1;

		for(int i=0;i<=n;i++)
		{
			float frequency = pow((float)2,i);
			float amplitude = pow(p,i);

			total = total + (Flt)MG_InterpolatedNoise_2D(x * frequency, y * frequency) * amplitude;
		}

		return total;
	}
	//-----------------------------------------------------------------------------
	Flt MGMath::getCosBetweenTwoVector( Vec3 vec1, Vec3 vec2 )
	{
		vec1.normalise();
		vec2.normalise();

		return vec1.dotProduct( vec2 );
	}
	//-----------------------------------------------------------------------------
	Bool MGMath::isTwoVectorCoincide( const Vec3& vec1, const Vec3& vec2, Flt tolerance )
	{
		Flt cos = getCosBetweenTwoVector( vec1, vec2 );
		return isTwoVectorCoincide( cos, tolerance );
	}
	//-----------------------------------------------------------------------
	Bool MGMath::isTwoVectorCoincide( Flt cos, Flt tolerance )
	{
		Flt tcos;
		tcos = abs( 1 - cos );
		if ( tcos < tolerance )
			return true;

		tcos = abs( 1 + cos );
		if ( tcos < tolerance )
			return true;

		return false;
	}
	//-----------------------------------------------------------------------
	Vec3 MGMath::getNormaliseVerticalVector( const Vec3& vector, Vec3 referPlaneVector1, Vec3 referPlaneVector2 )
	{
		Vec3 result;

		if ( isTwoVectorCoincide( referPlaneVector1, referPlaneVector2) )
		{
			return vector;
		}

		Vec3 rpv = referPlaneVector1;
		if ( isTwoVectorCoincide( vector, referPlaneVector1) )
			rpv = referPlaneVector2;

		result = rpv.crossProduct( vector );
		result = result.crossProduct( vector );

		result.normalise();

		return result;
	}

	//-------------------------------------------------------
	Vec3 MGMath::rotateVector( Vec3 vec, Flt angle, Vec3 axis )
	{
		//Radian rotateAngle = Radian(angle);
		//Vec3 vector = Vec3(vec.x, vec.y, vec.z);
		//Vector3 axisVector =  Vec3(axis.x, axis.y, axis.z);
		Qua q;
		q.FromAngleAxis(angle,axis);
		vec = q * vec;

		return vec;
	}

	//-------------------------------------------------------
	Flt MGMath::grtVectorIncludedAngle( Vec3 vec1, Vec3 vec2 )
	{
		//Vector3 vector1 = Vector3(vec1.x, vec1.y, vec1.z);
		//Vector3 vector2 =  Vector3(vec2.x, vec2.y, vec2.z);
		Flt angle = vec1.dotProduct(vec2);

		return angle;
	}

	//-------------------------------------------------------
	Vec3 MGMath::normaliseVector( Vec3 vec )
	{
		vec.normalise();

		return vec;
	}
	//-------------------------------------------------------
	MG::Int MGMath::MG_compareFloat( Flt a, Flt b )
	{
		Flt c = a-b;
		if (c > 0.000001)
			return 1;
		else if (c < -0.000001)
			return -1;
		else
			return 0;
	}
}
/******************************************************************************/