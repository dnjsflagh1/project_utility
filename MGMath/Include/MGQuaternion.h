/******************************************************************************/
#ifndef _MGQua_H_
#define _MGQua_H_
/******************************************************************************/
#include "MGTypes.h"
#include "MGVector.h"

/******************************************************************************/
namespace MG 
{
#pragma pack( push )
#pragma pack( 1 )

#pragma pack (pop) 

	struct  Qua
	{
		Flt w;
		Flt x;
		Flt y;
		Flt z;
		Qua()
			:w(1)
			,x(0)
			,y(0)
			,z(0)
		{

		}

		Qua(const Flt& fw, const Flt& fx, const Flt& fy, const Flt& fz)
			:w(fw),
			x(fx),
			y(fy),
			z(fz)
		{

		}

		Qua(const Qua& A)
			:w(A.w),
			x(A.x),
			y(A.y),
			z(A.z)
		{

		}

		inline Qua& operator=(const Qua& A)
		{
			w = A.w;
			x = A.x;
			y = A.y;
			z = A.z;
			return *this;
		}

		inline bool operator== (const Qua& rhs) const
		{
			return (rhs.x == x) && (rhs.y == y) &&
				(rhs.z == z) && (rhs.w == w);
		}

		inline bool operator!= (const Qua& rhs) const
		{
			return !operator==(rhs);
		}

		Qua operator+ (const Qua& rkQ) const
		{
			return Qua(w+rkQ.w,x+rkQ.x,y+rkQ.y,z+rkQ.z);
		}
		
		Qua operator- (const Qua& rkQ) const
		{
			return Qua(w-rkQ.w,x-rkQ.x,y-rkQ.y,z-rkQ.z);
		}
		
		Qua operator* (const Qua& rkQ) const
		{

			return Qua
				(
				w * rkQ.w - x * rkQ.x - y * rkQ.y - z * rkQ.z,
				w * rkQ.x + x * rkQ.w + y * rkQ.z - z * rkQ.y,
				w * rkQ.y + y * rkQ.w + z * rkQ.x - x * rkQ.z,
				w * rkQ.z + z * rkQ.w + x * rkQ.y - y * rkQ.x
				);
		}
		
		Qua operator* (Flt fScalar) const
		{
			return Qua(fScalar*w,fScalar*x,fScalar*y,fScalar*z);
		}
		
		friend Qua operator* (Flt fScalar, const Qua& rkQ)
		{
			return Qua(fScalar*rkQ.w,fScalar*rkQ.x,fScalar*rkQ.y,fScalar*rkQ.z);
		}

		Vec3 operator* (const Vec3& v) const
		{
			Vec3 uv, uuv;
			Vec3 qvec(x, y, z);
			uv = qvec.crossProduct(v);
			uuv = qvec.crossProduct(uv);
			uv *= (2.0f * w);
			uuv *= 2.0f;

			return v + uv + uuv;
		}
		
		Qua operator- () const
		{
			return Qua(-w,-x,-y,-z);
		}
		
		Flt Dot (const Qua& rkQ) const
		{
			return w*rkQ.w+x*rkQ.x+y*rkQ.y+z*rkQ.z;
		}
		
		Flt Norm () const
		{
			return w*w+x*x+y*y+z*z;
		}

        Qua Inverse () const
        {
            Flt fNorm = w*w+x*x+y*y+z*z;
            if ( fNorm > 0.0 )
            {
                Flt fInvNorm = 1.0f/fNorm;
                return Qua(w*fInvNorm,-x*fInvNorm,-y*fInvNorm,-z*fInvNorm);
            }
            else
            {
                // return an invalid result to flag the error
                return Qua(0.0,0.0,0.0,0.0);
            }
        }

        Flt normalise(void)
        {
            Flt len = Norm();
            Flt factor = 1.0f / len * len;
            *this = *this * factor;
            return len;
        }

		// from rotate angle to quaternion 
		void FromAngleAxis (const Flt& rfAngle,const Vec3& rkAxis);

		// to axis angle axis
		void ToAngleAxis (Flt& rfAngle, Vec3& rkAxis);

		// from local axis to quaternion
		void FromAxis(const Vec3& xaxis, const Vec3& yaxis, const Vec3& zaxis);

		

		/** Calculate the local roll,pitch or yaw element of this quaternion.
		The co-domain of the returned value is from -180 to 180 degrees.
		*/
		Flt getRoll(bool reprojectAxis = true) const;
		
		Flt getPitch(bool reprojectAxis = true) const;
		
		Flt getYaw(bool reprojectAxis = true) const;		

	};
}

#endif