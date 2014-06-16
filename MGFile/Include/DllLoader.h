/******************************************************************************/
#ifndef _DLLLOADER_H_
#define _DLLLOADER_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//DLL ╪сть
	/******************************************************************************/
	class DllLoader
	{
	public:
		DllLoader();
		~DllLoader();

	public:
        bool		load(const char* filePath, bool isFirst=false);
        bool		load(const wchar_t* filePath, bool isFirst=false);
        bool		unload();

        FARPROC		getProcAddress(const char* name);
        FARPROC		getProcAddress(const wchar_t* name);

    private:

        HINSTANCE	mHinst;

	};
	
}

/******************************************************************************/

#endif