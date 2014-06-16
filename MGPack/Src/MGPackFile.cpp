/******************************************************************************/

#include "stdafx.h"
#include "MGPackFile.h"


/******************************************************************************/
#define LEN 1024

namespace MG
{
	/******************************************************************************/
	// MGPackFileSet
	/******************************************************************************/
	const static char RES_PACK_PASSWORD[32] = "aosheng!yingxiongzhi@mg#cn$8858";
	//-----------------------------------------------------------------------
	MGPackFileSet::MGPackFileSet()
	{
	}

	//-----------------------------------------------------------------------
	MGPackFileSet::~MGPackFileSet()
	{
		unLoad();

		MG_SAFE_DELETE(mPathFileInfoMap);
		MG_SAFE_DELETE(mPathFileInfoTypeMap);
	}

	//-----------------------------------------------------------------------
	bool MGPackFileSet::packRes( const std::string& src, const std::string& des, const char* password )
	{
		WIN32_FIND_DATA FindData;
		HANDLE hError;
		int FileCount = 0;
		char FilePathName[LEN];
		// 构造路径
		char FullPathName[LEN];
		strcpy(FilePathName, src.c_str());
		strcat(FilePathName, "\\*.*");
		hError = FindFirstFile(FilePathName, &FindData);
		if (hError == INVALID_HANDLE_VALUE)
		{
			return 0;
		}
		while(::FindNextFile(hError, &FindData))
		{
			// 过虑.和..
			if (strcmp(FindData.cFileName, ".") == 0
				|| strcmp(FindData.cFileName, "..") == 0 )
			{
				continue;
			}
			// 构造完整路径
			wsprintf(FullPathName, "%s\\%s", src.c_str(), FindData.cFileName);
			FileCount++;
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{	
				if (strcmp(FindData.cFileName, ".svn") == 0)
					continue;

				char PackName[LEN];
				wsprintf(PackName, "%s\\%s.cn", des.c_str(), FindData.cFileName);
				if (!mArchive.Open(PackName, CZipArchive::zipCreate))
					return false;

				mArchive.AddNewFiles(FullPathName, "*.*", true, 0, true, CZipArchive::zipsmIgnoreDirectories);
				if ( (strcmp(FindData.cFileName, "Engine") == 0) ||
					 (strcmp(FindData.cFileName, "Game") == 0) ||
					 (strcmp(FindData.cFileName, "Script") == 0) ||
					 (strcmp(FindData.cFileName, "Core")) == 0 )
				{
					mArchive.SetPassword(password);
				}
				mArchive.EncryptAllFiles();

				mArchive.Close();
			}
		}
		return true;
	}

