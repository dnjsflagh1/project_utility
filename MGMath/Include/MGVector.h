/******************************************************************************/
#ifndef _H_MGVECTOR_75893_
#define _H_MGVECTOR_75893_
/******************************************************************************/

/******************************************************************************/
namespace MG 
{
    #pragma pack( push )
    #pragma pack( 1 )

        struct IVec2
        {
            Flt x;
            Flt y;
        };
        struct IVec2I
        {
            Int x;
            Int y;
        };
        struct IVec3
        {
            Flt x;
            Flt y;
            Flt z;
        };
        struct IVec3I
        {
            Int x;
            Int y;
            Int z;
        };
        struct IVec4
        {
            Flt w;
            Flt x;
            Flt y;
            Flt z;
        };
        struct IVec4I
        {
            Int w;
            Int x;
            Int y;
            Int z;
        };
    #pragma pack (pop)  

    
    template< typename T >
    struct  TemplateVec2 
    {
        T x;
        T y;
        TemplateVec2()
            :x(0),
            y(0)
        {

        }
        TemplateVec2(const T& fx, const T& fy)
            :x(fx),
            y(fy)
        {

        }
        TemplateVec2(const TemplateVec2& A)
            :x(A.x),
            y(A.y)
        {

        }
        inline TemplateVec2& operator=(const TemplateVec2& A)
        {
            x = A.x;
            y = A.y;
            return *this;
        }

        inline TemplateVec2 operator + ( const TemplateVec2& A ) const
        {
            return TemplateVec2(
                x + A.x,
                y + A.y);
        }

        inline TemplateVec2 operator - ( const TemplateVec2& A ) const
        {
            return TemplateVec2(
                x - A.x,
                y - A.y);
        }

		inline TemplateVec2 operator * ( const Flt fScalar ) const
		{
			return TemplateVec2(
				x * fScalar,
				y * fScalar);
		}

		inline TemplateVec2 operator * ( const TemplateVec2& rhs) const
		{
			return TemplateVec2(
				x * rhs.x,
				y * rhs.y);
		}

		inline TemplateVec2 operator / ( const Flt fScalar ) const
		{
			assert( fScalar != 0.0 );

			Flt fInv = 1.0f / fScalar;

			return TemplateVec2(
				x * fInv,
				y * fInv);
		}

		inline TemplateVec2 operator / ( const TemplateVec2& rhs) const
		{
			return TemplateVec2(
				x / rhs.x,
				y / rhs.y);
		}

		inline Bool operator == ( const TemplateVec2& A ) const
		{
			return ( x == A.x && y == A.y );
		}

		inline Flt length () const
		{
			return sqrt( x * x + y * y );
		}

        inline Flt lengthSquare () const
        {
            return ( x * x + y * y );
        }

        template< typename U >
        inline TemplateVec2& operator=(const TemplateVec2<U>& A)
        {
            x = A.x;
            y = A.y;
            return *this;
        }

    };

    template< typename T >
    struct  TemplateVec3 
    {
        T x;
        T y;
        T z;
        TemplateVec3()
            :x(0)
            ,y(0)
            ,z(0)
        {

        }
        TemplateVec3(const T& fx, const T& fy, const T& fz)
            :x(fx),
            y(fy),
            z(fz)
        {

        }
        TemplateVec3(const TemplateVec3& A)
            :x(A.x),
            y(A.y),
            z(A.z)
        {

        }

        inline TemplateVec3& operator=(const TemplateVec3& A)
        {
            x = A.x;
            y = A.y;
            z = A.z;
            return *this;
        }

        inline TemplateVec3 operator + ( const TemplateVec3& A ) const
        {
            return TemplateVec3(
                x + A.x,
                y + A.y,
                z + A.z);
        }

        inline TemplateVec3 operator - ( const TemplateVec3& A ) const
        {
            return TemplateVec3(
                x - A.x,
                y - A.y,
                z - A.z);
        }

        inline TemplateVec3 operator * ( const TemplateVec3& A ) const
        {
            return TemplateVec3(
                x * A.x,
                y * A.y,
                z * A.z);
        }

		inline TemplateVec3 operator * ( const Flt fScalar ) const
		{
			return TemplateVec3(
				x * fScalar,
				y * fScalar,
				z * fScalar);
		}


		inline TemplateVec3 operator / ( const Flt fScalar ) const
		{
			assert( fScalar != 0.0 );

			Flt fInv = 1.0f / fScalar;

			return TemplateVec3(
				x * fInv,
				y * fInv,
				z * fInv);
		}

		inline TemplateVec3 operator / ( const TemplateVec3& rhs) const
		{
			return TemplateVec3(
				x / rhs.x,
				y / rhs.y,
				z / rhs.z);
		}

