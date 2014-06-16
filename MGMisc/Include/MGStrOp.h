/******************************************************************************/
#ifndef _MGStrOp_H_
#define _MGStrOp_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//MGStrOp ×Ö·û²Ù×÷Àà
	/******************************************************************************/
	class MGStrOp
	{
    public:
		static Bool				isNullOrEmpty(CChar8* string);
		static Bool				isNullOrEmpty(CChar16* string);

		static void				makeLower (std::string& str);
		static void				makeLower (std::wstring& str);

		static void				makeUpper (std::string& str);
		static void				makeUpper (std::wstring& str);
	
		static Bool				isDigital(CChar8* digtial);
		static Bool				isDigital(CChar16* digtial);

		static I32				sprintf(Char8* dest, I32 destSize, CChar8* format, ...);
		static I32				sprintf(Char16* dest,I32 destSize, Char16* format, ...);

        static Bool             toI32(CChar8* src, I32& dest);
        static Bool				toU32(CChar8* src, U32& dest);
        static Bool				toFlt(CChar8* src, Flt& dest);
        static Bool				toU64(CChar8* src, U64& dest);
        static Bool				toDbl(CChar8* src ,Dbl& dest);

        static Bool             toI32(CChar16* src, I32& dest);
        static Bool				toU32(CChar16* src, U32& dest);
        static Bool				toU64(CChar16* src, U64& dest);
		static U64				toU64(CChar16* src);
        static Bool				toFlt(CChar16* src, Flt& dest);
        static Bool				toDbl(CChar16* src, Dbl& dest);

        static void				toString(CChar16* src, std::string& dest, int size = -1, unsigned int codePage = CP_ACP);
        static void				toString(I32 src,std::string& dest);
        static void				toString(U32 src,std::string& dest);
        static void				toString(U64 src,std::string& dest);
        static void				toString(Flt src,std::string& dest, I32 format = 2);

        static void				toString(CChar8* src, std::wstring& dest, int size = -1, unsigned int codePage = CP_ACP);
        static void				toString(I32 src, std::wstring& dest);
        static void				toString(U32 src, std::wstring& dest);
        static void				toString(U64 src, std::wstring& dest);
        static void				toString(Flt src,std::wstring& dest,I32 format = 2);

		static	void			splitFilename( const std::string& qualifiedName, std::string& outBasename, std::string& outPath );
		static  void			splitTypename( const std::string& filename, std::string& outname, std::string& outtypename );
		static  void			str_replace(std::string& strBig, const std::string& strsrc, const std::string& strdst);

        
		/* string,wstring ×Ö ·û ·Ö ¸î */
        template <typename T>
		static void             split(const T& src,char separator,std::vector<T>& dest)
		{
			std::string::size_type i = 0;
			std::string::size_type j = src.find(separator);
			while(j!=std::string::npos)
			{
				dest.push_back(src.substr(i,j-i));
				i=j+1;
				j=src.find(separator,i);
			}
			dest.push_back(src.substr(i));
		}


		/******************************************************************************/
		//×Ö·û²Ù×÷ºê
		/******************************************************************************/
		#ifdef MG_WCHAR_T_STRINGS
			#define MG_STR(x)			L ## x
			#define MG_CHAR(x)			L ## x
			#define MG_STR_CONV(x)		MG_STR(x)
			#define MG_CHAR_CONV(x)		MG_CHAR(x)
		#else
			#define MG_STR(x)			x
			#define MG_CHAR(x)			x
			#define MG_STR_CONV(x)		x
			#define MG_CHAR_CONV(x)		x
		#endif 

	};


}

/******************************************************************************/

#endif