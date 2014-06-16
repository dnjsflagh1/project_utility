/******************************************************************************/
#ifndef _INI_H_
#define _INI_H_
/******************************************************************************/


/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//INI ÎÄ¼þ¶ÁÈ¡
	/******************************************************************************/
	#pragma pack (push)
	#pragma pack (1)

	struct IniGroup
	{
		std::map< std::string, std::string >	grpItem;
		std::vector< std::string >	keys;
	};
	struct UIniGroup
	{
		std::map< std::wstring, std::wstring >	grpItem;
		std::vector< std::wstring >	keys;
	};

	#pragma pack (pop)

	// MG_CN_MODIFY
	class IMGExternalPackManager;

	class UINI
	{
	public:
		UINI();
		~UINI();

		//read INI
		// MG_CN_MODIFY
		Bool			create(std::wstring filename, IMGExternalPackManager* packManager);
		Bool			create(std::wstring filename);
		Bool			createFromMemory(Char16* memory);
		
		Bool			saveToFile(std::wstring filename, Char8* memory, I32 size);	//Save the file memory to file
		void			clear();
		void			destroy();

		Bool			hasGroup(std::wstring grpname);
		void			setGroup(std::wstring grpname);
		std::wstring	getGroup();
		std::wstring	getGroupName(const I32 index);
		I32				getGroupCount();

        std::wstring	getValue( std::wstring key, std::wstring defaultVal );
        std::wstring    getValue( std::wstring section, std::wstring key, std::wstring defaultVal );
		Bool			getKeyAndValue(I32 grpIndex, I32 itemIndex, std::wstring& key, std::wstring& val);
		
		I32				getSubGroupCount(std::wstring grpname);
		I32				getCurrentSubGroupCount();

		// create INI by yourself
		Bool			setGroupKeyValue(std::wstring group, std::wstring key, std::wstring value);//add (Group, key, value) or replace (value), cannot replace (group, key)
		Bool			deleteGroup(std::wstring group);
		Bool			deleteKey(std::wstring group, std::wstring key);
		Bool			save(std::wstring filename);

	protected:
		UIniGroup*		processLine(std::wstring nval, I32 keyLen, UIniGroup* groupData, std::wstring& curGroup);
		void*			fileRead(const Char16* filename, I32& fsize);
		Bool			getGroupName(Char16* src, I32 srcsize, std::wstring& outstr);
		Bool			getKeyName(Char16* src, std::wstring& outbuf);
		Bool			getValue(Char16* src, const I32& srcsize, std::wstring& outbuf);
		Bool			getComment(Char16* src);
		std::wstring	clearSpace(Char16* src, I32 size);
		std::wstring	clearFSpace(Char16* src, I32 size);
	
	protected:
		typedef std::map< std::wstring, UIniGroup* >::iterator				IniIter;
		typedef std::vector< std::wstring >::iterator						GroupIter;
		typedef std::map< std::wstring, std::wstring >::iterator			ItemIter;

	private:
		std::wstring														mCurrentGroup;
		std::map< std::wstring, UIniGroup* >								mIni;
		std::vector< std::wstring >											mGroupName;
        std::wstring                                                        mFilePath;
		FILE																*mFileStream;
	};

	class INI
	{
	public:
		INI();
		~INI();

		//read INI
		Bool			create(std::string filename);
		Bool			createFromMemory(Char8* src, const I32& srcsize, std::string filename);

		Bool			saveToFile(std::string filename, Char8* memory, I32 size);	//Save the file memory to file
		void			destroy();

		void			setGroup(std::string grpname);
		std::string		getGroup();
		std::string		getGroupName(const I32 index);
		I32				getGroupCount();

		std::string		getValue(std::string key, std::string defaultValue="");
		Bool			getKeyAndValue(I32 grpno, I32 index, std::string& key, std::string& value);

		I32				getSubGroupCount(std::string grpname);
		I32				getCurrentSubGroupCount();

		// create INI by yourself
		Bool			setGroupKeyValue(std::string group, std::string key, std::string value);//add (Group, key, value) or replace (value), cannot replace (group, key)
		Bool			deleteGroup(std::string group);
		Bool			deleteKey(std::string group, std::string key);
		Bool			save(std::string filename);

	protected:
		IniGroup*		processLine(std::string nval, I32 keyLen, IniGroup* pGroup, std::string& curGroup);
		void*			fileRead(const Char8* filename, I32& fsize);
		Bool			getGroupName(Char8* src, I32 srcsize, std::string& outstr);
		Bool			getKeyName(Char8* src, std::string& outbuf);
		Bool			getValue(Char8* src, const I32& srcsize, std::string& outbuf);
		Bool			getComment(Char8* src);
		std::string		clearSpace(Char8* src, I32 size);
		std::string		clearFSpace(Char8* src, I32 size);

	private:
		std::string								mCurrentGroup;
		std::map< std::string, IniGroup* >		mIni;
		std::vector< std::string >				mGroupName;
		FILE									*mFileStream;
	};

}

/******************************************************************************/

#endif