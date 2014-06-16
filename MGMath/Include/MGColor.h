/******************************************************************************/
#ifndef _H_MGVEC2_75893_
#define _H_MGVEC2_75893_
/******************************************************************************/
#define COLOR_RED			 ColorSet::red()         
#define COLOR_YELLOW		 ColorSet::yellow()
#define COLOR_CREAM			 ColorSet::cream()
#define COLOR_GREEN			 ColorSet::green()
#define COLOR_BLUE_GREEN     ColorSet::blueGreen()
#define COLOR_BLUE           ColorSet::blue()
#define COLOR_PURPLE         ColorSet::purple()
#define COLOR_WHITE          ColorSet::white()
/******************************************************************************/
namespace MG   //µã
{
    #pragma pack( push )
    #pragma pack( 1 )
        struct IColor
        {
            Flt r;
            Flt g;
            Flt b;
            Flt a;
        };
        struct IColorI
        {
            Int r;
            Int g;
            Int b;
            Int a;
        };
    #pragma pack (pop)  



    template< typename T >
    struct  TemplateColor 
    {
        T r;
        T g;
        T b;
        T a;

        TemplateColor()
            :r(0)
            ,g(0)
            ,b(0)
            ,a(1)
        {

        }
        ~TemplateColor()
        {

        }
        TemplateColor(const T& fr, const T& fg, const T& fb, const T& fa)
            :r(fr),
            g(fg),
            b(fb),
            a(fa)
        {

        }
        TemplateColor(const T& fr, const T& fg, const T& fb)
            :r(fr),
            g(fg),
            b(fb),
            a(1)
        {

        }
        TemplateColor(const TemplateColor& A)
            :r(A.r),
            g(A.g),
            b(A.b),
            a(A.a)
        {

        }
        TemplateColor& operator=(const TemplateColor& A)
        {
            r = A.r;
            g = A.g;
            b = A.b;
            a = A.a;
            return *this;
        }
        template< typename U >
        TemplateColor& operator=(const TemplateColor<U>& A)
        {
            r = A.r;
            g = A.g;
            b = A.b;
            a = A.a;
            return *this;
        }
    };

    typedef TemplateColor<Flt> Color;
    typedef TemplateColor<Int> ColorI;


    struct ColorSet
    {
    public:

        static Color zero(){ return Color(0.0f,0.0f,0.0f,0.0f); }
        static Color black(){ return Color(0.0f,0.0f,0.0f); }
        static Color white(){ return Color(1.0f,1.0f,1.0f); }
        static Color red(){ return Color(1.0f,0.0f,0.0f); }
		static Color yellow(){ return Color(0.9f,0.8f,0.2f); }
		static Color cream() { return Color(1.0f,0.98f,0.84f); }    //µ­»ÆÉ«
        static Color green(){ return Color(0.0f,1.0f,0.0f); }
		static Color blueGreen(){return Color(0.0f,1.0f,1.0f);}
        static Color blue(){ return Color(0.0f,0.0f,1.0f); }
		static Color purple(){ return Color(0.67f,0.0f,1.0f);}       //×ÏÉ«


    };
}
/******************************************************************************/
#endif
/******************************************************************************/