		inline TemplateVec3& operator += ( const TemplateVec3& rkVector )
		{
			x += rkVector.x;
			y += rkVector.y;
			z += rkVector.z;

			return *this;
		}

		inline TemplateVec3& operator += ( const Flt fScalar )
		{
			x += fScalar;
			y += fScalar;
			z += fScalar;
			return *this;
		}

		inline TemplateVec3& operator -= ( const TemplateVec3& rkVector )
		{
			x -= rkVector.x;
			y -= rkVector.y;
			z -= rkVector.z;

			return *this;
		}

		inline TemplateVec3& operator -= ( const Flt fScalar )
		{
			x -= fScalar;
			y -= fScalar;
			z -= fScalar;
			return *this;
		}

		inline TemplateVec3& operator *= ( const Flt fScalar )
		{
			x *= fScalar;
			y *= fScalar;
			z *= fScalar;
			return *this;
		}

		inline TemplateVec3& operator *= ( const TemplateVec3& rkVector )
		{
			x *= rkVector.x;
			y *= rkVector.y;
			z *= rkVector.z;

			return *this;
		}

		inline TemplateVec3& operator /= ( const Flt fScalar )
		{
			assert( fScalar != 0.0 );

			Flt fInv = 1.0f / fScalar;

			x *= fInv;
			y *= fInv;
			z *= fInv;

			return *this;
		}

		inline TemplateVec3& operator /= ( const TemplateVec3& rkVector )
		{
			x /= rkVector.x;
			y /= rkVector.y;
			z /= rkVector.z;

			return *this;
		}



		inline TemplateVec3 crossProduct( const TemplateVec3& rkVector ) const
		{
			return TemplateVec3(
				y * rkVector.z - z * rkVector.y,
				z * rkVector.x - x * rkVector.z,
				x * rkVector.y - y * rkVector.x);
		}

		inline Flt dotProduct(const TemplateVec3& vec) const
		{
			return x * vec.x + y * vec.y + z * vec.z;
		}

        inline Flt angleBetween(const TemplateVec3& dest)
        {
            Flt lenProduct = length() * dest.length();

            // Divide by zero check
            if(lenProduct < 1e-6f)
                lenProduct = 1e-6f;

            Flt f = dotProduct(dest) / lenProduct;

            if ( f < -1.0 )
                f = -1.0;
            if ( f > 1.0 )
                f = 1.0;

            return acos(f);
        }

		inline Flt normalise()
		{
			Flt fLength = sqrt( x * x + y * y + z * z );

			if ( fLength > 1e-08 )
			{
				Flt fInvLength = 1.0f / fLength;
				x *= fInvLength;
				y *= fInvLength;
				z *= fInvLength;
			}

			return fLength;
		}

		inline Flt length () const
		{
			return sqrt( x * x + y * y + z * z );
		}

        inline Flt lengthSquare () const
        {
            return ( x * x + y * y + z * z );
        }


		inline Bool operator == ( const TemplateVec3& A ) const
		{
			return ( x == A.x && y == A.y && z == A.z );
		}

		inline Bool operator != ( const TemplateVec3& A ) const
		{
			return ( x != A.x || y != A.y || z != A.z );
		}

        template< typename U >
        inline TemplateVec3& operator=(const TemplateVec3<U>& A)
        {
            x = A.x;
            y = A.y;
            z = A.z;
            return *this;
        }
    };

    template< typename T >
    struct  TemplateVec4 
    {
        T w;
        T x;
        T y;
        T z;
        TemplateVec4()
            :w(1)
            ,x(0)
            ,y(0)
            ,z(0)
        {

        }
        TemplateVec4(const T& fw, const T& fx, const T& fy, const T& fz)
            :w(fw),
            x(fx),
            y(fy),
            z(fz)
        {

        }
        TemplateVec4(const TemplateVec4& A)
            :w(A.w),
            x(A.x),
            y(A.y),
            z(A.z)
        {

        }
        inline TemplateVec4& operator=(const TemplateVec4& A)
        {
            w = A.w;
            x = A.x;
            y = A.y;
            z = A.z;
            return *this;
        }

		inline TemplateVec4 operator + ( const TemplateVec4& rkVector ) const
		{
			return TemplateVec4(
				x + rkVector.x,
				y + rkVector.y,
				z + rkVector.z,
				w + rkVector.w);
		}

		inline TemplateVec4 operator - ( const TemplateVec4& rkVector ) const
		{
			return TemplateVec4(
				x - rkVector.x,
				y - rkVector.y,
				z - rkVector.z,
				w - rkVector.w);
		}

		inline TemplateVec4 operator * ( const Flt fScalar ) const
		{
			return TemplateVec4(
				x * fScalar,
				y * fScalar,
				z * fScalar,
				w * fScalar);
		}

