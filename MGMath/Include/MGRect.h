/******************************************************************************/
#ifndef _H_MGRECT_75893_
#define _H_MGRECT_75893_
/******************************************************************************/
#include "MGVector.h"
/******************************************************************************/

namespace MG   //¾ØÐÎ
{
    template< typename T >
    struct  TemplateRect //¾ØÐÎ
    {
        T left;
        T right;
        T top;
        T bottom;
        TemplateRect()
            :left(0),
            right(0),
            top(0),
            bottom(0)
        {

        }
        TemplateRect(const T& fx, const T& fy, const T& fw, const T& fh)
            :left(fx),
            right(fy),
            top(fw),
            bottom(fh)
        {

        }
        TemplateRect(const TemplateRect& A)
            :left(A.left),
            right(A.right),
            top(A.top),
            bottom(A.bottom)
        {

        }
        TemplateRect& operator=(const TemplateRect& A)
        {
            left    = A.left;
            right   = A.right;
            top     = A.top;
            bottom  = A.bottom;
            return *this;
        }
		bool operator == (TemplateRect const& _obj) const
		{
			return ((left == _obj.left) && (top == _obj.top) && (right == _obj.right) && (bottom == _obj.bottom));
		}
        template< typename U >
        TemplateRect& operator=(const TemplateRect<U>& A)
        {
            left    = A.left;
            right   = A.right;
            top     = A.top;
            bottom  = A.bottom;
            return *this;
        }

		T width() const
		{
			return right - left;
		}

		T height() const
		{
			return bottom - top;
		}

		bool isNull() const
		{
			return width() == 0 || height() == 0;
		}

		template< typename T >
		TemplateRect<T> intersect(const TemplateRect<T>& rhs) const
		{
			TemplateRect<T> ret;
			if (isNull() || rhs.isNull())
			{
				// empty
				return ret;
			}
			else
			{
				ret.left = std::max(left, rhs.left);
				ret.right = std::min(right, rhs.right);
				ret.top = std::max(top, rhs.top);
				ret.bottom = std::min(bottom, rhs.bottom);
			}

			if (ret.left > ret.right || ret.top > ret.bottom)
			{
				// no intersection, return empty
				ret.left = ret.top = ret.right = ret.bottom = 0;
			}

			return ret;
		}

		template< typename T >
		Bool inRect( TemplateVec2<T> vec2 ) const
		{
			if ( ( vec2.x - left ) * ( vec2.x - right ) <= 0 &&
				( vec2.y - top ) * ( vec2.y - bottom ) <= 0 )
				return true;
			return false;
		}
    };
    typedef TemplateRect<Int> RectI;
    typedef TemplateRect<Flt> RectF;
    //----------------------------------------------------------------------------------------------
    template< typename T >
    struct  TemplateCoord //¾ØÐÎ
    {
        T left;
        T top;
        T width;
        T height;
        TemplateCoord()
            :left(0),
            top(0),
            width(0),
            height(0)
        {

        }
        TemplateCoord(const T& fx, const T& fy, const T& fw, const T& fh)
            :left(fx),
            top(fy),
            width(fw),
            height(fh)
        {

        }
        TemplateCoord(const TemplateCoord& A)
            :left(A.left),
            top(A.top),
            width(A.width),
            height(A.height)
        {

        }
        TemplateCoord& operator=(const TemplateCoord& A)
        {
            left    = A.left;
            top   = A.top;
            width     = A.width;
            height  = A.height;
            return *this;
        }
        template< typename U >
        TemplateCoord& operator=(const TemplateCoord<U>& A)
        {
            left    = A.left;
            top   = A.top;
            width     = A.width;
            height  = A.height;
            return *this;
        }
    };
    typedef TemplateCoord<Int> CoordI;
    typedef TemplateCoord<Flt> CoordF;
}
/******************************************************************************/
#endif
/******************************************************************************/