	//-----------------------------------------------------------------------
	bool MGPackFileSet::unZip( const std::string& zipname, const std::string& unzipfolder )
	{
		if( !mArchive.Open(zipname.c_str(), CZipArchive::zipOpenReadOnly ) )
			return FALSE;

		mArchive.SetPassword(RES_PACK_PASSWORD);

		for ( int nIndex = 0;  nIndex < mArchive.GetCount(); nIndex++ )
		{
			if( !mArchive.ExtractFile( (WORD)nIndex, unzipfolder.c_str(), true ) )
				break;
		}
		mArchive.Close();

		return true;
	}
	//-----------------------------------------------------------------------
	bool MGPackFileSet::injectPack( const std::string& unzipfolder, const std::string& tempClienResfolder, 
							StringMap &filemap )
	{
		std::string unzipfoldertemp, tempClienResfoldertemp;
		unzipfoldertemp = unzipfolder;
		tempClienResfoldertemp = tempClienResfolder;

		std::replace( unzipfoldertemp.begin(), unzipfoldertemp.end(), '\\', '/' );
		std::replace( tempClienResfoldertemp.begin(), tempClienResfoldertemp.end(), '\\', '/' );

		CZipArchive copyClientResArc,updateResArc;

		StringMap::iterator iter = filemap.begin();
		char clientResFileName[1024], updateResFileName[1024];
		for (; iter != filemap.end(); iter++)
		{
			strcpy(clientResFileName, tempClienResfoldertemp.c_str());
			wsprintf(clientResFileName, "%s/%s", clientResFileName, (iter->first).c_str());

			strcpy(updateResFileName, unzipfoldertemp.c_str());
			wsprintf(updateResFileName, "%s/%s", updateResFileName, (iter->first).c_str());

			// 客户端有这个压缩包的情况
			if (_access(clientResFileName, 0) == 0)
			{
				if (!copyClientResArc.Open(clientResFileName,CZipArchive::zipOpen))
					return false;

				if (!updateResArc.Open(updateResFileName,CZipArchive::zipOpen))
					return false;

				copyClientResArc.SetPassword(RES_PACK_PASSWORD);
				updateResArc.SetPassword(RES_PACK_PASSWORD);

				for(WORD i = 0;i < updateResArc.GetCount(false);i++)
				{
					CZipFileHeader info;
					if (updateResArc.GetFileInfo(info, i)) 
					{
						CZipMemFile* pmf = new CZipMemFile;
						if (updateResArc.ExtractFile(i, *pmf))
						{
							CZipString s = info.GetFileName();
							CZipFileHeader *deleteHeader = NULL;
							deleteHeader = copyClientResArc.GetFileInfo(copyClientResArc.FindFile(s));
							if (deleteHeader)
							{
								copyClientResArc.RemoveFile(copyClientResArc.FindFile(s));
							}

							copyClientResArc.AddNewFile(*pmf, info.GetFileName());
						}
					}
				}

				copyClientResArc.Close();
				updateResArc.Close();
			}
			else
			{
				if (!CopyFile(updateResFileName, clientResFileName, FALSE))
					return false;
			}

		}

		return true;
	}
	//-----------------------------------------------------------------------
	void MGPackFileSet::load( const std::string& filename, const char* password )
	{
		std::string path = filename;
		unsigned int i = path.find_last_of('/');

		std::string type, partPath, filetype;
		if (i != std::string::npos)
		{
			type = path.substr(i+1, path.size() - i - 1);

			MGStrOp::splitTypename(type, partPath, filetype);
		}
		std::string packtype = partPath;

		mArchive.Open(filename.c_str(), CZipArchive::zipOpenReadOnly);
		mArchive.EnableFindFast();	
		mArchive.SetPassword(password);

		mPathFileInfoMap = MG_NEW ClassifyPathMap();
		mPathFileInfoTypeMap = MG_NEW ClassifyPathTypeMap();

		// 同一路径下的vector;
		FileInfoVec PathFileVec;
		// 同一路径下的文件map;
		ClassifyPathMulMap	PathMap;

		// 用于做路径对比的字符串
		std::string	pathCmptemp;

		std::string fullpath, restypetemp;
		// 遍历所有文件
		for(WORD i = 0; i < mArchive.GetCount(false); i++)
		{
			CZipFileHeader infoFromZip;
			if (mArchive.GetFileInfo(infoFromZip, i)) 
			{
				CZipString s = infoFromZip.GetFileName();
				std::string fileName((LPCTSTR)s);

				MGFileInfo info;
				// Get basename / path
				MGStrOp::splitFilename(fileName, info.basename, info.path);
				// ignore folder entries
				if (filename.empty())
					continue;

				info.filename = fileName;
				// Get sizes
				info.compressedSize = infoFromZip.m_uComprSize;
				info.uncompressedSize = infoFromZip.m_uUncomprSize;

				std::string stypename, temp;
				MGStrOp::splitTypename(info.basename, temp, stypename);
				info.filetype = stypename;

				MGStrOp::makeLower(info.basename);
				MGStrOp::makeLower(info.filename);

				// 保存文件列表
				mFilePathMap.insert(std::make_pair(info.basename, info.filename));

				// 路径,压缩包类型
				char resPathtemp[LEN];
				if (path.find("Config", 0) != std::string::npos)
				{
					if (info.path.empty())
						wsprintf(resPathtemp, "Config/%s/", partPath.c_str());
					else
						wsprintf(resPathtemp, "Config/%s/%s", partPath.c_str(), info.path.c_str());
				}
				else if (path.find("Media", 0) != std::string::npos)
				{
					if (info.path.empty())
						wsprintf(resPathtemp, "Media/%s/", partPath.c_str());
					else
						wsprintf(resPathtemp, "Media/%s/%s", partPath.c_str(), info.path.c_str());
				}
				std::string restype, typetemp;
				MGStrOp::splitTypename(path, restype, typetemp);
				MGStrOp::makeLower(restype);
				mPathPackTypeMap.insert(std::make_pair(resPathtemp, restype));

				MGStrOp::splitTypename(path, fullpath, restypetemp);
				MGStrOp::makeLower(fullpath);
				MGStrOp::str_replace(fullpath, "\\", "/");
				MGStrOp::str_replace(info.filename, "\\", "/");
				char fullPathtemp[LEN];
				wsprintf(fullPathtemp, "%s/%s", fullpath.c_str(), info.filename.c_str());
				mFullNameResName.insert(std::make_pair(fullPathtemp, info.filename));

				info.path = info.path.substr(0, info.path.size()-1);
				MGStrOp::makeLower(info.path);

				if (pathCmptemp.empty())		
					pathCmptemp = info.path;

				if (pathCmptemp != info.path)
				{
					if ( PathFileVec.size() != 0 )
					{
						char fullPackPathtemp[LEN];
						wsprintf(fullPackPathtemp, "%s/%s", fullpath.c_str(), pathCmptemp.c_str());
						mPathFileInfoMap->insert(std::make_pair(fullPackPathtemp, PathFileVec));
						mPathFileInfoTypeMap->insert(std::make_pair(fullPackPathtemp, PathMap));
						PathFileVec.clear();
						PathMap.clear();
					}

					pathCmptemp = info.path;
					PathFileVec.push_back(info);
					PathMap.insert(std::make_pair(info.filetype, info));
				}
				else
				{
					PathFileVec.push_back(info);
					PathMap.insert(std::make_pair(info.filetype, info));
				}
			}
		}

		if (PathFileVec.size() != 0)
		{
			if (mPathFileInfoMap->size() == 0)
			{
				mPathFileInfoMap->insert(std::make_pair(fullpath, PathFileVec));
				mPathFileInfoTypeMap->insert(std::make_pair(fullpath, PathMap));
			}
			else
			{
				char fullPackPathtemp[LEN];
				wsprintf(fullPackPathtemp, "%s/%s", fullpath.c_str(), pathCmptemp.c_str());
				mPathFileInfoMap->insert(std::make_pair(fullPackPathtemp, PathFileVec));
				mPathFileInfoTypeMap->insert(std::make_pair(fullPackPathtemp, PathMap));
			}
		}
		//mArchive.Close();
	}

