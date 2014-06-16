/******************************************************************************/
#include "stdafx.h"
#include "Ini.h"
#include "MGStrOp.h"

// MG_CN_MODIFY
#include "IMGPackExterInterface.h"
/******************************************************************************/

namespace MG
{
	/*******************************************************************************/
	// class UINI  UNCIDOE 配置文件
	/*******************************************************************************/
	//-----------------------------------------------------------------------------
	// MG_CN_MODIFY
	MG::Bool UINI::create( std::wstring filename, IMGExternalPackManager* packManager)
	{
		if (create(filename))
			return true;
		else if (packManager)
		{
			Char16* buf = NULL;
			Str fileNametemp;
			MGStrOp::toString(filename.c_str(), fileNametemp);
			packManager->extractPackFile(fileNametemp, buf);

			if (createFromMemory(buf))
				return true;
			else
				return false;
		}
		else
			return false;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::create( std::wstring filename )
	{  
		//这种是最常用的方法。
		wchar_t   szFileName[256]; 
		GetModuleFileNameW(NULL,szFileName,256); 
		wchar_t   szDrive[256]; 
		wchar_t   szDir[256]; 
		wchar_t   szPath[256]; 
		_wsplitpath(szFileName,szDrive,szDir,NULL,NULL); 
		swprintf(szPath, L"%s%s",szDrive,szDir);
		mFilePath = szPath;									//当前应用程序所在路径
		mFilePath += filename;                              //INI文件所在路径
		if (_wfopen_s(&mFileStream,mFilePath.c_str(),L"r+b") == 0)
		{	
			I32 size = 0;
			fseek(mFileStream,0L,SEEK_END);
			size = ftell(mFileStream);
			fseek(mFileStream,0L,SEEK_SET);
			if (size != 0)
			{
				I32	uSize = size / 2;
				Char16* buf = MG_NEW Char16[uSize + 1];
				buf[uSize] = '\0';
				fread(buf,sizeof(Char16),uSize,mFileStream);
				fclose(mFileStream);
				mFileStream = NULL;
				//文件不是Unicode格式
				DYNAMIC_ASSERT(buf[0] == 0xFEFF);
				if (buf[0] != 0xFEFF)
				{
					MG_SAFE_DELETE(buf);
					return false;
				}

				std::wstring textValue = buf;
				MG_SAFE_DELETE(buf);
				I32 rowOldPos = 0;
				I32 rowNewPos = 0;

				// 去除 /**/ 注释
				rowOldPos = textValue.find(L"/*");
				if (rowOldPos == std::wstring::npos)
				{
					rowNewPos = textValue.find(L"*/");
					if (rowNewPos != std::wstring::npos)
					{
						//注释不匹配
						DYNAMIC_ASSERT(false);
						return false;
					}
				}
				while(rowOldPos != std::wstring::npos)
				{
					rowNewPos = textValue.find(L"*/",rowOldPos);
					if (rowNewPos == std::wstring::npos)
					{
						//注释不匹配
						DYNAMIC_ASSERT(false);
						return false;
					}
					textValue.erase(rowOldPos,rowNewPos - rowOldPos + 2);
					rowOldPos = textValue.find(L"/*",rowOldPos);

				}

				// 去除 //注释
				rowOldPos = rowOldPos = textValue.find(L"//");
				rowNewPos = std::wstring::npos;
				for(I32 i = 0; rowOldPos != std::wstring::npos; i++)
				{
					rowNewPos = textValue.find(L"\r\n",rowOldPos);
					if (rowNewPos == std::wstring::npos)
					{
						rowNewPos = textValue.size() - 1;
					}
					textValue.erase(rowOldPos,rowNewPos - rowOldPos + 1);
					rowOldPos = textValue.find(L"//");
				}
				// 去除空格
				rowNewPos = textValue.find(L' ');
				while(rowNewPos !=  std::wstring::npos)
				{
					textValue.erase(rowNewPos,1);
					rowNewPos = textValue.find(L' ',rowNewPos);
				}
				//解析数据 读取行
				UIniGroup* tempIniG = NULL;
				rowOldPos = 0;
				rowNewPos = textValue.find(L"\r\n");
				std::wstring row;
				for(I32 i = 0; true; i++)
				{

					if (rowNewPos !=  std::wstring::npos)
					{
						textValue.erase(rowNewPos,2);
						if (rowNewPos == rowOldPos)
						{
							rowNewPos = textValue.find(L"\r\n");
							continue;
						}
						row = textValue.substr(rowOldPos,rowNewPos - rowOldPos);
						Str16 key;
						Str16 value;
						// 解析行
						if (0 == i)
						{
							I32 oldPos = row.find(L"[");
							I32 newPos = row.find(L"]");
							if (oldPos == std::wstring::npos
								|| newPos == std::wstring::npos)
							{
								DYNAMIC_ASSERT(false);
								return false;
							}
							tempIniG = MG_NEW UIniGroup();
							DYNAMIC_ASSERT(tempIniG);
							key = row.substr(oldPos + 1,newPos - oldPos - 1);
							mIni[key] = tempIniG;
							mGroupName.push_back(key);
						}
						else
						{

							I32 oldPos = row.find(L"[");
							I32 newPos = row.find(L"]");
							if ((oldPos == std::wstring::npos|| newPos == std::wstring::npos) 
								&& oldPos != 0)
							{
								if (!tempIniG)
								{
									DYNAMIC_ASSERT(tempIniG);
									return false;
								}
								newPos = row.find(L'=');
								key = row.substr(0,newPos);
								if (tempIniG->grpItem.find(key) != tempIniG->grpItem.end())
								{
									DYNAMIC_ASSERT(false);
									return false;
								}
								value = row.substr(newPos + 1,row.size() - newPos - 1);
								tempIniG->grpItem[key] = value;
								tempIniG->keys.push_back(key);
							}
							else
							{
								tempIniG = MG_NEW UIniGroup();
								DYNAMIC_ASSERT(tempIniG);
								Str16 key = row.substr(oldPos + 1,newPos - oldPos - 1);
								if (mIni.find(key) != mIni.end())
								{
									DYNAMIC_ASSERT(false);
									return false;
								}
								mIni[key] = tempIniG;
								mGroupName.push_back(key);
							}

						}

					}
					else
					{
						break;
					}
					rowOldPos = rowNewPos;
					rowNewPos = textValue.find(L"\r\n");
					if (rowOldPos != string::npos && rowNewPos == string::npos)
					{
						rowNewPos = textValue.size();
					}
				}
				return true;
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::createFromMemory( Char16* memory )
	{
		DYNAMIC_ASSERT(memory[0] == 0xFEFF);
		if (memory[0] != 0xFEFF)
		{
			MG_SAFE_DELETE(memory);
			return false;
		}

		std::wstring textValue = memory;
		MG_SAFE_DELETE(memory);
		I32 rowOldPos = 0;
		I32 rowNewPos = 0;

		// 去除 /**/ 注释
		rowOldPos = textValue.find(L"/*");
		if (rowOldPos == std::wstring::npos)
		{
			rowNewPos = textValue.find(L"*/");
			if (rowNewPos != std::wstring::npos)
			{
				//注释不匹配
				DYNAMIC_ASSERT(false);
				return false;
			}
		}
		while(rowOldPos != std::wstring::npos)
		{
			rowNewPos = textValue.find(L"*/",rowOldPos);
			if (rowNewPos == std::wstring::npos)
			{
				//注释不匹配
				DYNAMIC_ASSERT(false);
				return false;
			}
			textValue.erase(rowOldPos,rowNewPos - rowOldPos + 2);
			rowOldPos = textValue.find(L"/*",rowOldPos);

		}

		// 去除 //注释
		rowOldPos = rowOldPos = textValue.find(L"//");
		rowNewPos = std::wstring::npos;
		for(I32 i = 0; rowOldPos != std::wstring::npos; i++)
		{
			rowNewPos = textValue.find(L"\r\n",rowOldPos);
			if (rowNewPos == std::wstring::npos)
			{
				rowNewPos = textValue.size() - 1;
			}
			textValue.erase(rowOldPos,rowNewPos - rowOldPos + 1);
			rowOldPos = textValue.find(L"//");
		}
		// 去除空格
		rowNewPos = textValue.find(L' ');
		while(rowNewPos !=  std::wstring::npos)
		{
			textValue.erase(rowNewPos,1);
			rowNewPos = textValue.find(L' ',rowNewPos);
		}
		//解析数据 读取行
		UIniGroup* tempIniG = NULL;
		rowOldPos = 0;
		rowNewPos = textValue.find(L"\r\n");
		std::wstring row;
		for(I32 i = 0; true; i++)
		{

			if (rowNewPos !=  std::wstring::npos)
			{
				textValue.erase(rowNewPos,2);
				if (rowNewPos == rowOldPos)
				{
					rowNewPos = textValue.find(L"\r\n");
					continue;
				}
				row = textValue.substr(rowOldPos,rowNewPos - rowOldPos);
				Str16 key;
				Str16 value;
				// 解析行
				if (0 == i)
				{
					I32 oldPos = row.find(L"[");
					I32 newPos = row.find(L"]");
					if (oldPos == std::wstring::npos
						|| newPos == std::wstring::npos)
					{
						DYNAMIC_ASSERT(false);
						return false;
					}
					tempIniG = MG_NEW UIniGroup();
					DYNAMIC_ASSERT(tempIniG);
					key = row.substr(oldPos + 1,newPos - oldPos - 1);
					mIni[key] = tempIniG;
					mGroupName.push_back(key);
				}
				else
				{

					I32 oldPos = row.find(L"[");
					I32 newPos = row.find(L"]");
					if (oldPos == std::wstring::npos
						|| newPos == std::wstring::npos)
					{
						if (!tempIniG)
						{
							DYNAMIC_ASSERT(tempIniG);
							return false;
						}
						newPos = row.find(L'=');
						key = row.substr(0,newPos);
						if (tempIniG->grpItem.find(key) != tempIniG->grpItem.end())
						{
							DYNAMIC_ASSERT(false);
							return false;
						}
						value = row.substr(newPos + 1,row.size() - newPos - 1);
						tempIniG->grpItem[key] = value;
						tempIniG->keys.push_back(key);
					}
					else
					{
						tempIniG = MG_NEW UIniGroup();
						DYNAMIC_ASSERT(tempIniG);
						Str16 key = row.substr(oldPos + 1,newPos - oldPos - 1);
						if (mIni.find(key) != mIni.end())
						{
							DYNAMIC_ASSERT(false);
							return false;
						}
						mIni[key] = tempIniG;
						mGroupName.push_back(key);
					}

				}

			}
			else
			{
				break;
			}
			rowOldPos = rowNewPos;
			rowNewPos = textValue.find(L"\r\n");
		}
		return true;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::saveToFile( std::wstring filename, Char8* memory, I32 size )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	void UINI::destroy()
	{
		if (mFileStream != NULL)
		{
			fclose(mFileStream);
			mFileStream = NULL;
		}

		IniIter it = mIni.begin();
		for (; it != mIni.end(); it++)
		{
			MG_SAFE_DELETE(it->second);
		}
		mIni.clear();
	}
	//-----------------------------------------------------------------------------
	Bool UINI::hasGroup(std::wstring grpname)
	{
		std::vector< std::wstring >	::iterator iter = mGroupName.begin();
		for (;iter!=mGroupName.end();iter++)
		{
			if ( *iter == grpname )
			{
				return true;
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	void UINI::setGroup( std::wstring grpname )
	{
		mCurrentGroup = grpname;
	}
	//-----------------------------------------------------------------------------
	std::wstring UINI::getGroup()
	{
		return mCurrentGroup;
	}
	//-----------------------------------------------------------------------------
	std::wstring UINI::getGroupName( const I32 index )
	{
		if (index > -1 && mGroupName.size() > (U32)index)
		{
			return mGroupName[index];
		}
		return L"";
	}
	//-----------------------------------------------------------------------------
	I32 UINI::getGroupCount()
	{
		return mGroupName.size();
	}
	//-----------------------------------------------------------------------------
	std::wstring UINI::getValue( std::wstring key, std::wstring defaultVal )
	{
		if (mCurrentGroup.size() > 0)
		{
			IniIter iniIt = mIni.find(mCurrentGroup);
			if (iniIt != mIni.end())
			{
				ItemIter itemIt = iniIt->second->grpItem.find(key);
				if (itemIt != iniIt->second->grpItem.end())
				{
					return itemIt->second;
				}
			}
		}

		IniIter iniIt = mIni.begin();
		for (; iniIt != mIni.end(); iniIt++)
		{
			ItemIter itemIt = iniIt->second->grpItem.find(key);
			if (itemIt != iniIt->second->grpItem.end())
			{
				return itemIt->second;
			}
		}
		return defaultVal;
	}
    //-----------------------------------------------------------------------------
    std::wstring UINI::getValue( std::wstring section, std::wstring key, std::wstring defaultVal )
    {
        IniIter iniIt = mIni.find(section);
        if (iniIt != mIni.end())
        {
            ItemIter itemIt = iniIt->second->grpItem.find(key);
            if (itemIt != iniIt->second->grpItem.end())
            {
                return itemIt->second;
            }
        }
        return defaultVal;
    }
	//-----------------------------------------------------------------------------
	//犹豫采用的是map
	Bool UINI::getKeyAndValue( I32 grpIndex, I32 itemIndex, std::wstring& key, std::wstring& val )
	{
		if ( grpIndex > -1 &&
			 mGroupName.size() > (U32)grpIndex &&
			 itemIndex > -1)
		{

			if (mIni[mGroupName[grpIndex]]->grpItem.size() > (U32)itemIndex)
			{
				return true;
			}
			 
		}

		return false;
	}
	//-----------------------------------------------------------------------------
	I32 UINI::getSubGroupCount( std::wstring grpname )
	{
		IniIter it = mIni.find(grpname);
		IniIter itEnd = mIni.end();
		if (it != itEnd)
		{
			return it->second->grpItem.size();
		}

		return 0;
	}
	//-----------------------------------------------------------------------------
	I32 UINI::getCurrentSubGroupCount()
	{
		IniIter it = mIni.find(mCurrentGroup);
		IniIter itEnd = mIni.end();
		if (it != itEnd)
		{
			return it->second->grpItem.size();
		}
		return  0;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::setGroupKeyValue( std::wstring group, std::wstring key, std::wstring val )
	{
		mCurrentGroup = group;
		IniIter it = mIni.find(mCurrentGroup);
		IniIter itEnd = mIni.end();
		if(it != itEnd)
		{	
			UIniGroup* groupData = it->second;
			groupData->grpItem[key] = val;
			return false;
		}
		else
		{
			UIniGroup* groupData = MG_NEW UIniGroup();
			mIni[group] = groupData;
			groupData->grpItem[key] = val;
			mGroupName.push_back( group );
			
			return true;
		}
	}
	//-----------------------------------------------------------------------------
	Bool UINI::deleteGroup( std::wstring group )
	{
		for (GroupIter it = mGroupName.begin(); it != mGroupName.end(); it++)
		{
			if (it->compare(group.c_str()) == 0)
			{
				mGroupName.erase(it);
				MG_SAFE_DELETE(mIni[group]);
				mIni.erase(group);
				return true;
			}
		}

		return false;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::deleteKey( std::wstring group, std::wstring key )
	{
		IniIter it = mIni.begin();
		IniIter itEnd = mIni.end();
		if (it != itEnd)
		{
			ItemIter itemIt = it->second->grpItem.begin();
			ItemIter itemItEnd = it->second->grpItem.end();
			if (itemIt != itemItEnd)
			{
				it->second->grpItem.erase(itemIt);
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::save( std::wstring filename )
	{
		wchar_t   szFileName[256]; 
		GetModuleFileNameW(NULL,szFileName,256); 
		wchar_t   szDrive[256]; 
		wchar_t   szDir[256]; 
		wchar_t   szPath[256]; 
		_wsplitpath(szFileName,szDrive,szDir,NULL,NULL); 
		swprintf(szPath, L"%s%s",szDrive,szDir);
		Str16 filePath = szPath;									//当前应用程序所在路径
		filePath += filename;										//INI文件所在路径

		if (filePath.compare(mFilePath))
		{	
			FILE* file = NULL;
			if (_wfopen_s(&file,filePath.c_str(),L"w+b") == 0)
			{
				fseek(file,0L,SEEK_SET);
				Str16 saveBuf;
				Char16 tempChar = 0xFEFF;
				saveBuf += tempChar;
				IniIter iniIt = mIni.begin();
				ItemIter itemIt;
				for ( ; iniIt != mIni.end(); iniIt++ )
				{
					saveBuf += L'[';
					saveBuf += iniIt->first;
					saveBuf += L']';
					saveBuf += L"\r\n";
					for (itemIt = iniIt->second->grpItem.begin(); itemIt != iniIt->second->grpItem.end(); itemIt++)
					{
						saveBuf += itemIt->first;
						saveBuf += L'=';
						saveBuf += itemIt->second;
						saveBuf += L"\r\n";
					}
				}
				if (fwrite(saveBuf.c_str(),2,saveBuf.size(),mFileStream))
				{
					fclose(file);
					return true;
				}
				fclose(file); 
			}
		}
		else
		{
			if (mFileStream)
			{
				fclose(mFileStream);
			}
			if (_wfopen_s(&mFileStream,mFilePath.c_str(),L"w+b") == 0)
			{
				fseek(mFileStream,0L,SEEK_SET);
				Str16 saveBuf;
				Char16 tempChar = 0xFEFF;
				saveBuf += tempChar;
				IniIter iniIt = mIni.begin();
				ItemIter itemIt;
				for ( ; iniIt != mIni.end(); iniIt++ )
				{
					saveBuf += L'[';
					saveBuf += iniIt->first;
					saveBuf += L']';
					saveBuf += L"\r\n";
					for (itemIt = iniIt->second->grpItem.begin(); itemIt != iniIt->second->grpItem.end(); itemIt++)
					{
						saveBuf += itemIt->first;
						saveBuf += L'=';
						saveBuf += itemIt->second;
						saveBuf += L"\r\n";
					}
				}
				if(fwrite(saveBuf.c_str(),2,saveBuf.size(),mFileStream))
				{
					fclose(mFileStream);
					return true;
				}
				fclose(mFileStream);
			}		
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	void UINI::clear()
	{
		//清空文件
		mCurrentGroup.clear();
		mGroupName.clear();

		for ( IniIter iniIter = mIni.begin(); iniIter != mIni.end(); iniIter++ )
		{
			MG_SAFE_DELETE( iniIter->second );
		}
		mIni.clear();
	}
	//-----------------------------------------------------------------------------
	UIniGroup* UINI::processLine( std::wstring nval, I32 keyLen, UIniGroup* groupData, std::wstring& curGroup )
	{
		return NULL;
	}
	//-----------------------------------------------------------------------------
	void* UINI::fileRead( const Char16* filename, I32& fsize )
	{
		return NULL;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::getGroupName( Char16* src, I32 srcsize, std::wstring& outstr )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::getKeyName( Char16* src, std::wstring& outbuf )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool UINI::getValue( Char16* src, const I32& srcsize, std::wstring& outbuf )
	{
		return false;
	}
    //-----------------------------------------------------------------------------
	Bool UINI::getComment( Char16* src )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	std::wstring UINI::clearSpace( Char16* src, I32 size )
	{
		return L"";
	}
	//-----------------------------------------------------------------------------
	std::wstring UINI::clearFSpace( Char16* src, I32 size )
	{
		return L"";
	}
    //-----------------------------------------------------------------------------
    UINI::UINI()
    {

    }
    //-----------------------------------------------------------------------------
    UINI::~UINI()
    {
        destroy();
    }
    //-----------------------------------------------------------------------------
	/*******************************************************************************/
	//class INI  ASCI 配置文件
	/*******************************************************************************/
	//-----------------------------------------------------------------------------
	Bool INI::create( std::string filename )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::createFromMemory( Char8* src, const I32& srcsize, std::string filename )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::saveToFile( std::string filename, Char8* memory, I32 size )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	void INI::destroy()
	{

	}
	//-----------------------------------------------------------------------------
	void INI::setGroup( std::string grpname )
	{

	}
	//-----------------------------------------------------------------------------
	std::string INI::getGroup()
	{
		return "";
	}
	//-----------------------------------------------------------------------------
	std::string INI::getGroupName( const I32 index )
	{
		return "";
	}
	//-----------------------------------------------------------------------------
	I32 INI::getGroupCount()
	{
		return 0;
	}
	//-----------------------------------------------------------------------------
	std::string INI::getValue( std::string key, std::string defaultValue/*=L""*/ )
	{
		return "";
	}
	//-----------------------------------------------------------------------------
	Bool INI::getKeyAndValue( I32 grpno, I32 index, std::string& key, std::string& value )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	I32 INI::getSubGroupCount( std::string grpname )
	{
		return 0;
	}
	//-----------------------------------------------------------------------------
	I32 INI::getCurrentSubGroupCount()
	{
		return 0;
	}
	//-----------------------------------------------------------------------------
	Bool INI::setGroupKeyValue( std::string group, std::string key, std::string value )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::deleteGroup( std::string group )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::deleteKey( std::string group, std::string key )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::save( std::string filename )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	IniGroup* INI::processLine( std::string nval, I32 keyLen, IniGroup* pGroup, std::string& curGroup )
	{
		return NULL;
	}
	//-----------------------------------------------------------------------------
	void* INI::fileRead( const Char8* filename, I32& fsize )
	{
		return NULL;
	}
	//-----------------------------------------------------------------------------
	Bool INI::getGroupName( Char8* src, I32 srcsize, std::string& outstr )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::getKeyName( Char8* src, std::string& outbuf )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::getValue( Char8* src, const I32& srcsize, std::string& outbuf )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool INI::getComment( Char8* src )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	std::string INI::clearSpace( Char8* src, I32 size )
	{
		return "";
	}
	//-----------------------------------------------------------------------------
	std::string INI::clearFSpace( Char8* src, I32 size )
	{
		return "";
	}
	//-----------------------------------------------------------------------------
}