		inline TemplateVec4 operator * ( const TemplateVec4& rhs) const
		{
			return TemplateVec4(
				rhs.x * x,
				rhs.y * y,
				rhs.z * z,
				rhs.w * w);
		}

		inline TemplateVec4 operator / ( const Flt fScalar ) const
		{
			assert( fScalar != 0.0 );

			Flt fInv = 1.0f / fScalar;

			return TemplateVec4(
				x * fInv,
				y * fInv,
				z * fInv,
				w * fInv);
		}

		inline TemplateVec4 operator / ( const TemplateVec4& rhs) const
		{
			return TemplateVec4(
				x / rhs.x,
				y / rhs.y,
				z / rhs.z,
				w / rhs.w);
		}

		inline const TemplateVec4& operator + () const
		{
			return *this;
		}

		inline TemplateVec4 operator - () const
		{
			return TemplateVec4(-x, -y, -z, -w);
		}

		inline friend TemplateVec4 operator * ( const Flt fScalar, const TemplateVec4& rkVector )
		{
			return TemplateVec4(
				fScalar * rkVector.x,
				fScalar * rkVector.y,
				fScalar * rkVector.z,
				fScalar * rkVector.w);
		}

		inline friend TemplateVec4 operator / ( const Flt fScalar, const TemplateVec4& rkVector )
		{
			return TemplateVec4(
				fScalar / rkVector.x,
				fScalar / rkVector.y,
				fScalar / rkVector.z,
				fScalar / rkVector.w);
		}

		// arithmetic updates
		inline TemplateVec4& operator += ( const TemplateVec4& rkVector )
		{
			x += rkVector.x;
			y += rkVector.y;
			z += rkVector.z;
			w += rkVector.w;

			return *this;
		}

		inline TemplateVec4& operator -= ( const TemplateVec4& rkVector )
		{
			x -= rkVector.x;
			y -= rkVector.y;
			z -= rkVector.z;
			w -= rkVector.w;

			return *this;
		}

		inline TemplateVec4& operator *= ( const Flt fScalar )
		{
			x *= fScalar;
			y *= fScalar;
			z *= fScalar;
			w *= fScalar;
			return *this;
		}

		inline TemplateVec4& operator += ( const Flt fScalar )
		{
			x += fScalar;
			y += fScalar;
			z += fScalar;
			w += fScalar;
			return *this;
		}

		inline TemplateVec4& operator -= ( const Flt fScalar )
		{
			x -= fScalar;
			y -= fScalar;
			z -= fScalar;
			w -= fScalar;
			return *this;
		}

		inline TemplateVec4& operator *= ( const TemplateVec4& rkVector )
		{
			x *= rkVector.x;
			y *= rkVector.y;
			z *= rkVector.z;
			w *= rkVector.w;

			return *this;
		}

		inline TemplateVec4& operator /= ( const Flt fScalar )
		{
			assert( fScalar != 0.0 );

			Flt fInv = 1.0f / fScalar;

			x *= fInv;
			y *= fInv;
			z *= fInv;
			w *= fInv;

			return *this;
		}

		inline TemplateVec4& operator /= ( const TemplateVec4& rkVector )
		{
			x /= rkVector.x;
			y /= rkVector.y;
			z /= rkVector.z;
			w /= rkVector.w;

			return *this;
		}

		//-----------------------------------------------------------------------
		template< typename U >
		inline TemplateVec3<U> quaternionCross(const TemplateVec3<U>& v) const
		{
			// nVidia SDK implementation
			TemplateVec3<U> uv, uuv;
			TemplateVec3<U> qvec(x, y, z);
			uv = qvec.crossProduct(v);
			uuv = qvec.crossProduct(uv);
			uv = uv*(2.0f * w);
			uuv = uuv*2.0f;

			return v + uv + uuv;
		}

		inline Bool operator == ( const TemplateVec4& A ) const
		{
			return ( w == A.w && x == A.x && y == A.y && z == A.z );
		}

        template< typename U >
        inline TemplateVec4& operator=(const TemplateVec4<U>& A)
        {
            w = A.w;
            x = A.x;
            y = A.y;
            z = A.z;
            return *this;
        }
    };

	typedef TemplateVec2<UInt> VecU2;
    typedef TemplateVec2<Int> VecI2;
	typedef TemplateVec2<I8> VecI82;
    typedef TemplateVec2<Flt> Vec2;

    typedef TemplateVec3<Int> VecI3;
    typedef TemplateVec3<Flt> Vec3;

    typedef TemplateVec4<Int> VecI4;
    typedef TemplateVec4<Flt> Vec4;
}
/******************************************************************************/
#endif
/******************************************************************************/