	//-----------------------------------------------------------------------
	void MGPackFileSet::unLoad()
	{

	}

	//-----------------------------------------------------------------------
	Bool MGPackFileSet::extractPackFile( const std::string& pathfilename, MGExtractedPackFile& file )
	{
		std::string name = pathfilename;
		MGStrOp::makeLower(name);
		MGStrOp::str_replace(name, "\\", "/");
		std::string filename =  mFullNameResName[name];
		MGStrOp::str_replace(filename, "/", "\\");

		// 查找文件
		int idx = mArchive.FindFile(filename.c_str(),CZipArchive::ffNoCaseSens,false);

		if( idx != (-1) )
		{
			CZipFileHeader info;
			if(mArchive.GetFileInfo(info,(WORD)idx))
			{
				CZipMemFile* pmf = new CZipMemFile;
				// 解压缩
				mArchive.ExtractFile((WORD)idx, *pmf, true, info.m_uUncomprSize);
				if (pmf)
				{
					CZipString s = info.GetFileName();
					std::string fileName((LPCTSTR)s);

					file.whole_path	= fileName;
					file.zmem_file	= pmf;
					file.st_size	= info.m_uUncomprSize;

					return true;
				}
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------
	Bool MGPackFileSet::extractPackFile( const std::string& pathfilename, char*& buf, Int& size )
	{
		MGExtractedPackFile file;

		if ( extractPackFile( pathfilename, file ) == false )
			return false;
		
		if (file.st_size != 0)
		{
			size = file.st_size;
			buf = new char[size + 1];
			memcpy(buf, file.zmem_file->Detach(), size);
			buf[size] = '\0';
			return true;
		}

		return false;
	}

	//-----------------------------------------------------------------------
	Bool MGPackFileSet::extractPackFile( const std::string& pathfilename, wchar_t*& buf )
	{
		MGExtractedPackFile file;

		if ( extractPackFile( pathfilename, file ) == false )
			return false;

		if (file.st_size != 0)
		{
			I32 size = file.st_size / sizeof(wchar_t);
			buf = new wchar_t[size + 1];
			memcpy(buf, file.zmem_file->Detach(), file.st_size);
			buf[size] = '\0';
			return true;
		}

		return false;
	}

	//-----------------------------------------------------------------------
	StringMap& MGPackFileSet::getPathPackTypeMap()
	{
		return mPathPackTypeMap;
	}

	//-----------------------------------------------------------------------
	ClassifyPathMap*& MGPackFileSet::getPathFileInfoMap()
	{
		return mPathFileInfoMap;
	}

	//-----------------------------------------------------------------------
	ClassifyPathTypeMap*& MGPackFileSet::getPathFileInfoTypeMap()
	{
		return mPathFileInfoTypeMap;
	}

	//-----------------------------------------------------------------------
	StringMap&	 MGPackFileSet::getFilePathMap()
	{
		return mFilePathMap;
	}
	
	//-----------------------------------------------------------------------
	bool MGPackFileSet::isFileExist( const std::string& filename )
	{
		std::string name = filename;
		MGStrOp::makeLower(name);
		
		StringMap::iterator iter = mFullNameResName.find(name);
		if (iter != mFullNameResName.end())
			return true;
		else
			return false;
	}


}

/******************************************************************************/


