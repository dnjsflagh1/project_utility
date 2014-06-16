//*********************************************************************************************
#include "stdafx.h"
#include "MGQuaternion.h"
//*********************************************************************************************
namespace MG
{
	void Qua::FromAngleAxis (const Flt& rfAngle,
									const Vec3& rkAxis)
	{
		// assert:  axis[] is unit length
		//
		// The quaternion representing the rotation is
		//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

		//Radian fHalfAngle ( 0.5*rfAngle );
		Flt fSin = (Flt)sin( 0.5*rfAngle );
		w = (Flt)cos(0.5*rfAngle);
		x = fSin*rkAxis.x;
		y = fSin*rkAxis.y;
		z = fSin*rkAxis.z;
	}
	//-----------------------------------------------------------------------
	void Qua::FromAxis(const Vec3& xaxis, const Vec3& yaxis, const Vec3& zaxis)
	{
		// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
		// article "Quaternion Calculus and Fast Animation".

		Flt kRot[3][3];

		kRot[0][0] = xaxis.x;
		kRot[1][0] = xaxis.y;
		kRot[2][0] = xaxis.z;

		kRot[0][1] = yaxis.x;
		kRot[1][1] = yaxis.y;
		kRot[2][1] = yaxis.z;

		kRot[0][2] = zaxis.x;
		kRot[1][2] = zaxis.y;
		kRot[2][2] = zaxis.z;

		Flt fTrace = kRot[0][0]+kRot[1][1]+kRot[2][2];
		Flt fRoot;

		if ( fTrace > 0.0 )
		{
			// |w| > 1/2, may as well choose w > 1/2
			fRoot = sqrt(fTrace + 1.0f);  // 2w
			w = 0.5f*fRoot;
			fRoot = 0.5f/fRoot;  // 1/(4w)
			x = (kRot[2][1]-kRot[1][2])*fRoot;
			y = (kRot[0][2]-kRot[2][0])*fRoot;
			z = (kRot[1][0]-kRot[0][1])*fRoot;
		}
		else
		{
			// |w| <= 1/2
			static size_t s_iNext[3] = { 1, 2, 0 };
			size_t i = 0;
			if ( kRot[1][1] > kRot[0][0] )
				i = 1;
			if ( kRot[2][2] > kRot[i][i] )
				i = 2;
			size_t j = s_iNext[i];
			size_t k = s_iNext[j];

			fRoot = sqrt(kRot[i][i]-kRot[j][j]-kRot[k][k] + 1.0f);
			Flt* apkQuat[3] = { &x, &y, &z };
			*apkQuat[i] = 0.5f*fRoot;
			fRoot = 0.5f/fRoot;
			w = (kRot[k][j]-kRot[j][k])*fRoot;
			*apkQuat[j] = (kRot[j][i]+kRot[i][j])*fRoot;
			*apkQuat[k] = (kRot[k][i]+kRot[i][k])*fRoot;
		};
	}
	//-----------------------------------------------------------------------
	void Qua::ToAngleAxis( Flt& rfAngle, Vec3& rkAxis )
	{
		Flt fSqrLength = x*x + y*y + z*z;
		if (fSqrLength > 0.0)
		{
			////////////////////////////////////////////////////

			if (-1.0 < w)
			{
				if (w < 1.0)
				{
					rfAngle = acos(w);
				}
				else
				{
					rfAngle = .0f;
				}
			}
			else
			{
				rfAngle = Flt(2.0 * atan( 1.0 )) ;
			}

			rfAngle *= 2.0f;

			//////////////////////////////////////////////////////////////////////////

			double fInvLength = 1.0f / sqrt(fSqrLength);
			rkAxis.x = Flt(x * fInvLength);
			rkAxis.y = Flt(y * fInvLength);
			rkAxis.z = Flt(z * fInvLength);
		}
		else
		{
			// angle is 0 (mod 2*pi), so any axis will do
			rfAngle = .0f;
			rkAxis.x = 1.0;
			rkAxis.y = 0.0;
			rkAxis.z = 0.0;
		}
	}
	//-----------------------------------------------------------------------
	Flt Qua::getRoll(bool reprojectAxis) const
	{
		if (reprojectAxis)
		{
			// roll = atan2(localx.y, localx.x)
			// pick parts of xAxis() implementation that we need
			//			Real fTx  = 2.0*x;
			Flt fTy  = 2.0f*y;
			Flt fTz  = 2.0f*z;
			Flt fTwz = fTz*w;
			Flt fTxy = fTy*x;
			Flt fTyy = fTy*y;
			Flt fTzz = fTz*z;

			// Vector3(1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);

			return Flt(MGMath::ATan2(fTxy+fTwz, 1.0f-(fTyy+fTzz)));

		}
		else
		{
			return Flt(MGMath::ATan2(2*(x*y + w*z), w*w + x*x - y*y - z*z));
		}
	}
	//-----------------------------------------------------------------------
	Flt Qua::getPitch(bool reprojectAxis) const
	{
		if (reprojectAxis)
		{
			// pitch = atan2(localy.z, localy.y)
			// pick parts of yAxis() implementation that we need
			Flt fTx  = 2.0f*x;
			//			Real fTy  = 2.0f*y;
			Flt fTz  = 2.0f*z;
			Flt fTwx = fTx*w;
			Flt fTxx = fTx*x;
			Flt fTyz = fTz*y;
			Flt fTzz = fTz*z;

			// Vector3(fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx);
			return Flt(MGMath::ATan2(fTyz+fTwx, 1.0f-(fTxx+fTzz)));
		}
		else
		{
			// internal version
			return Flt(MGMath::ATan2(2*(y*z + w*x), w*w - x*x - y*y + z*z));
		}
	}
	//-----------------------------------------------------------------------
	Flt Qua::getYaw(bool reprojectAxis) const
	{
		if (reprojectAxis)
		{
			// yaw = atan2(localz.x, localz.z)
			// pick parts of zAxis() implementation that we need
			Flt fTx  = 2.0f*x;
			Flt fTy  = 2.0f*y;
			Flt fTz  = 2.0f*z;
			Flt fTwy = fTy*w;
			Flt fTxx = fTx*x;
			Flt fTxz = fTz*x;
			Flt fTyy = fTy*y;

			// Vector3(fTxz+fTwy, fTyz-fTwx, 1.0-(fTxx+fTyy));

			return Flt(MGMath::ATan2(fTxz+fTwy, 1.0f-(fTxx+fTyy)));

		}
		else
		{
			// internal version
			return Flt(MGMath::ASin(-2*(x*z - w*y)));
		}
	}

}
